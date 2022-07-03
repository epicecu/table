#include "tests_tables.h"
#include <unity.h>
#include "Table.h"
#include <cstdio>

Table<uint8_t> testMap;

void setup_testMap(void)
{
  //Setup the 3d table with some sane values for testing
  //Table is setup per the below
  /*
  40  |   70 |   70 |   90 |   100
  30  |   65 |   85 |   85 |   90
  20  |   55 |   60 |   85 |   70
  10  |   40 |   55 |   65 |   70
      ----------------------------
          10 |   20 |   30 |   40
  */
  reset_heap(); // reset table heap
  testMap.initilise(4,4);
  TEST_ASSERT_EQUAL(testMap.xSize, 4);
  TEST_ASSERT_EQUAL(testMap.ySize, 4);
  
  int tempXAxis[4] = {10, 20, 30, 40};
  for (char x = 0; x< testMap.xSize; x++) { testMap.axisX[x] = tempXAxis[x]; }
  int tempYAxis[4] = {10, 20, 30, 40};
  for (char y = 0; y< testMap.ySize; y++) { testMap.axisY[y] = tempYAxis[y]; }

  for (char x = 0; x< testMap.xSize; x++) { testMap.values[0][x] = tempRow1[x]; }
  for (char x = 0; x< testMap.xSize; x++) { testMap.values[1][x] = tempRow2[x]; }
  for (char x = 0; x< testMap.xSize; x++) { testMap.values[2][x] = tempRow3[x]; }
  for (char x = 0; x< testMap.xSize; x++) { testMap.values[3][x] = tempRow4[x]; }
  
}

void run_tests()
{
  UNITY_BEGIN(); // IMPORTANT LINE!
  RUN_TEST(test_table_values);
  RUN_TEST(test_tableLookup_50pct);
  RUN_TEST(test_tableLookup_exact1Axis);
  RUN_TEST(test_tableLookup_exact2Axis);
  RUN_TEST(test_tableLookup_overMaxX);
  RUN_TEST(test_tableLookup_overMaxY);
  RUN_TEST(test_tableLookup_underMinX);
  RUN_TEST(test_tableLookup_underMinY);
//  RUN_TEST(test_all_incrementing);
  UNITY_END(); // stop unit testing
  
}

void test_table_values(void)
{
  setup_testMap();

  TEST_ASSERT_EQUAL(testMap.values[0][0], 40);
  TEST_ASSERT_EQUAL(testMap.values[2][2], 85);
  TEST_ASSERT_EQUAL(testMap.values[3][3], 100);
}

void test_tableLookup_50pct(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  setup_testMap();

  int x_axis = 15;
  int y_axis = 15;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 52);
}

void test_tableLookup_exact1Axis(void)
{
  //Tests a lookup that exactly matches on the X axis and 50% of the way between cells on the Y axis
  setup_testMap();

  int x_axis = 20;
  int y_axis = 20;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 52);
}

void test_tableLookup_exact2Axis(void)
{
  setup_testMap();

  int x_axis = 2500;
  int y_axis = 70;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 86);
}

void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the x_axis exceeds the highest value in the table. The Y value is a 50% match
  setup_testMap();

  int x_axis = 10000;
  int y_axis = 73;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 86);
}

void test_tableLookup_overMaxY(void)
{
  //Tests a lookup where the load value exceeds the highest value in the table. The X value is a 50% match
  setup_testMap();

  int x_axis = 600;
  int y_axis = 110;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 110);
}

void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the x_axis value is below the lowest value in the table. The Y value is a 50% match
  setup_testMap();

  int x_axis = 300;
  int y_axis = 38;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 37);
}

void test_tableLookup_underMinY(void)
{
  //Tests a lookup where the load value is below the lowest value in the table. The X value is a 50% match
  setup_testMap();

  int x_axis = 600;
  int y_axis = 8;

  uint8_t value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(value, 34);
}

void test_all_incrementing(void)
{
  //Test the when going up both the load and x_axis axis that the returned value is always equal or higher to the previous one
  //Tests all combinations of load/x_axis from between 0-200 load and 0-9000 x_axis
  //WARNING: This can take a LONG time to run. It is disabled by default for this reason
  setup_testMap();

  uint8_t tempVE = 0;
  
  for(uint8_t x_axis = 0; x_axis<8000; x_axis+=100)
  {
    tempVE = 0;
    for(uint8_t load = 0; load<120; load++)
    {
      uint8_t value = testMap.getValue(x_axis, load);
      TEST_ASSERT_GREATER_OR_EQUAL(tempVE, value);
      tempVE = value;
    }
  }
}

int main(int argc, char **argv) {
  run_tests();
  return 0;
}
