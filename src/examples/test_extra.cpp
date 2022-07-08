#include "../Table.h"

/**
 * Cpp example of Table.h
 * 
 * Used for debugging and testing the library during development.
 */

#include <iostream>

//Go through the x rows and add the column values
const int tempRow1[5] = {50, 100, 150, 200, 250};


int main(int argc, char **argv) {
    constexpr auto xSize = 5;

    Table<std::uint32_t, xSize> map;

    std::cout << "Table library" << std::endl;
    map.resetData(); // reset table heap
    std::cout << "DataSize: " << std::to_string(map.getDataSize()) << std::endl;
    map.initilise();

    // Axis data
    int tempXAxis[5] = {0, 50, 60, 75, 100};
    for (char x = 0; x< xSize; x++) { map.axisX[x] = tempXAxis[x]; }
    
    // Table data
    for (char x = 0; x< xSize; x++) { map.setValueByIndex(x, tempRow1[x]); }

    std::cout << "Get value: x=0, r=" << std::to_string(map.getValue(0)) << std::endl;
    std::cout << "Get value: x=50, r=" << std::to_string(map.getValue(50)) << std::endl;
    std::cout << "Get value: x=25, r=" << std::to_string(map.getValue(60)) << std::endl;
    std::cout << "Get value: x=75, r=" << std::to_string(map.getValue(75)) << std::endl;
    std::cout << "Get value: x=100, r=" << std::to_string(map.getValue(100)) << std::endl;

    std::cout << "Get value: x=25, r=" << std::to_string(map.getValue(25)) << std::endl;
    // Will return the cached version
    std::cout << "Get value: x=25, r=" << std::to_string(map.getValue(25)) << std::endl;

    // Edge
    std::cout << "Get value: x=0, y=25, r=" << std::to_string(map.getValue(0,25)) << std::endl;
    std::cout << "Get value: x=80, y=100, r=" << std::to_string(map.getValue(80,100)) << std::endl;

    // Over
    std::cout << "Get value: x=80, y=300, r=" << std::to_string(map.getValue(80,300)) << std::endl;

    return 0;
}
