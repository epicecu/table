#include "table/nanopb.h"

#include <gtest/gtest.h>

#include <pb_decode.h>
#include <pb_encode.h>

#include <array>
#include <cstdint>

namespace {

struct EncodedMap {
  std::array<pb_byte_t, 256u> bytes{};
  std::size_t size{0u};
};

EncodedMap encodeMap(table_v1_Map &message) {
  EncodedMap encoded;
  pb_ostream_t stream = pb_ostream_from_buffer(encoded.bytes.data(), encoded.bytes.size());
  EXPECT_TRUE(pb_encode(&stream, table_v1_Map_fields, &message)) << PB_GET_ERROR(&stream);
  encoded.size = stream.bytes_written;
  return encoded;
}

template <typename AxisT, typename CellT>
void roundTripCurve(const AxisT (&sourceX)[2], table_scalar_type_t xType, const CellT (&sourceCells)[2], table_scalar_type_t cellType) {
  table_view_t source{};
  ASSERT_EQ(TABLE_OK, table_curve_view_init(&source, sourceX, 2u, xType, sourceCells, cellType));
  table_nanopb_curve_encoder_t encoder{};
  table_v1_Curve message = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_encode_init(&encoder, &message, &source));
  std::array<pb_byte_t, 128u> bytes{};
  pb_ostream_t output = pb_ostream_from_buffer(bytes.data(), bytes.size());
  ASSERT_TRUE(pb_encode(&output, table_v1_Curve_fields, &message)) << PB_GET_ERROR(&output);

  AxisT decodedX[2]{};
  CellT decodedCells[2]{};
  table_nanopb_curve_decoder_t decoder{};
  table_v1_Curve decodedMessage = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_decode_init(&decoder, &decodedMessage, decodedX, 2u, xType, decodedCells, cellType));
  pb_istream_t input = pb_istream_from_buffer(bytes.data(), output.bytes_written);
  ASSERT_TRUE(pb_decode(&input, table_v1_Curve_fields, &decodedMessage)) << PB_GET_ERROR(&input);
  table_view_t decoded{};
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_decode_finish(&decoder, &decodedMessage, &decoded));
  EXPECT_EQ(sourceX[0], decodedX[0]);
  EXPECT_EQ(sourceX[1], decodedX[1]);
  EXPECT_EQ(sourceCells[0], decodedCells[0]);
  EXPECT_EQ(sourceCells[1], decodedCells[1]);
}

TEST(NanopbMap, RoundTripsMixedTypesAndCommitsStagingData) {
  const std::int16_t sourceX[] = {-10, 10};
  const std::uint16_t sourceY[] = {0u, 100u};
  const std::uint8_t sourceCells[] = {10u, 20u, 30u, 40u};
  table_view_t source{};
  ASSERT_EQ(TABLE_OK, table_map_view_init(&source, sourceX, 2u, TABLE_SCALAR_I16, sourceY, 2u, TABLE_SCALAR_U16, sourceCells, TABLE_SCALAR_U8));

  table_nanopb_map_encoder_t encoder{};
  table_v1_Map encodedMessage = table_v1_Map_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_map_encode_init(&encoder, &encodedMessage, &source));
  EncodedMap encoded = encodeMap(encodedMessage);

  std::int16_t stagingX[2]{};
  std::uint16_t stagingY[2]{};
  std::uint8_t stagingCells[4]{};
  table_nanopb_map_decoder_t decoder{};
  table_v1_Map decodedMessage = table_v1_Map_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_map_decode_init(&decoder, &decodedMessage, stagingX, 2u, TABLE_SCALAR_I16, stagingY, 2u, TABLE_SCALAR_U16, stagingCells, TABLE_SCALAR_U8));
  pb_istream_t stream = pb_istream_from_buffer(encoded.bytes.data(), encoded.size);
  ASSERT_TRUE(pb_decode(&stream, table_v1_Map_fields, &decodedMessage)) << PB_GET_ERROR(&stream);
  table_view_t staging{};
  ASSERT_EQ(TABLE_OK, table_nanopb_map_decode_finish(&decoder, &decodedMessage, &staging));

  EXPECT_EQ(sourceX[0], stagingX[0]);
  EXPECT_EQ(sourceY[1], stagingY[1]);
  EXPECT_EQ(sourceCells[3], stagingCells[3]);

  std::int16_t activeX[] = {0, 20};
  std::uint16_t activeY[] = {10u, 20u};
  std::uint8_t activeCells[] = {1u, 2u, 3u, 4u};
  table_mutable_t active{};
  ASSERT_EQ(TABLE_OK, table_map_init(&active, activeX, 2u, TABLE_SCALAR_I16, activeY, 2u, TABLE_SCALAR_U16, activeCells, TABLE_SCALAR_U8));
  ASSERT_EQ(TABLE_OK, table_replace(&active, &staging));
  EXPECT_EQ(-10, activeX[0]);
  EXPECT_EQ(40u, activeCells[3]);
}

TEST(NanopbCurve, RejectsTypeMismatchWithoutChangingActiveData) {
  const std::int16_t sourceX[] = {0, 10};
  const std::uint8_t sourceCells[] = {1u, 2u};
  table_view_t source{};
  ASSERT_EQ(TABLE_OK, table_curve_view_init(&source, sourceX, 2u, TABLE_SCALAR_I16, sourceCells, TABLE_SCALAR_U8));
  table_nanopb_curve_encoder_t encoder{};
  table_v1_Curve encodedMessage = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_encode_init(&encoder, &encodedMessage, &source));
  encodedMessage.x_axis.type = table_v1_ScalarType_SCALAR_TYPE_U16;

  std::array<pb_byte_t, 128u> bytes{};
  pb_ostream_t output = pb_ostream_from_buffer(bytes.data(), bytes.size());
  ASSERT_TRUE(pb_encode(&output, table_v1_Curve_fields, &encodedMessage));

  std::int16_t stagingX[2]{};
  std::uint8_t stagingCells[2]{};
  table_nanopb_curve_decoder_t decoder{};
  table_v1_Curve decodedMessage = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_decode_init(&decoder, &decodedMessage, stagingX, 2u, TABLE_SCALAR_I16, stagingCells, TABLE_SCALAR_U8));
  pb_istream_t input = pb_istream_from_buffer(bytes.data(), output.bytes_written);
  ASSERT_TRUE(pb_decode(&input, table_v1_Curve_fields, &decodedMessage));
  table_view_t staging{};
  EXPECT_EQ(TABLE_TYPE_MISMATCH, table_nanopb_curve_decode_finish(&decoder, &decodedMessage, &staging));
}

TEST(NanopbCurve, RejectsTruncationAndCapacityExhaustion) {
  const std::uint32_t sourceX[] = {0u, 10u, 20u};
  const float sourceCells[] = {1.0F, 2.0F, 3.0F};
  table_view_t source{};
  ASSERT_EQ(TABLE_OK, table_curve_view_init(&source, sourceX, 3u, TABLE_SCALAR_U32, sourceCells, TABLE_SCALAR_F32));
  table_nanopb_curve_encoder_t encoder{};
  table_v1_Curve message = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_encode_init(&encoder, &message, &source));
  std::array<pb_byte_t, 128u> bytes{};
  pb_ostream_t output = pb_ostream_from_buffer(bytes.data(), bytes.size());
  ASSERT_TRUE(pb_encode(&output, table_v1_Curve_fields, &message));

  std::uint32_t stagingX[2]{};
  float stagingCells[2]{};
  table_nanopb_curve_decoder_t decoder{};
  table_v1_Curve decoded = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_decode_init(&decoder, &decoded, stagingX, 2u, TABLE_SCALAR_U32, stagingCells, TABLE_SCALAR_F32));
  pb_istream_t capacityInput = pb_istream_from_buffer(bytes.data(), output.bytes_written);
  EXPECT_FALSE(pb_decode(&capacityInput, table_v1_Curve_fields, &decoded));

  std::uint32_t fullX[3]{};
  float fullCells[3]{};
  decoded = table_v1_Curve_init_zero;
  ASSERT_EQ(TABLE_OK, table_nanopb_curve_decode_init(&decoder, &decoded, fullX, 3u, TABLE_SCALAR_U32, fullCells, TABLE_SCALAR_F32));
  pb_istream_t truncatedInput = pb_istream_from_buffer(bytes.data(), output.bytes_written - 1u);
  EXPECT_FALSE(pb_decode(&truncatedInput, table_v1_Curve_fields, &decoded));
}

TEST(NanopbCurve, RoundTripsRemainingScalarRepresentations) {
  const std::int8_t i8Axis[] = {-10, 10};
  const std::int32_t i32Cells[] = {-100000, 100000};
  roundTripCurve(i8Axis, TABLE_SCALAR_I8, i32Cells, TABLE_SCALAR_I32);

  const std::uint8_t u8Axis[] = {1u, 200u};
  const std::uint32_t u32Cells[] = {0u, UINT32_MAX};
  roundTripCurve(u8Axis, TABLE_SCALAR_U8, u32Cells, TABLE_SCALAR_U32);

  const float floatAxis[] = {-1.5F, 2.5F};
  const float floatCells[] = {-3.25F, 4.75F};
  roundTripCurve(floatAxis, TABLE_SCALAR_F32, floatCells, TABLE_SCALAR_F32);
}

TEST(NanopbErrors, RejectsInvalidPublicArgumentsAndIncompleteMessages) {
  table_nanopb_curve_encoder_t curveEncoder{};
  table_nanopb_map_encoder_t mapEncoder{};
  table_v1_Curve curveMessage = table_v1_Curve_init_zero;
  table_v1_Map mapMessage = table_v1_Map_init_zero;
  table_view_t view{};
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_curve_encode_init(nullptr, &curveMessage, &view));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_map_encode_init(&mapEncoder, nullptr, &view));
  view.kind = TABLE_KIND_MAP;
  EXPECT_EQ(TABLE_INVALID_STATE, table_nanopb_curve_encode_init(&curveEncoder, &curveMessage, &view));
  view.kind = TABLE_KIND_CURVE;
  EXPECT_EQ(TABLE_INVALID_STATE, table_nanopb_map_encode_init(&mapEncoder, &mapMessage, &view));

  std::int16_t axis[2]{};
  float cells[2]{};
  table_nanopb_curve_decoder_t curveDecoder{};
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_curve_decode_init(nullptr, &curveMessage, axis, 2u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_curve_decode_init(&curveDecoder, &curveMessage, nullptr, 2u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_F32));
  table_view_t decoded{};
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_curve_decode_finish(nullptr, &curveMessage, &decoded));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_curve_decode_finish(&curveDecoder, &curveMessage, &decoded));

  table_nanopb_map_decoder_t mapDecoder{};
  std::int16_t yAxis[2]{};
  float mapCells[4]{};
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_map_decode_init(nullptr, &mapMessage, axis, 2u, TABLE_SCALAR_I16, yAxis, 2u, TABLE_SCALAR_I16, mapCells, TABLE_SCALAR_F32));
  ASSERT_EQ(TABLE_OK, table_nanopb_map_decode_init(&mapDecoder, &mapMessage, axis, 2u, TABLE_SCALAR_I16, yAxis, 2u, TABLE_SCALAR_I16, mapCells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_nanopb_map_decode_finish(&mapDecoder, &mapMessage, &decoded));
}

} // namespace
