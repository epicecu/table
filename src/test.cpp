#include "Table.h"

#include <iostream>

//Go through the 8 rows and add the column values
const char tempRow5[5] = {1, 6, 11, 16, 21};
const char tempRow4[5] = {2, 7, 12, 17, 22};
const char tempRow3[5] = {3, 8, 13, 18, 23};
const char tempRow2[5] = {4, 9, 14, 19, 24};
const char tempRow1[5] = {5, 10, 15, 20, 25};


int main(int argc, char **argv) {
    constexpr auto xSize = 5;
    constexpr auto ySize = 5;

    Table<std::uint8_t, xSize, ySize> map;

    std::cout << "Table library" << std::endl;
    map.resetData(); // reset table heap
    std::cout << "DataSize: " << std::to_string(map.getDataSize()) << std::endl;
    map.initilise();
    std::cout << "DataPtr: " << std::to_string(map.dataPointer) << std::endl;

    // Axis data
    int tempXAxis[5] = {0, 25, 50, 75, 100};
    int tempYAxis[5] = {0, 25, 50, 75, 100};
    for (char x = 0; x< xSize; x++) { map.axisX[x] = tempXAxis[x]; }
    for (char y = 0; y< ySize; y++) { map.axisY[y] = tempYAxis[y]; }
    
    // Table data
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 0, tempRow1[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 1, tempRow2[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 2, tempRow3[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 3, tempRow4[x]); }
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, 4, tempRow5[x]); }

    std::cout << "Get value: x=0, y=0, r=" << std::to_string(map.getValue(0,0)) << std::endl;
    std::cout << "Get value: x=25, y=25, r=" << std::to_string(map.getValue(25,25)) << std::endl;
    std::cout << "Get value: x=50, y=50, r=" << std::to_string(map.getValue(50,50)) << std::endl;
    std::cout << "Get value: x=75, y=75, r=" << std::to_string(map.getValue(75,75)) << std::endl;
    std::cout << "Get value: x=100, y=100, r=" << std::to_string(map.getValue(100,100)) << std::endl;



    return 0;
}
