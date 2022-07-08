#include <unity.h>
#include <cstdio>

void run_tests();
void setup_testMap(void);
void test_table_values(void);
void test_tableLookup_50pct(void);
void test_tableLookup_exact1Axis(void);
void test_tableLookup_exact2Axis(void);
void test_tableLookup_overMaxX(void);
void test_tableLookup_overMaxY(void);
void test_tableLookup_underMinX(void);
void test_tableLookup_underMinY(void);
void test_all_incrementing(void);
void test_setValue(void);
void test_setValueNonDirect(void);

constexpr unsigned int xSize = 4;
constexpr unsigned int ySize = 4;

constexpr uint8_t tempRow4[xSize] = {20, 25, 60, 65};
constexpr uint8_t tempRow3[xSize] = {15, 30, 55, 70};
constexpr uint8_t tempRow2[xSize] = {10, 35, 50, 75};
constexpr uint8_t tempRow1[xSize] = {5, 40, 45, 80};