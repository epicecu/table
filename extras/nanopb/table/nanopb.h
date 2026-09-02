// SPDX-License-Identifier: MIT
/**
 * @file nanopb.h
 * @brief Optional Nanopb bindings for Table snapshots.
 */
#ifndef TABLE_NANOPB_H
#define TABLE_NANOPB_H

#include "table/table.h"
#include "table/v1/table.pb.h"

#include <pb_decode.h>
#include <pb_encode.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Internal value-family selector retained by a callback context. */
typedef enum table_nanopb_family {
  TABLE_NANOPB_FAMILY_SIGNED = 0,
  TABLE_NANOPB_FAMILY_UNSIGNED,
  TABLE_NANOPB_FAMILY_FLOAT
} table_nanopb_family_t;

/** @brief Encoding state for one scalar array; retain until `pb_encode()` completes. */
typedef struct table_nanopb_encoder_array {
  const void *data;             /**< Borrowed typed array. */
  size_t count;                 /**< Element count. */
  table_scalar_type_t type;     /**< Array storage type. */
  table_nanopb_family_t family; /**< Selected Protobuf value family. */
} table_nanopb_encoder_array_t;

/** @brief Encoding state for a curve snapshot. */
typedef struct table_nanopb_curve_encoder {
  table_nanopb_encoder_array_t x_axis; /**< X-axis callback state. */
  table_nanopb_encoder_array_t cells;  /**< Cell callback state. */
} table_nanopb_curve_encoder_t;

/** @brief Encoding state for a map snapshot. */
typedef struct table_nanopb_map_encoder {
  table_nanopb_encoder_array_t x_axis; /**< X-axis callback state. */
  table_nanopb_encoder_array_t y_axis; /**< Y-axis callback state. */
  table_nanopb_encoder_array_t cells;  /**< Cell callback state. */
} table_nanopb_map_encoder_t;

struct table_nanopb_decoder_array;

/** @brief Callback field state for one possible scalar family. */
typedef struct table_nanopb_decoder_field {
  struct table_nanopb_decoder_array *array; /**< Owning array state. */
  table_nanopb_family_t family;             /**< Field value family. */
} table_nanopb_decoder_field_t;

/** @brief Decoding state for one scalar array; retain until completion. */
typedef struct table_nanopb_decoder_array {
  void *data;                             /**< Borrowed writable typed array. */
  size_t capacity;                        /**< Required and maximum element count. */
  size_t count;                           /**< Elements decoded so far. */
  table_scalar_type_t type;               /**< Required storage type. */
  table_nanopb_family_t family;           /**< Required Protobuf family. */
  table_nanopb_decoder_field_t fields[3]; /**< Per-family callback arguments. */
} table_nanopb_decoder_array_t;

/** @brief Decoding state for a complete curve snapshot. */
typedef struct table_nanopb_curve_decoder {
  table_nanopb_decoder_array_t x_axis; /**< X-axis callback state. */
  table_nanopb_decoder_array_t cells;  /**< Cell callback state. */
} table_nanopb_curve_decoder_t;

/** @brief Decoding state for a complete map snapshot. */
typedef struct table_nanopb_map_decoder {
  table_nanopb_decoder_array_t x_axis; /**< X-axis callback state. */
  table_nanopb_decoder_array_t y_axis; /**< Y-axis callback state. */
  table_nanopb_decoder_array_t cells;  /**< Cell callback state. */
} table_nanopb_map_decoder_t;

/** @brief Binds a valid curve view to a generated snapshot for encoding. @param[out] encoder Retained callback state. @param[out] message Generated message embedded in an application share. @param[in] view Curve view retained until encoding completes. @return Operation status. */
table_status_t table_nanopb_curve_encode_init(table_nanopb_curve_encoder_t *encoder, table_v1_Curve *message, const table_view_t *view);

/** @brief Binds a valid map view to a generated snapshot for encoding. @param[out] encoder Retained callback state. @param[out] message Generated message embedded in an application share. @param[in] view Map view retained until encoding completes. @return Operation status. */
table_status_t table_nanopb_map_encode_init(table_nanopb_map_encoder_t *encoder, table_v1_Map *message, const table_view_t *view);

/** @brief Prepares a generated curve message to decode into staging arrays. @param[out] decoder Retained callback state. @param[out] message Generated message embedded in an application share. @param[out] x_axis Staging X-axis array. @param x_count Required point count. @param x_type X-axis storage type. @param[out] cells Staging cell array. @param cell_type Cell storage type. @return Operation status. */
table_status_t table_nanopb_curve_decode_init(table_nanopb_curve_decoder_t *decoder, table_v1_Curve *message, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *cells, table_scalar_type_t cell_type);

/** @brief Prepares a generated map message to decode into staging arrays. @param[out] decoder Retained callback state. @param[out] message Generated message embedded in an application share. @param[out] x_axis Staging X-axis array. @param x_count Required X count. @param x_type X-axis storage type. @param[out] y_axis Staging Y-axis array. @param y_count Required Y count. @param y_type Y-axis storage type. @param[out] cells Staging row-major cells. @param cell_type Cell storage type. @return Operation status. */
table_status_t table_nanopb_map_decode_init(table_nanopb_map_decoder_t *decoder, table_v1_Map *message, void *x_axis, size_t x_count, table_scalar_type_t x_type, void *y_axis, size_t y_count, table_scalar_type_t y_type, void *cells, table_scalar_type_t cell_type);

/** @brief Validates a decoded curve snapshot and creates a staging view. Call only after successful `pb_decode()`. @param[in] decoder Completed callback state. @param[in] message Decoded generated message. @param[out] view Validated view over staging arrays. @return Operation status. */
table_status_t table_nanopb_curve_decode_finish(const table_nanopb_curve_decoder_t *decoder, const table_v1_Curve *message, table_view_t *view);

/** @brief Validates a decoded map snapshot and creates a staging view. Call only after successful `pb_decode()`. @param[in] decoder Completed callback state. @param[in] message Decoded generated message. @param[out] view Validated view over staging arrays. @return Operation status. */
table_status_t table_nanopb_map_decode_finish(const table_nanopb_map_decoder_t *decoder, const table_v1_Map *message, table_view_t *view);

#ifdef __cplusplus
}
#endif

#endif
