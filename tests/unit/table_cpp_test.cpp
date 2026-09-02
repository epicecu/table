#include "table/table.hpp"

#include <gtest/gtest.h>

#include <cstdint>

namespace {

TEST(CurveCpp, OwnsStorageAndDelegatesLookup) {
  table::Curve<std::uint8_t, 3u, std::int16_t> curve;
  const std::int16_t axis[] = {0, 10, 20};
  const std::uint8_t cells[] = {0, 20, 40};
  ASSERT_EQ(table::Status::Ok, curve.init(axis, cells));
  float output = 0.0F;
  EXPECT_EQ(table::Status::Ok, curve.lookup(15.0F, output));
  EXPECT_FLOAT_EQ(30.0F, output);
  EXPECT_EQ(table::Status::Ok, curve.setCell(1u, 30u));
  EXPECT_EQ(table::Status::Ok, curve.lookup(10.0F, output));
  EXPECT_FLOAT_EQ(30.0F, output);
}

TEST(MapCpp, AcceptsTwoDimensionalCells) {
  table::Map<float, 2u, 2u, std::uint16_t, std::uint16_t> map;
  const std::uint16_t xAxis[] = {0u, 10u};
  const std::uint16_t yAxis[] = {0u, 10u};
  const float cells[2][2] = {{0.0F, 10.0F}, {20.0F, 30.0F}};
  ASSERT_EQ(table::Status::Ok, map.init(xAxis, yAxis, cells));
  float output = 0.0F;
  EXPECT_EQ(table::Status::Ok, map.lookup(5.0F, 5.0F, output));
  EXPECT_FLOAT_EQ(15.0F, output);
  EXPECT_TRUE(map.isReady());
  EXPECT_NE(nullptr, map.nativeView());
  EXPECT_EQ(table::Status::Ok, map.setX(1u, 12u));
  EXPECT_EQ(table::Status::Ok, map.setY(1u, 12u));
  EXPECT_EQ(table::Status::Ok, map.setCell(1u, 1u, 36.0F));
  std::uint16_t axisValue = 0u;
  EXPECT_EQ(table::Status::Ok, map.x(1u, axisValue));
  EXPECT_EQ(12u, axisValue);
  EXPECT_EQ(table::Status::Ok, map.y(1u, axisValue));
  EXPECT_EQ(12u, axisValue);
  float cell = 0.0F;
  EXPECT_EQ(table::Status::Ok, map.cell(1u, 1u, cell));
  EXPECT_FLOAT_EQ(36.0F, cell);
}

TEST(CppErrors, RejectsUseBeforeInitAndInvalidReinitialisation) {
  table::Curve<float, 2u> curve;
  float output = 9.0F;
  EXPECT_EQ(table::Status::InvalidState, curve.lookup(0.0F, output));
  const std::int32_t goodAxis[] = {0, 10};
  const float goodCells[] = {1.0F, 2.0F};
  ASSERT_EQ(table::Status::Ok, curve.init(goodAxis, goodCells));
  const std::int32_t badAxis[] = {10, 0};
  const float badCells[] = {3.0F, 4.0F};
  EXPECT_EQ(table::Status::InvalidAxis, curve.init(badAxis, badCells));
  EXPECT_EQ(table::Status::Ok, curve.lookup(0.0F, output));
  EXPECT_FLOAT_EQ(1.0F, output);
}

TEST(MapCpp, AcceptsFlatCellsAndValidReinitialisation) {
  table::Map<std::int32_t, 2u, 2u, std::int8_t, std::uint8_t> map;
  EXPECT_FALSE(map.isReady());
  EXPECT_EQ(nullptr, map.nativeView());
  float output = 0.0F;
  std::int8_t xValue = 0;
  std::uint8_t yValue = 0u;
  std::int32_t cellValue = 0;
  EXPECT_EQ(table::Status::InvalidState, map.lookup(0.0F, 0.0F, output));
  EXPECT_EQ(table::Status::InvalidState, map.setX(0u, 0));
  EXPECT_EQ(table::Status::InvalidState, map.setY(0u, 0u));
  EXPECT_EQ(table::Status::InvalidState, map.setCell(0u, 0u, 0));
  EXPECT_EQ(table::Status::InvalidState, map.x(0u, xValue));
  EXPECT_EQ(table::Status::InvalidState, map.y(0u, yValue));
  EXPECT_EQ(table::Status::InvalidState, map.cell(0u, 0u, cellValue));

  const std::int8_t xAxis[] = {-10, 10};
  const std::uint8_t yAxis[] = {0u, 20u};
  const std::int32_t cells[] = {1, 2, 3, 4};
  ASSERT_EQ(table::Status::Ok, map.init(xAxis, yAxis, cells));
  const std::int8_t replacementX[] = {-20, 20};
  const std::uint8_t replacementY[] = {5u, 25u};
  const std::int32_t replacementCells[] = {5, 6, 7, 8};
  EXPECT_EQ(table::Status::Ok, map.init(replacementX, replacementY, replacementCells));
  EXPECT_EQ(table::Status::Ok, map.lookup(20.0F, 25.0F, output));
  EXPECT_FLOAT_EQ(8.0F, output);
}

} // namespace
