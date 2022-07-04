#ifndef TABLE_H
#define TABLE_H


/**
 * 3D Tables have an origin (0,0) in the top left hand corner. Vertical axis is expressed first.
 * Eg: 2x2 table
 * 
 *   -----
 *   |2 7|
 *   |1 4|
 *   -----
 *
 *   (0,1) = 7
 *   (0,0) = 2
 *   (1,0) = 1
 *
 */
template<typename T, unsigned int xSize, unsigned int ySize = 1>
class Table {
public:
    // 3d table
    void initilise()
    {
        this->values = (T*)allocData(xSize * ySize * sizeof(T));
        this->axisX = (int*)allocData(xSize * sizeof(int));
        this->axisY = (int*)allocData(ySize * sizeof(int));
        this->lastXInput = 1; // some what hacky...
        this->lastYInput = 1;
        this->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
    }
    // 3d table
    T getValue(const int X_in, const int Y_in)
    {
        int X = X_in;
        int Y = Y_in;

        T tableResult = 0;

        // Check if requesting a direct value
        int x = -1;
        int y = -1;
        for (unsigned int i = 0; i < xSize; i++){
            if(X_in == axisX[i]) x = i;
        }
        for (unsigned int i = 0; i < ySize; i++){
            if(Y_in == axisY[i]) y = i;
        }
        // Direct cell found, return the value
        if (x >= 0 && y >= 0){

            tableResult = getValueByIndex(x, y);
        
        // Interpolation is required
        }else{

            // TODO: Implement interpolation routine
        }

        return tableResult;
    }

    // 2d table
    T getValue(const int X_in){
        return getValue(X_in, 1);
    }

    void setValue(const int X_in, const int Y_in, const T value)
    {
        int x = -1;
        int y = -1;
        for (unsigned int i = 0; i < xSize; i++){
            if(X_in == axisX[i]) x = i;
        }
        for (unsigned int i = 0; i < ySize; i++){
            if(Y_in == axisY[i]) y = i;
        }
        // Direct cell found, return the direct value
        if (x >= 0 && y >= 0){
            setValueByIndex(x, y, value);
        }
    }

    // Set value at x & y cell
    void setValueByIndex(const int x, const int y, const T value){
        if(x < xSize && y < ySize){
            values[x * ySize + y] = value;
        }
    }

    T getValueByIndex(const int x, const int y){
        return values[x * ySize + y];
    }

public:
    static constexpr int getDataSize(){
        // Values, xAxis, yAxis
        return xSize*ySize*sizeof(T) + xSize*sizeof(int) + ySize*sizeof(int) + 1;
    }
    char data[getDataSize()] = {0};
    void resetData(){
        for(unsigned int i = 0; i < getDataSize(); i++) data[i] = 0x00;
        dataPointer = 0;
    }
    

    unsigned int dataPointer = 0;
private:
    constexpr void* allocData(unsigned int size){
        char* ptr = nullptr;
        if(size < getDataSize()-dataPointer){
            ptr = &data[dataPointer];
        }
        dataPointer+=size;
        return ptr;
    }

public:
    // table values
    T* values; // x * ySize + y
    int* axisX;
    int* axisY;
    // caching
    T lastXMax, lastXMin;
    T lastYMax, lastYMin;
    // caching
    int lastXInput, lastYInput;
    T lastOutput;
    bool cacheIsValid; ///< This tracks whether the tables cache should be used. Ordinarily this is true, but is set to false whenever TunerStudio sends a new value for the table
};

#endif // TABLE_H