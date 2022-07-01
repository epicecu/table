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

//Go through the 8 rows and add the column values
const char tempRow4[4] = {70, 70, 90, 100};
const char tempRow3[4] = {65, 85, 85, 90};
const char tempRow2[4] = {55, 60, 85, 70};
const char tempRow1[4] = {40, 55, 65, 70};