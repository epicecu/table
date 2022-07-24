#include <unity.h>
#include <cstdio>

void run_tests();
void setup_testMap(bool validate = true);
void test_table_values(void);
void test_tableLookup_50pct(void);
void test_tableLookup_exactAxis(void);
void test_tableLookup_overMaxX(void);
void test_tableLookup_underMinX(void);
void test_all_incrementing(void);

constexpr unsigned int xSize = 5;

constexpr uint8_t tempRow1[xSize] = {20, 40, 80, 85, 90};