// SPDX-License-Identifier: MIT
/**
 * @file table.c
 * @brief Heap-free curve and map implementation.
 */
#include "table/table.h"

#include <math.h>
#include <string.h>

#define TABLE_STATE UINT32_C(0x4554424C)

typedef struct table_bounds {
  size_t lower;
  size_t upper;
  float fraction;
} table_bounds_t;

static int table_type_is_supported(table_scalar_type_t type) {
  return (type >= TABLE_SCALAR_I8) && (type <= TABLE_SCALAR_F32);
}

static size_t table_scalar_size(table_scalar_type_t type) {
  switch (type) {
  case TABLE_SCALAR_I8:
    return sizeof(int8_t);
  case TABLE_SCALAR_U8:
    return sizeof(uint8_t);
  case TABLE_SCALAR_I16:
    return sizeof(int16_t);
  case TABLE_SCALAR_U16:
    return sizeof(uint16_t);
  case TABLE_SCALAR_I32:
    return sizeof(int32_t);
  case TABLE_SCALAR_U32:
    return sizeof(uint32_t);
  case TABLE_SCALAR_F32:
    return sizeof(float);
  default:
    return 0u;
  }
}

static table_status_t table_read_scalar(const void *data, table_scalar_type_t type, size_t index, table_scalar_t *value) {
  if ((data == NULL) || (value == NULL) || !table_type_is_supported(type)) {
    return TABLE_INVALID_ARGUMENT;
  }
  value->type = type;
  switch (type) {
  case TABLE_SCALAR_I8:
    value->value.i8 = ((const int8_t *)data)[index];
    break;
  case TABLE_SCALAR_U8:
    value->value.u8 = ((const uint8_t *)data)[index];
    break;
  case TABLE_SCALAR_I16:
    value->value.i16 = ((const int16_t *)data)[index];
    break;
  case TABLE_SCALAR_U16:
    value->value.u16 = ((const uint16_t *)data)[index];
    break;
  case TABLE_SCALAR_I32:
    value->value.i32 = ((const int32_t *)data)[index];
    break;
  case TABLE_SCALAR_U32:
    value->value.u32 = ((const uint32_t *)data)[index];
    break;
  case TABLE_SCALAR_F32:
    value->value.f32 = ((const float *)data)[index];
    break;
  default:
    return TABLE_INVALID_ARGUMENT;
  }
  return TABLE_OK;
}

static float table_scalar_to_float(table_scalar_t value) {
  switch (value.type) {
  case TABLE_SCALAR_I8:
    return (float)value.value.i8;
  case TABLE_SCALAR_U8:
    return (float)value.value.u8;
  case TABLE_SCALAR_I16:
    return (float)value.value.i16;
  case TABLE_SCALAR_U16:
    return (float)value.value.u16;
  case TABLE_SCALAR_I32:
    return (float)value.value.i32;
  case TABLE_SCALAR_U32:
    return (float)value.value.u32;
  case TABLE_SCALAR_F32:
    return value.value.f32;
  default:
    return 0.0F;
  }
}

static table_status_t table_write_scalar(void *data, table_scalar_type_t type, size_t index, table_scalar_t value) {
  if (data == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  if (value.type != type) {
    return TABLE_TYPE_MISMATCH;
  }
  if ((type == TABLE_SCALAR_F32) && !isfinite(value.value.f32)) {
    return TABLE_INVALID_ARGUMENT;
  }
  switch (type) {
  case TABLE_SCALAR_I8:
    ((int8_t *)data)[index] = value.value.i8;
    break;
  case TABLE_SCALAR_U8:
    ((uint8_t *)data)[index] = value.value.u8;
    break;
  case TABLE_SCALAR_I16:
    ((int16_t *)data)[index] = value.value.i16;
    break;
  case TABLE_SCALAR_U16:
    ((uint16_t *)data)[index] = value.value.u16;
    break;
  case TABLE_SCALAR_I32:
    ((int32_t *)data)[index] = value.value.i32;
    break;
  case TABLE_SCALAR_U32:
    ((uint32_t *)data)[index] = value.value.u32;
    break;
  case TABLE_SCALAR_F32:
    ((float *)data)[index] = value.value.f32;
    break;
  default:
    return TABLE_INVALID_ARGUMENT;
  }
  return TABLE_OK;
}

static table_status_t table_read_float(const void *data, table_scalar_type_t type, size_t index, float *value) {
  table_scalar_t scalar;
  table_status_t status = table_read_scalar(data, type, index, &scalar);
  if (status != TABLE_OK) {
    return status;
  }
  *value = table_scalar_to_float(scalar);
  return TABLE_OK;
}

static table_status_t table_validate_axis(const void *axis, size_t count, table_scalar_type_t type) {
  float previous = 0.0F;
  if ((axis == NULL) || (count == 0u) || !table_type_is_supported(type)) {
    return TABLE_INVALID_ARGUMENT;
  }
  (void)table_read_float(axis, type, 0u, &previous);
  if (!isfinite(previous)) {
    return TABLE_INVALID_AXIS;
  }
  for (size_t index = 1u; index < count; ++index) {
    float current = 0.0F;
    (void)table_read_float(axis, type, index, &current);
    if (!isfinite(current) || (current <= previous)) {
      return TABLE_INVALID_AXIS;
    }
    previous = current;
  }
  return TABLE_OK;
}

static table_status_t table_validate_cells(const void *cells, size_t count, table_scalar_type_t type) {
  if ((cells == NULL) || (count == 0u) || !table_type_is_supported(type)) {
    return TABLE_INVALID_ARGUMENT;
  }
  if (type != TABLE_SCALAR_F32) {
    return TABLE_OK;
  }
  for (size_t index = 0u; index < count; ++index) {
    if (!isfinite(((const float *)cells)[index])) {
      return TABLE_INVALID_ARGUMENT;
    }
  }
  return TABLE_OK;
}

static int table_multiply_size(size_t left, size_t right, size_t *result) {
  if ((left != 0u) && (right > (SIZE_MAX / left))) {
    return 0;
  }
  *result = left * right;
  return 1;
}

table_status_t table_validate(const table_view_t *view) {
  size_t cell_count;
  table_status_t status;
  if (view == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  if ((view->kind != TABLE_KIND_CURVE) && (view->kind != TABLE_KIND_MAP)) {
    return TABLE_INVALID_STATE;
  }
  status = table_validate_axis(view->x_axis, view->x_count, view->x_type);
  if (status != TABLE_OK) {
    return status;
  }
  if (view->kind == TABLE_KIND_CURVE) {
    if ((view->y_axis != NULL) || (view->y_count != 0u) || (view->y_type != TABLE_SCALAR_UNSPECIFIED)) {
      return TABLE_INVALID_ARGUMENT;
    }
    cell_count = view->x_count;
  } else {
    status = table_validate_axis(view->y_axis, view->y_count, view->y_type);
    if (status != TABLE_OK) {
      return status;
    }
    if (!table_multiply_size(view->x_count, view->y_count, &cell_count)) {
      return TABLE_INVALID_ARGUMENT;
    }
  }
  return table_validate_cells(view->cells, cell_count, view->cell_type);
}

table_status_t table_curve_view_init(table_view_t *view, const void *x_axis, size_t x_count, table_scalar_type_t x_type, const void *cells, table_scalar_type_t cell_type) {
  table_view_t candidate;
  table_status_t status;
  if (view == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  candidate.kind = TABLE_KIND_CURVE;
  candidate.x_type = x_type;
  candidate.y_type = TABLE_SCALAR_UNSPECIFIED;
  candidate.cell_type = cell_type;
  candidate.x_count = x_count;
  candidate.y_count = 0u;
  candidate.x_axis = x_axis;
  candidate.y_axis = NULL;
  candidate.cells = cells;
  candidate._state = 0u;
  status = table_validate(&candidate);
  if (status != TABLE_OK) {
    return status;
  }
  candidate._state = TABLE_STATE;
  *view = candidate;
  return TABLE_OK;
}

table_status_t table_map_view_init(table_view_t *view, const void *x_axis, size_t x_count, table_scalar_type_t x_type, const void *y_axis, size_t y_count, table_scalar_type_t y_type, const void *cells, table_scalar_type_t cell_type) {
  table_view_t candidate;
  table_status_t status;
  if (view == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  candidate.kind = TABLE_KIND_MAP;
  candidate.x_type = x_type;
  candidate.y_type = y_type;
  candidate.cell_type = cell_type;
  candidate.x_count = x_count;
  candidate.y_count = y_count;
  candidate.x_axis = x_axis;
  candidate.y_axis = y_axis;
  candidate.cells = cells;
  candidate._state = 0u;
  status = table_validate(&candidate);
  if (status != TABLE_OK) {
    return status;
  }
  candidate._state = TABLE_STATE;
  *view = candidate;
  return TABLE_OK;
}

table_status_t table_curve_init(table_mutable_t *table, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *cells, table_scalar_type_t cell_type) {
  table_view_t view;
  table_status_t status;
  if (table == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  status = table_curve_view_init(&view, x_axis, x_count, x_type, cells, cell_type);
  if (status != TABLE_OK) {
    return status;
  }
  table->view = view;
  table->x_axis = x_axis;
  table->y_axis = NULL;
  table->cells = cells;
  return TABLE_OK;
}

table_status_t table_map_init(table_mutable_t *table, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *y_axis, size_t y_count, table_scalar_type_t y_type, void *cells, table_scalar_type_t cell_type) {
  table_view_t view;
  table_status_t status;
  if (table == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  status = table_map_view_init(&view, x_axis, x_count, x_type, y_axis, y_count, y_type, cells, cell_type);
  if (status != TABLE_OK) {
    return status;
  }
  table->view = view;
  table->x_axis = x_axis;
  table->y_axis = y_axis;
  table->cells = cells;
  return TABLE_OK;
}

static table_status_t table_check_ready(const table_view_t *view, table_kind_t kind) {
  if (view == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  if ((view->_state != TABLE_STATE) || (view->kind != kind)) {
    return TABLE_INVALID_STATE;
  }
  return TABLE_OK;
}

static void table_find_bounds(const void *axis, table_scalar_type_t type, size_t count, float input, table_bounds_t *bounds) {
  float first = 0.0F;
  float last = 0.0F;
  (void)table_read_float(axis, type, 0u, &first);
  (void)table_read_float(axis, type, count - 1u, &last);
  if ((input <= first) || (count == 1u)) {
    bounds->lower = 0u;
    bounds->upper = 0u;
    bounds->fraction = 0.0F;
    return;
  }
  if (input >= last) {
    bounds->lower = count - 1u;
    bounds->upper = count - 1u;
    bounds->fraction = 0.0F;
    return;
  }
  size_t lower = 0u;
  size_t upper = count - 1u;
  while ((upper - lower) > 1u) {
    const size_t middle = lower + (upper - lower) / 2u;
    float value = 0.0F;
    (void)table_read_float(axis, type, middle, &value);
    if (input < value) {
      upper = middle;
    } else {
      lower = middle;
    }
  }
  float lower_value = 0.0F;
  float upper_value = 0.0F;
  (void)table_read_float(axis, type, lower, &lower_value);
  (void)table_read_float(axis, type, upper, &upper_value);
  bounds->lower = lower;
  bounds->upper = upper;
  bounds->fraction = (input - lower_value) / (upper_value - lower_value);
}

static float table_lerp(float lower, float upper, float fraction) {
  return (lower * (1.0F - fraction)) + (upper * fraction);
}

table_status_t table_curve_lookup(const table_view_t *view, float x, float *output) {
  table_bounds_t bounds;
  float lower = 0.0F;
  float upper = 0.0F;
  table_status_t status = table_check_ready(view, TABLE_KIND_CURVE);
  if (status != TABLE_OK) {
    return status;
  }
  if ((output == NULL) || !isfinite(x)) {
    return TABLE_INVALID_ARGUMENT;
  }
  table_find_bounds(view->x_axis, view->x_type, view->x_count, x, &bounds);
  (void)table_read_float(view->cells, view->cell_type, bounds.lower, &lower);
  (void)table_read_float(view->cells, view->cell_type, bounds.upper, &upper);
  *output = table_lerp(lower, upper, bounds.fraction);
  return TABLE_OK;
}

table_status_t table_map_lookup(const table_view_t *view, float x, float y, float *output) {
  table_bounds_t x_bounds;
  table_bounds_t y_bounds;
  float lower_left = 0.0F;
  float lower_right = 0.0F;
  float upper_left = 0.0F;
  float upper_right = 0.0F;
  table_status_t status = table_check_ready(view, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  if ((output == NULL) || !isfinite(x) || !isfinite(y)) {
    return TABLE_INVALID_ARGUMENT;
  }
  table_find_bounds(view->x_axis, view->x_type, view->x_count, x, &x_bounds);
  table_find_bounds(view->y_axis, view->y_type, view->y_count, y, &y_bounds);
  (void)table_read_float(view->cells, view->cell_type, (y_bounds.lower * view->x_count) + x_bounds.lower, &lower_left);
  (void)table_read_float(view->cells, view->cell_type, (y_bounds.lower * view->x_count) + x_bounds.upper, &lower_right);
  (void)table_read_float(view->cells, view->cell_type, (y_bounds.upper * view->x_count) + x_bounds.lower, &upper_left);
  (void)table_read_float(view->cells, view->cell_type, (y_bounds.upper * view->x_count) + x_bounds.upper, &upper_right);
  *output = table_lerp(table_lerp(lower_left, lower_right, x_bounds.fraction), table_lerp(upper_left, upper_right, x_bounds.fraction), y_bounds.fraction);
  return TABLE_OK;
}

table_status_t table_get_x(const table_view_t *view, size_t index, table_scalar_t *value) {
  if ((view == NULL) || (view->_state != TABLE_STATE)) {
    return (view == NULL) ? TABLE_INVALID_ARGUMENT : TABLE_INVALID_STATE;
  }
  if (index >= view->x_count) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  return table_read_scalar(view->x_axis, view->x_type, index, value);
}

table_status_t table_get_y(const table_view_t *view, size_t index, table_scalar_t *value) {
  table_status_t status = table_check_ready(view, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  if (index >= view->y_count) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  return table_read_scalar(view->y_axis, view->y_type, index, value);
}

table_status_t table_curve_get_cell(const table_view_t *view, size_t x_index, table_scalar_t *value) {
  table_status_t status = table_check_ready(view, TABLE_KIND_CURVE);
  if (status != TABLE_OK) {
    return status;
  }
  if (x_index >= view->x_count) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  return table_read_scalar(view->cells, view->cell_type, x_index, value);
}

table_status_t table_map_get_cell(const table_view_t *view, size_t x_index, size_t y_index, table_scalar_t *value) {
  table_status_t status = table_check_ready(view, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  if ((x_index >= view->x_count) || (y_index >= view->y_count)) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  return table_read_scalar(view->cells, view->cell_type, (y_index * view->x_count) + x_index, value);
}

static table_status_t table_check_mutable(const table_mutable_t *table, table_kind_t kind) {
  table_status_t status;
  if (table == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  status = table_check_ready(&table->view, kind);
  if (status != TABLE_OK) {
    return status;
  }
  if ((table->x_axis == NULL) || (table->cells == NULL) || ((kind == TABLE_KIND_MAP) && (table->y_axis == NULL))) {
    return TABLE_INVALID_STATE;
  }
  return TABLE_OK;
}

static table_status_t table_set_axis(void *axis, const void *axis_view, size_t count, table_scalar_type_t type, size_t index, table_scalar_t value) {
  float candidate;
  if (index >= count) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  if (value.type != type) {
    return TABLE_TYPE_MISMATCH;
  }
  candidate = table_scalar_to_float(value);
  if (!isfinite(candidate)) {
    return TABLE_INVALID_AXIS;
  }
  if (index > 0u) {
    float previous = 0.0F;
    (void)table_read_float(axis_view, type, index - 1u, &previous);
    if (candidate <= previous) {
      return TABLE_INVALID_AXIS;
    }
  }
  if ((index + 1u) < count) {
    float next = 0.0F;
    (void)table_read_float(axis_view, type, index + 1u, &next);
    if (candidate >= next) {
      return TABLE_INVALID_AXIS;
    }
  }
  return table_write_scalar(axis, type, index, value);
}

table_status_t table_set_x(table_mutable_t *table, size_t index, table_scalar_t value) {
  table_status_t status;
  if (table == NULL) {
    return TABLE_INVALID_ARGUMENT;
  }
  status = (table->view.kind == TABLE_KIND_CURVE) ? table_check_mutable(table, TABLE_KIND_CURVE) : table_check_mutable(table, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  return table_set_axis(table->x_axis, table->view.x_axis, table->view.x_count, table->view.x_type, index, value);
}

table_status_t table_set_y(table_mutable_t *table, size_t index, table_scalar_t value) {
  table_status_t status = table_check_mutable(table, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  return table_set_axis(table->y_axis, table->view.y_axis, table->view.y_count, table->view.y_type, index, value);
}

table_status_t table_curve_set_cell(table_mutable_t *table, size_t x_index, table_scalar_t value) {
  table_status_t status = table_check_mutable(table, TABLE_KIND_CURVE);
  if (status != TABLE_OK) {
    return status;
  }
  if (x_index >= table->view.x_count) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  return table_write_scalar(table->cells, table->view.cell_type, x_index, value);
}

table_status_t table_map_set_cell(table_mutable_t *table, size_t x_index, size_t y_index, table_scalar_t value) {
  table_status_t status = table_check_mutable(table, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  if ((x_index >= table->view.x_count) || (y_index >= table->view.y_count)) {
    return TABLE_INDEX_OUT_OF_RANGE;
  }
  return table_write_scalar(table->cells, table->view.cell_type, (y_index * table->view.x_count) + x_index, value);
}

table_status_t table_replace(table_mutable_t *table, const table_view_t *candidate) {
  size_t cell_count;
  size_t cell_bytes;
  size_t x_bytes;
  size_t y_bytes = 0u;
  table_status_t status;
  if ((table == NULL) || (candidate == NULL)) {
    return TABLE_INVALID_ARGUMENT;
  }
  status = (table->view.kind == TABLE_KIND_CURVE) ? table_check_mutable(table, TABLE_KIND_CURVE) : table_check_mutable(table, TABLE_KIND_MAP);
  if (status != TABLE_OK) {
    return status;
  }
  if ((candidate->kind != table->view.kind) || (candidate->x_count != table->view.x_count) || (candidate->y_count != table->view.y_count) || (candidate->x_type != table->view.x_type) || (candidate->y_type != table->view.y_type) || (candidate->cell_type != table->view.cell_type)) {
    return TABLE_TYPE_MISMATCH;
  }
  status = table_validate(candidate);
  if (status != TABLE_OK) {
    return status;
  }
  if (!table_multiply_size(table->view.x_count, table_scalar_size(table->view.x_type), &x_bytes)) {
    return TABLE_INVALID_ARGUMENT;
  }
  if (table->view.kind == TABLE_KIND_MAP) {
    if (!table_multiply_size(table->view.y_count, table_scalar_size(table->view.y_type), &y_bytes)) {
      return TABLE_INVALID_ARGUMENT;
    }
    if (!table_multiply_size(table->view.x_count, table->view.y_count, &cell_count)) {
      return TABLE_INVALID_ARGUMENT;
    }
  } else {
    cell_count = table->view.x_count;
  }
  if (!table_multiply_size(cell_count, table_scalar_size(table->view.cell_type), &cell_bytes)) {
    return TABLE_INVALID_ARGUMENT;
  }
  (void)memmove(table->x_axis, candidate->x_axis, x_bytes);
  if (table->view.kind == TABLE_KIND_MAP) {
    (void)memmove(table->y_axis, candidate->y_axis, y_bytes);
  }
  (void)memmove(table->cells, candidate->cells, cell_bytes);
  return TABLE_OK;
}
