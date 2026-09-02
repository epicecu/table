#include "table/table.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>

namespace {

table_scalar_t i16(std::int16_t value) {
  table_scalar_t scalar{};
  scalar.type = TABLE_SCALAR_I16;
  scalar.value.i16 = value;
  return scalar;
}

template <typename ValueT>
table_scalar_t scalar(table_scalar_type_t type, ValueT value);

#define TABLE_SCALAR_SPECIALISATION(cpp_type, enum_value, member)             \
  template <>                                                                 \
  table_scalar_t scalar<cpp_type>(table_scalar_type_t type, cpp_type value) { \
    table_scalar_t result{};                                                  \
    result.type = type;                                                       \
    result.value.member = value;                                              \
    return result;                                                            \
  }

TABLE_SCALAR_SPECIALISATION(std::int8_t, TABLE_SCALAR_I8, i8)
TABLE_SCALAR_SPECIALISATION(std::uint8_t, TABLE_SCALAR_U8, u8)
TABLE_SCALAR_SPECIALISATION(std::int16_t, TABLE_SCALAR_I16, i16)
TABLE_SCALAR_SPECIALISATION(std::uint16_t, TABLE_SCALAR_U16, u16)
TABLE_SCALAR_SPECIALISATION(std::int32_t, TABLE_SCALAR_I32, i32)
TABLE_SCALAR_SPECIALISATION(std::uint32_t, TABLE_SCALAR_U32, u32)
TABLE_SCALAR_SPECIALISATION(float, TABLE_SCALAR_F32, f32)

#undef TABLE_SCALAR_SPECIALISATION

template <typename ValueT>
void exerciseScalarType(table_scalar_type_t type, ValueT first, ValueT second) {
  ValueT axis[] = {first};
  ValueT cells[] = {first};
  table_mutable_t curve{};
  ASSERT_EQ(TABLE_OK, table_curve_init(&curve, axis, 1u, type, cells, type));
  EXPECT_EQ(TABLE_OK, table_set_x(&curve, 0u, scalar(type, second)));
  EXPECT_EQ(TABLE_OK, table_curve_set_cell(&curve, 0u, scalar(type, second)));
  table_scalar_t result{};
  EXPECT_EQ(TABLE_OK, table_get_x(&curve.view, 0u, &result));
  EXPECT_EQ(type, result.type);
  float output = 0.0F;
  EXPECT_EQ(TABLE_OK, table_curve_lookup(&curve.view, 0.0F, &output));
  EXPECT_FLOAT_EQ(static_cast<float>(second), output);
}

TEST(CurveC, InterpolatesExactAndClampedValues) {
  const std::int32_t axis[] = {0, 20, 40, 60, 80};
  const std::uint8_t cells[] = {20, 40, 80, 85, 90};
  table_view_t curve{};
  ASSERT_EQ(TABLE_OK, table_curve_view_init(&curve, axis, 5u, TABLE_SCALAR_I32, cells, TABLE_SCALAR_U8));

  float output = 0.0F;
  EXPECT_EQ(TABLE_OK, table_curve_lookup(&curve, 30.0F, &output));
  EXPECT_FLOAT_EQ(60.0F, output);
  EXPECT_EQ(TABLE_OK, table_curve_lookup(&curve, -10.0F, &output));
  EXPECT_FLOAT_EQ(20.0F, output);
  EXPECT_EQ(TABLE_OK, table_curve_lookup(&curve, 100.0F, &output));
  EXPECT_FLOAT_EQ(90.0F, output);
}

TEST(MapC, UsesRowMajorBilinearInterpolation) {
  const std::int16_t xAxis[] = {10, 20, 30, 40};
  const std::uint16_t yAxis[] = {10, 20, 30, 40};
  const std::uint8_t cells[] = {
      5, 40, 45, 80,
      10, 35, 50, 75,
      15, 30, 55, 70,
      20, 25, 60, 65};
  table_view_t map{};
  ASSERT_EQ(TABLE_OK, table_map_view_init(&map, xAxis, 4u, TABLE_SCALAR_I16, yAxis, 4u, TABLE_SCALAR_U16, cells, TABLE_SCALAR_U8));

  float output = 0.0F;
  EXPECT_EQ(TABLE_OK, table_map_lookup(&map, 15.0F, 15.0F, &output));
  EXPECT_FLOAT_EQ(22.5F, output);
  EXPECT_EQ(TABLE_OK, table_map_lookup(&map, 10.0F, 15.0F, &output));
  EXPECT_FLOAT_EQ(7.5F, output);
  EXPECT_EQ(TABLE_OK, table_map_lookup(&map, 35.0F, 30.0F, &output));
  EXPECT_FLOAT_EQ(62.5F, output);
}

TEST(ValidationC, RejectsMalformedAndPrecisionCollidingAxes) {
  const std::int32_t descending[] = {0, 2, 1};
  const std::int32_t colliding[] = {16777216, 16777217};
  const float nonFinite[] = {0.0F, std::numeric_limits<float>::infinity()};
  const float cells[] = {1.0F, 2.0F, 3.0F};
  table_view_t view{};
  EXPECT_EQ(TABLE_INVALID_AXIS, table_curve_view_init(&view, descending, 3u, TABLE_SCALAR_I32, cells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_INVALID_AXIS, table_curve_view_init(&view, colliding, 2u, TABLE_SCALAR_I32, cells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_INVALID_AXIS, table_curve_view_init(&view, nonFinite, 2u, TABLE_SCALAR_F32, cells, TABLE_SCALAR_F32));
}

TEST(MutationC, PreservesOrderingAndCommitsValidReplacement) {
  std::int16_t axis[] = {0, 10, 20};
  std::int16_t cells[] = {0, 100, 200};
  table_mutable_t curve{};
  ASSERT_EQ(TABLE_OK, table_curve_init(&curve, axis, 3u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_I16));

  EXPECT_EQ(TABLE_INVALID_AXIS, table_set_x(&curve, 1u, i16(20)));
  EXPECT_EQ(10, axis[1]);
  EXPECT_EQ(TABLE_OK, table_curve_set_cell(&curve, 1u, i16(120)));

  const std::int16_t replacementAxis[] = {0, 20, 40};
  const std::int16_t replacementCells[] = {10, 20, 30};
  table_view_t replacement{};
  ASSERT_EQ(TABLE_OK, table_curve_view_init(&replacement, replacementAxis, 3u, TABLE_SCALAR_I16, replacementCells, TABLE_SCALAR_I16));
  EXPECT_EQ(TABLE_OK, table_replace(&curve, &replacement));
  EXPECT_EQ(20, axis[1]);
  EXPECT_EQ(30, cells[2]);
}

TEST(ErrorsC, LeaveLookupOutputUnchanged) {
  const float axis[] = {0.0F, 1.0F};
  const float cells[] = {2.0F, 4.0F};
  table_view_t curve{};
  ASSERT_EQ(TABLE_OK, table_curve_view_init(&curve, axis, 2u, TABLE_SCALAR_F32, cells, TABLE_SCALAR_F32));
  float output = 17.0F;
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_curve_lookup(&curve, std::numeric_limits<float>::quiet_NaN(), &output));
  EXPECT_FLOAT_EQ(17.0F, output);
  EXPECT_EQ(TABLE_INDEX_OUT_OF_RANGE, table_curve_get_cell(&curve, 2u, nullptr));
}

TEST(ScalarTypesC, ReadsAndWritesEverySupportedRepresentation) {
  exerciseScalarType<std::int8_t>(TABLE_SCALAR_I8, -2, 3);
  exerciseScalarType<std::uint8_t>(TABLE_SCALAR_U8, 2u, 3u);
  exerciseScalarType<std::int16_t>(TABLE_SCALAR_I16, -20, 30);
  exerciseScalarType<std::uint16_t>(TABLE_SCALAR_U16, 20u, 30u);
  exerciseScalarType<std::int32_t>(TABLE_SCALAR_I32, -200, 300);
  exerciseScalarType<std::uint32_t>(TABLE_SCALAR_U32, 200u, 300u);
  exerciseScalarType<float>(TABLE_SCALAR_F32, -2.5F, 3.5F);
}

TEST(ApiC, CoversMapAccessMutationAndReplacement) {
  std::int16_t xAxis[] = {0, 10};
  std::uint16_t yAxis[] = {0u, 20u};
  float cells[] = {1.0F, 2.0F, 3.0F, 4.0F};
  table_mutable_t map{};
  ASSERT_EQ(TABLE_OK, table_map_init(&map, xAxis, 2u, TABLE_SCALAR_I16, yAxis, 2u, TABLE_SCALAR_U16, cells, TABLE_SCALAR_F32));

  table_scalar_t value{};
  EXPECT_EQ(TABLE_OK, table_get_y(&map.view, 1u, &value));
  EXPECT_EQ(20u, value.value.u16);
  EXPECT_EQ(TABLE_OK, table_map_get_cell(&map.view, 1u, 1u, &value));
  EXPECT_FLOAT_EQ(4.0F, value.value.f32);
  EXPECT_EQ(TABLE_OK, table_set_x(&map, 1u, scalar<std::int16_t>(TABLE_SCALAR_I16, 15)));
  EXPECT_EQ(TABLE_OK, table_set_y(&map, 1u, scalar<std::uint16_t>(TABLE_SCALAR_U16, 25u)));
  EXPECT_EQ(TABLE_OK, table_map_set_cell(&map, 0u, 1u, scalar<float>(TABLE_SCALAR_F32, 5.0F)));

  const std::int16_t replacementX[] = {-10, 10};
  const std::uint16_t replacementY[] = {5u, 30u};
  const float replacementCells[] = {6.0F, 7.0F, 8.0F, 9.0F};
  table_view_t replacement{};
  ASSERT_EQ(TABLE_OK, table_map_view_init(&replacement, replacementX, 2u, TABLE_SCALAR_I16, replacementY, 2u, TABLE_SCALAR_U16, replacementCells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_OK, table_replace(&map, &replacement));
  EXPECT_EQ(-10, xAxis[0]);
  EXPECT_EQ(30u, yAxis[1]);
  EXPECT_FLOAT_EQ(8.0F, cells[2]);
}

TEST(ErrorsC, RejectsInvalidDescriptorsAndMutations) {
  const std::int16_t axis[] = {0, 10};
  const float cells[] = {1.0F, 2.0F};
  table_view_t view{};
  table_mutable_t mutableTable{};
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_validate(nullptr));
  EXPECT_EQ(TABLE_INVALID_STATE, table_validate(&view));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_curve_view_init(nullptr, axis, 2u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_map_view_init(nullptr, axis, 2u, TABLE_SCALAR_I16, axis, 2u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_F32));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_curve_init(nullptr, nullptr, 0u, TABLE_SCALAR_UNSPECIFIED, nullptr, TABLE_SCALAR_UNSPECIFIED));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_map_init(nullptr, nullptr, 0u, TABLE_SCALAR_UNSPECIFIED, nullptr, 0u, TABLE_SCALAR_UNSPECIFIED, nullptr, TABLE_SCALAR_UNSPECIFIED));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_curve_lookup(nullptr, 0.0F, nullptr));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_map_lookup(nullptr, 0.0F, 0.0F, nullptr));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_get_x(nullptr, 0u, nullptr));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_set_x(nullptr, 0u, i16(0)));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_set_y(nullptr, 0u, i16(0)));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_replace(nullptr, &view));
  EXPECT_EQ(TABLE_INVALID_ARGUMENT, table_replace(&mutableTable, nullptr));

  ASSERT_EQ(TABLE_OK, table_curve_view_init(&view, axis, 2u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_F32));
  float output = 0.0F;
  table_scalar_t value{};
  EXPECT_EQ(TABLE_INVALID_STATE, table_map_lookup(&view, 0.0F, 0.0F, &output));
  EXPECT_EQ(TABLE_INVALID_STATE, table_get_y(&view, 0u, nullptr));
  EXPECT_EQ(TABLE_INDEX_OUT_OF_RANGE, table_get_x(&view, 2u, &value));
}

} // namespace
