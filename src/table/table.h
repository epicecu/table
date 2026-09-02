// SPDX-License-Identifier: MIT
/**
 * @file table.h
 * @brief Heap-free C11 curve and map lookup API.
 */
#ifndef TABLE_TABLE_H
#define TABLE_TABLE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Result of a table operation. */
typedef enum table_status {
  TABLE_OK = 0,
  TABLE_INVALID_ARGUMENT,
  TABLE_INVALID_STATE,
  TABLE_INVALID_AXIS,
  TABLE_INDEX_OUT_OF_RANGE,
  TABLE_TYPE_MISMATCH
} table_status_t;

/** @brief Supported scalar storage representations. */
typedef enum table_scalar_type {
  TABLE_SCALAR_UNSPECIFIED = 0,
  TABLE_SCALAR_I8,
  TABLE_SCALAR_U8,
  TABLE_SCALAR_I16,
  TABLE_SCALAR_U16,
  TABLE_SCALAR_I32,
  TABLE_SCALAR_U32,
  TABLE_SCALAR_F32
} table_scalar_type_t;

/** @brief Scalar value with an explicit storage representation. */
typedef struct table_scalar {
  table_scalar_type_t type; /**< Active union member. */
  union {
    int8_t i8;    /**< Signed 8-bit value. */
    uint8_t u8;   /**< Unsigned 8-bit value. */
    int16_t i16;  /**< Signed 16-bit value. */
    uint16_t u16; /**< Unsigned 16-bit value. */
    int32_t i32;  /**< Signed 32-bit value. */
    uint32_t u32; /**< Unsigned 32-bit value. */
    float f32;    /**< IEEE-754 single-precision value. */
  } value;        /**< Typed scalar payload. */
} table_scalar_t;

/** @brief Table dimensionality. */
typedef enum table_kind {
  TABLE_KIND_UNSPECIFIED = 0,
  TABLE_KIND_CURVE,
  TABLE_KIND_MAP
} table_kind_t;

/** @brief Immutable view over caller-owned table arrays. */
typedef struct table_view {
  table_kind_t kind;             /**< Curve or map. */
  table_scalar_type_t x_type;    /**< X-axis storage type. */
  table_scalar_type_t y_type;    /**< Y-axis storage type, or unspecified for a curve. */
  table_scalar_type_t cell_type; /**< Cell storage type. */
  size_t x_count;                /**< Number of X-axis points. */
  size_t y_count;                /**< Number of Y-axis points, or zero for a curve. */
  const void *x_axis;            /**< Borrowed X-axis array. */
  const void *y_axis;            /**< Borrowed Y-axis array, or null for a curve. */
  const void *cells;             /**< Borrowed row-major cell array. */
  uint32_t _state;               /**< Internal initialisation marker; do not modify. */
} table_view_t;

/** @brief Mutable binding over caller-owned table arrays. */
typedef struct table_mutable {
  table_view_t view; /**< Read-only view of the bound storage. */
  void *x_axis;      /**< Borrowed writable X-axis array. */
  void *y_axis;      /**< Borrowed writable Y-axis array, or null for a curve. */
  void *cells;       /**< Borrowed writable row-major cell array. */
} table_mutable_t;

/** @brief Initialises and validates an immutable curve view. @param[out] view Destination view. @param[in] x_axis X-axis array. @param x_count Axis and cell count. @param x_type X-axis format. @param[in] cells Cell array. @param cell_type Cell format. @return Operation status. */
table_status_t table_curve_view_init(table_view_t *view, const void *x_axis, size_t x_count, table_scalar_type_t x_type, const void *cells, table_scalar_type_t cell_type);

/** @brief Initialises and validates an immutable map view. @param[out] view Destination view. @param[in] x_axis X-axis array. @param x_count X-axis count. @param x_type X-axis format. @param[in] y_axis Y-axis array. @param y_count Y-axis count. @param y_type Y-axis format. @param[in] cells Row-major cell array. @param cell_type Cell format. @return Operation status. */
table_status_t table_map_view_init(table_view_t *view, const void *x_axis, size_t x_count, table_scalar_type_t x_type, const void *y_axis, size_t y_count, table_scalar_type_t y_type, const void *cells, table_scalar_type_t cell_type);

/** @brief Initialises and validates a mutable curve binding. @param[out] table Destination binding. @param[in,out] x_axis Writable X-axis array. @param x_count Axis and cell count. @param x_type X-axis format. @param[in,out] cells Writable cell array. @param cell_type Cell format. @return Operation status. */
table_status_t table_curve_init(table_mutable_t *table, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *cells, table_scalar_type_t cell_type);

/** @brief Initialises and validates a mutable map binding. @param[out] table Destination binding. @param[in,out] x_axis Writable X-axis array. @param x_count X-axis count. @param x_type X-axis format. @param[in,out] y_axis Writable Y-axis array. @param y_count Y-axis count. @param y_type Y-axis format. @param[in,out] cells Writable row-major cells. @param cell_type Cell format. @return Operation status. */
table_status_t table_map_init(table_mutable_t *table, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *y_axis, size_t y_count, table_scalar_type_t y_type, void *cells, table_scalar_type_t cell_type);

/** @brief Validates a table view and all referenced data. @param[in] view Table view. @return Operation status. */
table_status_t table_validate(const table_view_t *view);

/** @brief Looks up a clamped curve value. @param[in] view Valid curve view. @param x Finite X coordinate. @param[out] output Interpolated result written only on success. @return Operation status. */
table_status_t table_curve_lookup(const table_view_t *view, float x, float *output);

/** @brief Looks up a clamped map value. @param[in] view Valid map view. @param x Finite X coordinate. @param y Finite Y coordinate. @param[out] output Bilinearly interpolated result written only on success. @return Operation status. */
table_status_t table_map_lookup(const table_view_t *view, float x, float y, float *output);

/** @brief Reads an X-axis point. @param[in] view Valid view. @param index X index. @param[out] value Typed value written only on success. @return Operation status. */
table_status_t table_get_x(const table_view_t *view, size_t index, table_scalar_t *value);

/** @brief Reads a Y-axis point. @param[in] view Valid map view. @param index Y index. @param[out] value Typed value written only on success. @return Operation status. */
table_status_t table_get_y(const table_view_t *view, size_t index, table_scalar_t *value);

/** @brief Reads a curve cell. @param[in] view Valid curve view. @param x_index X index. @param[out] value Typed value written only on success. @return Operation status. */
table_status_t table_curve_get_cell(const table_view_t *view, size_t x_index, table_scalar_t *value);

/** @brief Reads a map cell. @param[in] view Valid map view. @param x_index X index. @param y_index Y index. @param[out] value Typed value written only on success. @return Operation status. */
table_status_t table_map_get_cell(const table_view_t *view, size_t x_index, size_t y_index, table_scalar_t *value);

/** @brief Replaces one X-axis point while preserving strict ordering. @param[in,out] table Mutable table. @param index X index. @param value Replacement with matching type. @return Operation status. */
table_status_t table_set_x(table_mutable_t *table, size_t index, table_scalar_t value);

/** @brief Replaces one Y-axis point while preserving strict ordering. @param[in,out] table Mutable map. @param index Y index. @param value Replacement with matching type. @return Operation status. */
table_status_t table_set_y(table_mutable_t *table, size_t index, table_scalar_t value);

/** @brief Replaces one curve cell. @param[in,out] table Mutable curve. @param x_index X index. @param value Replacement with matching type. @return Operation status. */
table_status_t table_curve_set_cell(table_mutable_t *table, size_t x_index, table_scalar_t value);

/** @brief Replaces one map cell. @param[in,out] table Mutable map. @param x_index X index. @param y_index Y index. @param value Replacement with matching type. @return Operation status. */
table_status_t table_map_set_cell(table_mutable_t *table, size_t x_index, size_t y_index, table_scalar_t value);

/** @brief Replaces all bound data after validating a matching candidate. The operation is not safe to race with lookup. @param[in,out] table Active writable table. @param[in] candidate Complete candidate with identical kind, dimensions, and formats. @return Operation status. */
table_status_t table_replace(table_mutable_t *table, const table_view_t *candidate);

#ifdef __cplusplus
}
#endif

#endif
