#include "tests_table_3d.h"

#include "Table.h"

#include <string.h>

Table<uint8_t, xSize, ySize> testMap;
Table<uint8_t, xSize/2, ySize/2> secondMap;

void setup_testMap(void)
{
  //Setup the 3d table with some sane values for testing
  //Table is setup per the below
  /*
  40  |   20 |   25 |   60 |   65
  30  |   15 |   30 |   55 |   70
  20  |   10 |   35 |   50 |   75
  10  |    5 |   40 |   45 |   80
      ----------------------------
          10 |   20 |   30 |   40
  */
  testMap.initialise();
  
  constexpr int tempXAxis[xSize] = {10, 20, 30, 40};
  for (char x = 0; x< xSize; x++) { testMap.axisX[x] = tempXAxis[x]; }
  constexpr int tempYAxis[ySize] = {10, 20, 30, 40};
  for (char y = 0; y< ySize; y++) { testMap.axisY[y] = tempYAxis[y]; }

  for (char x = 0; x< xSize; x++) { testMap.setValueByIndex(x, 0, tempRow1[x]); }
  for (char x = 0; x< xSize; x++) { testMap.setValueByIndex(x, 1, tempRow2[x]); }
  for (char x = 0; x< xSize; x++) { testMap.setValueByIndex(x, 2, tempRow3[x]); }
  for (char x = 0; x< xSize; x++) { testMap.setValueByIndex(x, 3, tempRow4[x]); }
  
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
  RUN_TEST(test_setValue);
  RUN_TEST(test_setValueNonDirect);
  RUN_TEST(test_copyData);
  UNITY_END(); // stop unit testing
  
}

void test_table_values(void)
{
  setup_testMap();

  TEST_ASSERT_EQUAL(5, testMap.getValueByIndex(0,0));
  TEST_ASSERT_EQUAL(35, testMap.getValueByIndex(1,1));
  TEST_ASSERT_EQUAL(55, testMap.getValueByIndex(2,2));
  TEST_ASSERT_EQUAL(65, testMap.getValueByIndex(3,3));
}

void test_tableLookup_50pct(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  setup_testMap();

  constexpr int x_axis = 15;
  constexpr int y_axis = 15;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(22.5, value);
}

void test_tableLookup_exact1Axis(void)
{
  //Tests a lookup that exactly matches on the X axis and 50% of the way between cells on the Y axis
  setup_testMap();

  constexpr int x_axis = 10;
  constexpr int y_axis = 15;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(7.5, value);
}

void test_tableLookup_exact2Axis(void)
{
  //Tests a lookup that exactly matches on the Y axis and 50% of the way between cells on the X axis
  setup_testMap();

  constexpr int x_axis = 35;
  constexpr int y_axis = 30;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(62.5, value);
}

void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the x_axis exceeds the highest value in the table. The Y value is a 50% match
  setup_testMap();

  constexpr int x_axis = 10000;
  constexpr int y_axis = 35;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(-1, value);
}

void test_tableLookup_overMaxY(void)
{
  //Tests a lookup where the load value exceeds the highest value in the table. The X value is a 50% match
  setup_testMap();

  constexpr int x_axis = 25;
  constexpr int y_axis = 100;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(-1, value);
}

void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the x_axis value is below the lowest value in the table. The Y value is a 50% match
  setup_testMap();

  constexpr int x_axis = -10;
  constexpr int y_axis = 35;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(-1, value);
}

void test_tableLookup_underMinY(void)
{
  //Tests a lookup where the load value is below the lowest value in the table. The X value is a 50% match
  setup_testMap();

  constexpr int x_axis = 25;
  constexpr int y_axis = -10;

  double value = testMap.getValue(x_axis, y_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(-1, value);
}

void test_setValue(void)
{
  setup_testMap();

  // Set value by direct axis values
  constexpr int x_axis = 20;
  constexpr int y_axis = 20;

  bool result = testMap.setValue(x_axis, y_axis, 57);

  // Check if returns the desired value
  TEST_ASSERT_TRUE(result);
  double value = testMap.getValue(x_axis, y_axis);
  TEST_ASSERT_EQUAL(57, value);
}

void test_setValueNonDirect()
{
  setup_testMap();

  // Set value by direct axis values
  constexpr int x_axis = 21;
  constexpr int y_axis = 20;

  bool result = testMap.setValue(x_axis, y_axis, 57);

  // Check if returns the desired value
  TEST_ASSERT_FALSE(result);
  double value = testMap.getValue(x_axis, y_axis);
  TEST_ASSERT_NOT_EQUAL(57, value);
}

void test_copyData()
{
  setup_testMap();

  // Set value by direct axis values
  constexpr int x_axis = 20;
  constexpr int y_axis = 20;

  bool result = testMap.setValue(x_axis, y_axis, 57);

  // copy data
  char copyData[testMap.getDataSize()] = {0};
  memcpy(&copyData, &testMap.data, testMap.getDataSize());
  
  // change value 
  result = testMap.setValue(x_axis, y_axis, 32);
  TEST_ASSERT_TRUE(result);
  double value = testMap.getValue(x_axis, y_axis);
  TEST_ASSERT_EQUAL(32, value);

  // copy data back...
  memcpy(&testMap.data, &copyData, testMap.getDataSize());
  // Clear cache
  testMap.invalidateCache();

  // the value should be the original value from the first setValue call
  value = testMap.getValue(x_axis, y_axis);
  TEST_ASSERT_EQUAL(57, value);

}

void setUp (void) {}

void tearDown (void) {}

int main(int argc, char **argv) {
  run_tests();
  return 0;
}