// SPDX-License-Identifier: MIT
/**
 * @file nanopb.c
 * @brief Optional Nanopb bindings for Table snapshots.
 */
#include "table/nanopb.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define TABLE_ASSERT_PROTO_TYPE(native_type, proto_type) _Static_assert((int)(native_type) == (int)(proto_type), "table scalar type values must match the protobuf schema")
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_UNSPECIFIED, table_v1_ScalarType_SCALAR_TYPE_UNSPECIFIED);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_I8, table_v1_ScalarType_SCALAR_TYPE_I8);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_U8, table_v1_ScalarType_SCALAR_TYPE_U8);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_I16, table_v1_ScalarType_SCALAR_TYPE_I16);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_U16, table_v1_ScalarType_SCALAR_TYPE_U16);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_I32, table_v1_ScalarType_SCALAR_TYPE_I32);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_U32, table_v1_ScalarType_SCALAR_TYPE_U32);
TABLE_ASSERT_PROTO_TYPE(TABLE_SCALAR_F32, table_v1_ScalarType_SCALAR_TYPE_F32);
#undef TABLE_ASSERT_PROTO_TYPE

static table_nanopb_family_t table_nanopb_family(table_scalar_type_t type) {
  if (type == TABLE_SCALAR_I8 || type == TABLE_SCALAR_I16 || type == TABLE_SCALAR_I32)
    return TABLE_NANOPB_FAMILY_SIGNED;
  if (type == TABLE_SCALAR_F32)
    return TABLE_NANOPB_FAMILY_FLOAT;
  return TABLE_NANOPB_FAMILY_UNSIGNED;
}

static table_v1_ScalarType table_nanopb_proto_type(table_scalar_type_t type) {
  return (table_v1_ScalarType)type;
}

static int table_nanopb_type_is_supported(table_scalar_type_t type) {
  return type >= TABLE_SCALAR_I8 && type <= TABLE_SCALAR_F32;
}

static uint32_t table_nanopb_unsigned_value(const void *data, table_scalar_type_t type, size_t index) {
  switch (type) {
  case TABLE_SCALAR_U8:
    return ((const uint8_t *)data)[index];
  case TABLE_SCALAR_U16:
    return ((const uint16_t *)data)[index];
  case TABLE_SCALAR_U32:
    return ((const uint32_t *)data)[index];
  default:
    return 0u;
  }
}

static int32_t table_nanopb_signed_value(const void *data, table_scalar_type_t type, size_t index) {
  switch (type) {
  case TABLE_SCALAR_I8:
    return ((const int8_t *)data)[index];
  case TABLE_SCALAR_I16:
    return ((const int16_t *)data)[index];
  case TABLE_SCALAR_I32:
    return ((const int32_t *)data)[index];
  default:
    return 0;
  }
}

static size_t table_nanopb_varint_size(uint32_t value) {
  size_t size = 1u;
  while (value >= UINT32_C(0x80)) {
    value >>= 7u;
    ++size;
  }
  return size;
}

static uint32_t table_nanopb_zigzag(int32_t value) {
  return ((uint32_t)value << 1u) ^ (uint32_t) - (value < 0);
}

static bool table_nanopb_encode_values(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
  const table_nanopb_encoder_array_t *array = (const table_nanopb_encoder_array_t *)*arg;
  size_t payload_size = 0u;
  if (array == NULL || array->data == NULL || array->count == 0u)
    PB_RETURN_ERROR(stream, "invalid table encoder");
  if (array->family == TABLE_NANOPB_FAMILY_FLOAT) {
    if (array->count > UINT32_MAX / sizeof(float))
      PB_RETURN_ERROR(stream, "table payload too large");
    payload_size = array->count * sizeof(float);
  } else {
    for (size_t index = 0u; index < array->count; ++index) {
      const uint32_t value = array->family == TABLE_NANOPB_FAMILY_SIGNED ? table_nanopb_zigzag(table_nanopb_signed_value(array->data, array->type, index)) : table_nanopb_unsigned_value(array->data, array->type, index);
      const size_t item_size = table_nanopb_varint_size(value);
      if (payload_size > UINT32_MAX - item_size)
        PB_RETURN_ERROR(stream, "table payload too large");
      payload_size += item_size;
    }
  }
  if (!pb_encode_tag(stream, PB_WT_STRING, field->tag) || !pb_encode_varint(stream, (uint32_t)payload_size))
    return false;
  for (size_t index = 0u; index < array->count; ++index) {
    if (array->family == TABLE_NANOPB_FAMILY_SIGNED) {
      if (!pb_encode_svarint(stream, table_nanopb_signed_value(array->data, array->type, index)))
        return false;
    } else if (array->family == TABLE_NANOPB_FAMILY_UNSIGNED) {
      if (!pb_encode_varint(stream, table_nanopb_unsigned_value(array->data, array->type, index)))
        return false;
    } else {
      const float value = ((const float *)array->data)[index];
      if (!pb_encode_fixed32(stream, &value))
        return false;
    }
  }
  return true;
}

static void table_nanopb_bind_encoder_array(table_nanopb_encoder_array_t *encoder, table_v1_ScalarArray *message, const void *data, size_t count, table_scalar_type_t type) {
  *message = (table_v1_ScalarArray)table_v1_ScalarArray_init_zero;
  encoder->data = data;
  encoder->count = count;
  encoder->type = type;
  encoder->family = table_nanopb_family(type);
  message->type = table_nanopb_proto_type(type);
  if (encoder->family == TABLE_NANOPB_FAMILY_SIGNED) {
    message->signed_values.funcs.encode = table_nanopb_encode_values;
    message->signed_values.arg = encoder;
  } else if (encoder->family == TABLE_NANOPB_FAMILY_UNSIGNED) {
    message->unsigned_values.funcs.encode = table_nanopb_encode_values;
    message->unsigned_values.arg = encoder;
  } else {
    message->float_values.funcs.encode = table_nanopb_encode_values;
    message->float_values.arg = encoder;
  }
}

table_status_t table_nanopb_curve_encode_init(table_nanopb_curve_encoder_t *encoder, table_v1_Curve *message, const table_view_t *view) {
  table_status_t status;
  if (encoder == NULL || message == NULL || view == NULL)
    return TABLE_INVALID_ARGUMENT;
  if (view->kind != TABLE_KIND_CURVE)
    return TABLE_INVALID_STATE;
  status = table_validate(view);
  if (status != TABLE_OK)
    return status;
  *message = (table_v1_Curve)table_v1_Curve_init_zero;
  message->has_x_axis = true;
  message->has_cells = true;
  table_nanopb_bind_encoder_array(&encoder->x_axis, &message->x_axis, view->x_axis, view->x_count, view->x_type);
  table_nanopb_bind_encoder_array(&encoder->cells, &message->cells, view->cells, view->x_count, view->cell_type);
  return TABLE_OK;
}

table_status_t table_nanopb_map_encode_init(table_nanopb_map_encoder_t *encoder, table_v1_Map *message, const table_view_t *view) {
  size_t cell_count;
  table_status_t status;
  if (encoder == NULL || message == NULL || view == NULL)
    return TABLE_INVALID_ARGUMENT;
  if (view->kind != TABLE_KIND_MAP)
    return TABLE_INVALID_STATE;
  status = table_validate(view);
  if (status != TABLE_OK)
    return status;
  if (view->x_count != 0u && view->y_count > SIZE_MAX / view->x_count)
    return TABLE_INVALID_ARGUMENT;
  cell_count = view->x_count * view->y_count;
  *message = (table_v1_Map)table_v1_Map_init_zero;
  message->has_x_axis = true;
  message->has_y_axis = true;
  message->has_cells = true;
  table_nanopb_bind_encoder_array(&encoder->x_axis, &message->x_axis, view->x_axis, view->x_count, view->x_type);
  table_nanopb_bind_encoder_array(&encoder->y_axis, &message->y_axis, view->y_axis, view->y_count, view->y_type);
  table_nanopb_bind_encoder_array(&encoder->cells, &message->cells, view->cells, cell_count, view->cell_type);
  return TABLE_OK;
}

static int table_nanopb_store_signed(table_nanopb_decoder_array_t *array, int32_t value) {
  switch (array->type) {
  case TABLE_SCALAR_I8:
    if (value < INT8_MIN || value > INT8_MAX)
      return 0;
    ((int8_t *)array->data)[array->count] = (int8_t)value;
    return 1;
  case TABLE_SCALAR_I16:
    if (value < INT16_MIN || value > INT16_MAX)
      return 0;
    ((int16_t *)array->data)[array->count] = (int16_t)value;
    return 1;
  case TABLE_SCALAR_I32:
    ((int32_t *)array->data)[array->count] = value;
    return 1;
  default:
    return 0;
  }
}

static int table_nanopb_store_unsigned(table_nanopb_decoder_array_t *array, uint32_t value) {
  switch (array->type) {
  case TABLE_SCALAR_U8:
    if (value > UINT8_MAX)
      return 0;
    ((uint8_t *)array->data)[array->count] = (uint8_t)value;
    return 1;
  case TABLE_SCALAR_U16:
    if (value > UINT16_MAX)
      return 0;
    ((uint16_t *)array->data)[array->count] = (uint16_t)value;
    return 1;
  case TABLE_SCALAR_U32:
    ((uint32_t *)array->data)[array->count] = value;
    return 1;
  default:
    return 0;
  }
}

static bool table_nanopb_decode_values(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  table_nanopb_decoder_field_t *decoder_field = (table_nanopb_decoder_field_t *)*arg;
  table_nanopb_decoder_array_t *array;
  (void)field;
  if (decoder_field == NULL || decoder_field->array == NULL)
    PB_RETURN_ERROR(stream, "invalid table decoder");
  array = decoder_field->array;
  if (decoder_field->family != array->family)
    PB_RETURN_ERROR(stream, "wrong table value family");
  while (stream->bytes_left > 0u) {
    if (array->count >= array->capacity)
      PB_RETURN_ERROR(stream, "table capacity exceeded");
    if (array->family == TABLE_NANOPB_FAMILY_SIGNED) {
#ifndef PB_WITHOUT_64BIT
      int64_t decoded;
      if (!pb_decode_svarint(stream, &decoded) || decoded < INT32_MIN || decoded > INT32_MAX || !table_nanopb_store_signed(array, (int32_t)decoded))
        PB_RETURN_ERROR(stream, "invalid signed table value");
#else
      int32_t decoded;
      if (!pb_decode_svarint(stream, &decoded) || !table_nanopb_store_signed(array, decoded))
        PB_RETURN_ERROR(stream, "invalid signed table value");
#endif
    } else if (array->family == TABLE_NANOPB_FAMILY_UNSIGNED) {
#ifndef PB_WITHOUT_64BIT
      uint64_t decoded;
      if (!pb_decode_varint(stream, &decoded) || decoded > UINT32_MAX || !table_nanopb_store_unsigned(array, (uint32_t)decoded))
        PB_RETURN_ERROR(stream, "invalid unsigned table value");
#else
      uint32_t decoded;
      if (!pb_decode_varint(stream, &decoded) || !table_nanopb_store_unsigned(array, decoded))
        PB_RETURN_ERROR(stream, "invalid unsigned table value");
#endif
    } else {
      float decoded = 0.0F;
      if (!pb_decode_fixed32(stream, &decoded) || !isfinite(decoded))
        PB_RETURN_ERROR(stream, "invalid float table value");
      ((float *)array->data)[array->count] = decoded;
    }
    ++array->count;
  }
  return true;
}

static table_status_t table_nanopb_bind_decoder_array(table_nanopb_decoder_array_t *decoder, table_v1_ScalarArray *message, void *data, size_t capacity, table_scalar_type_t type) {
  if (decoder == NULL || message == NULL || data == NULL || capacity == 0u || !table_nanopb_type_is_supported(type))
    return TABLE_INVALID_ARGUMENT;
  *message = (table_v1_ScalarArray)table_v1_ScalarArray_init_zero;
  decoder->data = data;
  decoder->capacity = capacity;
  decoder->count = 0u;
  decoder->type = type;
  decoder->family = table_nanopb_family(type);
  for (size_t index = 0u; index < 3u; ++index) {
    decoder->fields[index].array = decoder;
    decoder->fields[index].family = (table_nanopb_family_t)index;
  }
  message->signed_values.funcs.decode = table_nanopb_decode_values;
  message->signed_values.arg = &decoder->fields[TABLE_NANOPB_FAMILY_SIGNED];
  message->unsigned_values.funcs.decode = table_nanopb_decode_values;
  message->unsigned_values.arg = &decoder->fields[TABLE_NANOPB_FAMILY_UNSIGNED];
  message->float_values.funcs.decode = table_nanopb_decode_values;
  message->float_values.arg = &decoder->fields[TABLE_NANOPB_FAMILY_FLOAT];
  return TABLE_OK;
}

table_status_t table_nanopb_curve_decode_init(table_nanopb_curve_decoder_t *decoder, table_v1_Curve *message, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *cells, table_scalar_type_t cell_type) {
  table_status_t status;
  if (decoder == NULL || message == NULL)
    return TABLE_INVALID_ARGUMENT;
  *message = (table_v1_Curve)table_v1_Curve_init_zero;
  status = table_nanopb_bind_decoder_array(&decoder->x_axis, &message->x_axis, x_axis, x_count, x_type);
  if (status != TABLE_OK)
    return status;
  return table_nanopb_bind_decoder_array(&decoder->cells, &message->cells, cells, x_count, cell_type);
}

table_status_t table_nanopb_map_decode_init(table_nanopb_map_decoder_t *decoder, table_v1_Map *message, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *y_axis, size_t y_count, table_scalar_type_t y_type, void *cells, table_scalar_type_t cell_type) {
  size_t cell_count;
  table_status_t status;
  if (decoder == NULL || message == NULL || (x_count != 0u && y_count > SIZE_MAX / x_count))
    return TABLE_INVALID_ARGUMENT;
  cell_count = x_count * y_count;
  *message = (table_v1_Map)table_v1_Map_init_zero;
  status = table_nanopb_bind_decoder_array(&decoder->x_axis, &message->x_axis, x_axis, x_count, x_type);
  if (status != TABLE_OK)
    return status;
  status = table_nanopb_bind_decoder_array(&decoder->y_axis, &message->y_axis, y_axis, y_count, y_type);
  if (status != TABLE_OK)
    return status;
  return table_nanopb_bind_decoder_array(&decoder->cells, &message->cells, cells, cell_count, cell_type);
}

static table_status_t table_nanopb_decoder_array_finish(const table_nanopb_decoder_array_t *decoder, const table_v1_ScalarArray *message) {
  if (decoder == NULL || message == NULL)
    return TABLE_INVALID_ARGUMENT;
  if (message->type != table_nanopb_proto_type(decoder->type))
    return TABLE_TYPE_MISMATCH;
  if (decoder->count != decoder->capacity)
    return TABLE_INVALID_ARGUMENT;
  return TABLE_OK;
}

table_status_t table_nanopb_curve_decode_finish(const table_nanopb_curve_decoder_t *decoder, const table_v1_Curve *message, table_view_t *view) {
  table_status_t status;
  if (decoder == NULL || message == NULL || view == NULL)
    return TABLE_INVALID_ARGUMENT;
  if (!message->has_x_axis || !message->has_cells)
    return TABLE_INVALID_ARGUMENT;
  status = table_nanopb_decoder_array_finish(&decoder->x_axis, &message->x_axis);
  if (status != TABLE_OK)
    return status;
  status = table_nanopb_decoder_array_finish(&decoder->cells, &message->cells);
  if (status != TABLE_OK)
    return status;
  return table_curve_view_init(view, decoder->x_axis.data, decoder->x_axis.capacity, decoder->x_axis.type, decoder->cells.data, decoder->cells.type);
}

table_status_t table_nanopb_map_decode_finish(const table_nanopb_map_decoder_t *decoder, const table_v1_Map *message, table_view_t *view) {
  table_status_t status;
  if (decoder == NULL || message == NULL || view == NULL)
    return TABLE_INVALID_ARGUMENT;
  if (!message->has_x_axis || !message->has_y_axis || !message->has_cells)
    return TABLE_INVALID_ARGUMENT;
  status = table_nanopb_decoder_array_finish(&decoder->x_axis, &message->x_axis);
  if (status != TABLE_OK)
    return status;
  status = table_nanopb_decoder_array_finish(&decoder->y_axis, &message->y_axis);
  if (status != TABLE_OK)
    return status;
  status = table_nanopb_decoder_array_finish(&decoder->cells, &message->cells);
  if (status != TABLE_OK)
    return status;
  return table_map_view_init(view, decoder->x_axis.data, decoder->x_axis.capacity, decoder->x_axis.type, decoder->y_axis.data, decoder->y_axis.capacity, decoder->y_axis.type, decoder->cells.data, decoder->cells.type);
}
