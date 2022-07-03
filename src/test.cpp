#include "Table.h"

#include <iostream>

//Go through the 8 rows and add the column values
const char tempRow1[5] = {4, 8, 9, 16, 20};
const char tempRow2[5] = {3, 7, 10, 15, 19};
const char tempRow3[5] = {2, 6, 11, 14, 18};
const char tempRow4[5] = {1, 5, 12, 13, 17};


int main(int argc, char **argv) {
    constexpr auto xSize = 5;
    constexpr auto ySize = 5;

    Table<std::uint8_t, xSize, ySize> map;

    std::cout << "Table library" << std::endl;
    reset_heap(); // reset table heap
    
    map.initilise();

    // Axis data
    int tempXAxis[5] = {0, 25, 50, 75, 100};
    int tempYAxis[5] = {0, 25, 50, 75, 100};
    for (char x = 0; x< xSize; x++) { map.axisX[x] = tempXAxis[x]; }
    for (char y = 0; y< ySize; y++) { map.axisY[y] = tempYAxis[y]; }
    
    // Table data
    for (char x = 0; x< xSize; x++) { map.values[0][x] = tempRow1[x]; }
    for (char x = 0; x< xSize; x++) { map.values[1][x] = tempRow2[x]; }
    for (char x = 0; x< xSize; x++) { map.values[2][x] = tempRow3[x]; }
    for (char x = 0; x< xSize; x++) { map.values[3][x] = tempRow4[x]; }

    std::cout << "Get value: x=0, y=0, r=" << std::to_string(map.getValue(0,0)) << std::endl;
    std::cout << "Get value: x=25, y=25, r=" << std::to_string(map.getValue(25,25)) << std::endl;
    std::cout << "Get value: x=50, y=50, r=" << std::to_string(map.getValue(50,50)) << std::endl;
    std::cout << "Get value: x=75, y=75, r=" << std::to_string(map.getValue(75,75)) << std::endl;
    std::cout << "Get value: x=100, y=100, r=" << std::to_string(map.getValue(100,100)) << std::endl;


    return 0;
}
