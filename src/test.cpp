#include "Table.h"

/**
 * Cpp example of Table.h
 * 
 * Used for debugging and testing the library during development.
 */

#include <iostream>

//Go through the 8 rows and add the column values
const int tempRow5[5] = {10, 60, 110, 160, 210};
const int tempRow4[5] = {20, 70, 120, 170, 220};
const int tempRow3[5] = {30, 80, 130, 180, 230};
const int tempRow2[5] = {40, 90, 140, 190, 240};
const int tempRow1[5] = {50, 100, 150, 200, 250};


int main(int argc, char **argv) {
    constexpr auto xSize = 5;
    constexpr auto ySize = 5;

    Table<std::uint32_t, xSize, ySize> map;

    std::cout << "Table library" << std::endl;
    map.resetData(); // reset table heap
    std::cout << "DataSize: " << std::to_string(map.getDataSize()) << std::endl;
    map.initilise();

    // Axis data
    int tempXAxis[5] = {0, 50, 60, 75, 100};
    int tempYAxis[5] = {0, 50, 60, 75, 100};
    for (char x = 0; x< xSize; x++) { map.axisX[x] = tempXAxis[x]; }
    for (char y = 0; y< ySize; y++) { map.axisY[y] = tempYAxis[y]; }
    
    // Table data
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 0, tempRow1[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 1, tempRow2[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 2, tempRow3[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 3, tempRow4[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 4, tempRow5[x]); }

    std::cout << "Get value: x=0, y=0, r=" << std::to_string(map.getValue(0,0)) << std::endl;
    std::cout << "Get value: x=50, y=50, r=" << std::to_string(map.getValue(50,50)) << std::endl;
    std::cout << "Get value: x=25, y=25, r=" << std::to_string(map.getValue(60,60)) << std::endl;
    std::cout << "Get value: x=75, y=75, r=" << std::to_string(map.getValue(75,75)) << std::endl;
    std::cout << "Get value: x=100, y=100, r=" << std::to_string(map.getValue(100,100)) << std::endl;

    std::cout << "Get value: x=25, y=25, r=" << std::to_string(map.getValue(25,25)) << std::endl;
    // Will return the cached version
    std::cout << "Get value: x=25, y=25, r=" << std::to_string(map.getValue(25,25)) << std::endl;

    // Edge
    std::cout << "Get value: x=0, y=25, r=" << std::to_string(map.getValue(0,25)) << std::endl;
    std::cout << "Get value: x=80, y=100, r=" << std::to_string(map.getValue(80,100)) << std::endl;

    // Over
    std::cout << "Get value: x=80, y=300, r=" << std::to_string(map.getValue(80,300)) << std::endl;

    return 0;
}
