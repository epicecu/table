/**
 * 
 * Example use of <Table.h>
 * 
 * By: David Cedar
 * URL: https://github.com/epicecu/table
 * 
 */

#include <Table.h>

// <Select the table cell data type, the x axis size, and the y axix size>
// The xaxis & yaxis data type is defined as int.
constexpr int xSize = 5;
constexpr int ySize = 5;
Table<int, xSize, ySize> table;

// A variable to read incoming serial data into
int incomingByte;
bool waitingByte=false;

void setup() {
    // initialize serial communication at 9600 bits per second:
    Serial.begin(9600);
    // Initilise the table object  
    table.initilise();

    constexpr int tempRow5[5] = {10, 60, 110, 160, 210};
    constexpr int tempRow4[5] = {20, 70, 120, 170, 220};
    constexpr int tempRow3[5] = {30, 80, 130, 180, 230};
    constexpr int tempRow2[5] = {40, 90, 140, 190, 240};
    constexpr int tempRow1[5] = {50, 100, 150, 200, 250};

    // Axis data
    int tempXAxis[5] = {0, 50, 60, 75, 100};
    int tempYAxis[5] = {0, 50, 60, 75, 100};
    for (char x = 0; x< xSize; x++) { table.axisX[x] = tempXAxis[x]; }
    for (char y = 0; y< ySize; y++) { table.axisY[y] = tempYAxis[y]; }
    
    // Table data
    for (char x = 0; x< xSize; x++) { table.setValueByIndex(x, 0, tempRow1[x]); }
    for (char x = 0; x< xSize; x++) { table.setValueByIndex(x, 1, tempRow2[x]); }
    for (char x = 0; x< xSize; x++) { table.setValueByIndex(x, 2, tempRow3[x]); }
    for (char x = 0; x< xSize; x++) { table.setValueByIndex(x, 3, tempRow4[x]); }
    for (char x = 0; x< xSize; x++) { table.setValueByIndex(x, 4, tempRow5[x]); }



}

void loop() {

    // see if there's incoming serial data:
    if (Serial.available() > 0) {
        
        // read the oldest byte in the serial buffer:
        if(!waitingByte){
            incomingByte = Serial.read();
        }

        // Set a value in the table
        if(incomingByte == 's'){
            
            incomingByte = Serial.read();
            table.setValue(50, 60, incomingByte);
            waitingByte = false;

        // Read a value from the table
        }else if(incomingByte == 'r'){

            table.getValue(50, 60);

        }
        
    }

}