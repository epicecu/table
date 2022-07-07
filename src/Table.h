#ifndef TABLE_H
#define TABLE_H

/**
 * Table implementation with bilinear interpolation support between points.
 * Author: David Cedar
 * Email: david@epicecu.com
 * URL: https://github.com/epicecu/table
 */
template<typename T, unsigned int xSize, unsigned int ySize = 1>
class Table {
public:
    // 3d table
    void initilise()
    {
        this->dataPointer = 0;
        this->values = (T*)allocData(xSize * ySize * sizeof(T));
        this->axisX = (int*)allocData(xSize * sizeof(int));
        this->axisY = (int*)allocData(ySize * sizeof(int));
        this->cacheIsValid = false;
    }
    // 3d table
    double getValue(const int X_in, const int Y_in)
    {
        double tableResult = -1;

        // Check if requesting over bounds
        if(X_in > axisX[xSize-1] || Y_in > axisY[ySize-1] || X_in < axisX[0] || Y_in < axisY[0]){
           return tableResult; 
        }

        // Load cache
        if(X_in == lastX_in && Y_in == lastY_in){
            if(cacheIsValid){
                return lastOutput;
            }
        }else{
            cacheIsValid = false;
        }

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
            unsigned int xMinIdx=0, yMinIdx=0, xMaxIdx=0, yMaxIdx=0; 
            int xMin=0, yMin=0, xMax=0, yMax=0;
            // x-axis
            for (unsigned int i = 0; i < xSize; i++){
                if(X_in > axisX[i]){
                    if(i+1 < xSize){
                        if(X_in < axisX[i+1]){
                            // Found the bounds in x axis
                            xMinIdx = i;
                            xMaxIdx = i+1;
                        }
                    }
                }
                else if(X_in == axisX[i]){
                    if(i == xMax-1){
                        xMinIdx = i-1;
                        xMaxIdx = i;
                    }else{
                        xMinIdx = i;
                        xMaxIdx = i+1;
                    }
                }
                xMin = axisX[xMinIdx];
                xMax = axisX[xMaxIdx];
            }
            // y-axis
            for (unsigned int i = 0; i < ySize; i++){
                if(Y_in > axisY[i]){
                    if(i+1 < ySize){
                        if(Y_in < axisY[i+1]){
                            // Found the bounds in x axis
                            yMinIdx = i;
                            yMaxIdx = i+1;
                        }
                    }
                }
                else if(Y_in == axisY[i]){
                    if(i == yMax-1){
                        yMinIdx = i-1;
                        yMaxIdx = i;
                    }else{
                        yMinIdx = i;
                        yMaxIdx = i+1;
                    }
                }
                yMin = axisY[yMinIdx];
                yMax = axisY[yMaxIdx];
            }
            double Q11 = getValueByIndex(xMinIdx, yMinIdx);
            double Q12 = getValueByIndex(xMinIdx, yMaxIdx);
            double Q21 = getValueByIndex(xMaxIdx, yMinIdx);
            double Q22 = getValueByIndex(xMaxIdx, yMaxIdx);

            tableResult = biLinearInterpolation(Q11, Q12, Q21, Q22, xMin, xMax, yMin, yMax, X_in, Y_in);

        }

        // Cache result
        lastOutput = tableResult;
        lastX_in = X_in;
        lastY_in = Y_in;
        cacheIsValid = true;

        return tableResult;
    }

    /** Bi-Linear Interpolation Alg
    * 
    *   |
    * y2|-Q12----R2----Q22
    *   |  !     !      !
    *  y|--------P--------
    *   |  !     !      !
    * y1|-Q11----R1----Q21
    *   |__!_____!______!_
    *      x1     x    x2
    */
    static double biLinearInterpolation(const double q11, const double q12, const double q21, const double q22, const int x1, const int x2, const int y1, const int y2, const int x, const int y) 
    {
        int x2x1 = x2 - x1;
        int y2y1 = y2 - y1;
        int x2x = x2 - x;
        int y2y = y2 - y;
        int yy1 = y - y1;
        int xx1 = x - x1;
        return 1.0 / (x2x1 * y2y1) * (
            q11 * x2x * y2y +
            q21 * xx1 * y2y +
            q12 * x2x * yy1 +
            q22 * xx1 * yy1
        );
    }

    // 2d table
    double getValue(const int X_in){
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

    static constexpr int getDataSize(){
        // Values, xAxis, yAxis
        return xSize*ySize*sizeof(T) + xSize*sizeof(int) + ySize*sizeof(int) + 1;
    }
    char data[getDataSize()] = {0};
    void resetData(){
        for(unsigned int i = 0; i < getDataSize(); i++) data[i] = 0x00;
        dataPointer = 0;
    }

    void invalidateCache(){
        cacheIsValid = false;
    }
    
private:
    unsigned int dataPointer = 0;
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

private:
    // caching
    int lastX_in;
    int lastY_in;
    double lastOutput;
    bool cacheIsValid;
};

#endif // TABLE_H