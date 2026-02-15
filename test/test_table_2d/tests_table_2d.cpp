#include "tests_table_2d.h"

#include "Table.h"

Table<uint8_t, xSize> testMap;

void setup_testMap(void)
{
  //Setup the 3d table with some sane values for testing
  //Table is setup per the below
  /*

  na  |   20 |   40 |   80 |   85 |   90
      -----------------------------------
           0 |   20 |   40 |   60 |   80
  */
  testMap.initialise();
  
  // Axis
  constexpr int tempXAxis[xSize] = {0, 20, 40, 60, 80};
  for (char x = 0; x< xSize; x++) { testMap.axisX[x] = tempXAxis[x]; }
  // Cells
  for (char x = 0; x< xSize; x++) { testMap.setValueByIndex(x, 0, tempRow1[x]); }

  
}

void run_tests()
{
  UNITY_BEGIN(); // IMPORTANT LINE!
  RUN_TEST(test_table_values);
  RUN_TEST(test_tableLookup_50pct);
  RUN_TEST(test_tableLookup_exactAxis);
  RUN_TEST(test_tableLookup_overMaxX);
  RUN_TEST(test_tableLookup_underMinX);
  UNITY_END(); // stop unit testing
  
}

void test_table_values(void)
{
  setup_testMap();

  TEST_ASSERT_EQUAL(20, testMap.getValueByIndex(0));
  TEST_ASSERT_EQUAL(40, testMap.getValueByIndex(1));
  TEST_ASSERT_EQUAL(80, testMap.getValueByIndex(2));
  TEST_ASSERT_EQUAL(85, testMap.getValueByIndex(3));
  TEST_ASSERT_EQUAL(90, testMap.getValueByIndex(4));
}

void test_tableLookup_50pct(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  setup_testMap();

  constexpr int x_axis = 30;

  double value = testMap.getValue(x_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(60, value);
}

void test_tableLookup_exactAxis(void)
{
  //Tests a lookup that exactly matches on the X axis
  setup_testMap();

  constexpr int x_axis = 20;

  double value = testMap.getValue(x_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(40, value);
}

void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the x_axis exceeds the highest value in the table.
  setup_testMap();

  constexpr int x_axis = 10000;

  double value = testMap.getValue(x_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(-1, value);
}

void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the x_axis value is below the lowest value in the table. The Y value is a 50% match
  setup_testMap();

  constexpr int x_axis = -10;

  double value = testMap.getValue(x_axis); //Perform lookup into fuel map for x_axis vs MAP value
  TEST_ASSERT_EQUAL(-1, value);
}

void setUp (void) {}

void tearDown (void) {}

int main(int argc, char **argv) {
  run_tests();
  return 0;
}
