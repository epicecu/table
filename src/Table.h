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
        resetData();
        this->values = (T*)allocData(xSize * ySize * sizeof(T));
        this->axisX = (int*)allocData(xSize * sizeof(int));
        this->axisY = (int*)allocData(ySize * sizeof(int));
        this->cacheIsValid = false;
        if(ySize == 1) this->axisY[0] = 1;
    }
    
    /**
     * Get table value by x,y axis value/s
     */
    double getValue(const int X_in, const int Y_in)
    {
        double tableResult = -1;

        // Check if the table has been validated
        if(!valid){
            return tableResult;
        }

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
                }else if(X_in == axisX[i] && xSize == 1){
                    xMinIdx = i;
                    xMaxIdx = i;
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
                else if(Y_in == axisY[i] && ySize > 1){
                    if(i == yMax-1){
                        yMinIdx = i-1;
                        yMaxIdx = i;
                    }else{
                        yMinIdx = i;
                        yMaxIdx = i+1;
                    }
                }else if(Y_in == axisY[i] && ySize == 1){
                    yMinIdx = i;
                    yMaxIdx = i;
                }
                yMin = axisY[yMinIdx];
                yMax = axisY[yMaxIdx];
            }
            double Q11 = getValueByIndex(xMinIdx, yMinIdx);
            double Q12 = getValueByIndex(xMinIdx, yMaxIdx);
            double Q21 = getValueByIndex(xMaxIdx, yMinIdx);
            double Q22 = getValueByIndex(xMaxIdx, yMaxIdx);

            if(Q11 == Q12 && Q21 == Q22){
                // 2d interpolation in a (x, 1) sized table
                tableResult = linearInterpolation(Q11, Q21, xMin, xMax, X_in);
            }else if(Q11 == Q21 && Q12 == Q22){
                // 2d interpolation in a (1, y) sized table
                tableResult = linearInterpolation(Q11, Q12, yMin, yMax, Y_in);
            }else{
                // 3d interpolation
                tableResult = biLinearInterpolation(Q11, Q12, Q21, Q22, xMin, xMax, yMin, yMax, X_in, Y_in);
            }

        }

        // Cache result
        lastOutput = tableResult;
        lastX_in = X_in;
        lastY_in = Y_in;
        cacheIsValid = true;

        return tableResult;
    }

    double getValue(const int X_in){
        return getValue(X_in, 1);
    }

    /**
     * Set table values by x,y axis value/s
     */ 
    bool setValue(const int X_in, const int Y_in, const T value)
    {
        bool result = false;
        // check if table is valid
        if(valid){
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
                result = true;
            }
        }
        return result;
    }

    bool setValue(const int X_in, const T value){
        bool result = false;
        // check if table is valid
        if(valid){
            int x = -1;
            for (unsigned int i = 0; i < xSize; i++){
                if(X_in == axisX[i]) x = i;
            }
            // Direct cell found, return the direct value
            if (x >= 0){
                setValueByIndex(x, value);
                result = true;
            }
        }
        return result;
    }

    /**
     * Set Value 
     */
    void setValueByIndex(const int x, const int y, const T value){
        if(x < xSize && y < ySize){
            values[x * ySize + y] = value;
        }
    }

    void setValueByIndex(const int x, const T value){
        setValueByIndex(x, 0, value);
    }

    /// Get Value
    /// @brief Gets the table value by column & row indexes
    /// @param x column index
    /// @param y row index
    /// @return Table value in cell x, y
    T getValueByIndex(const int x, const int y){
        return values[x * ySize + y];
    }

    /// Get Value
    /// @brief Gets the table value by column & row indexes
    /// @param x column index
    /// @return Table value in cell x
    T getValueByIndex(const int x){
        return getValueByIndex(x, 0);
    }

    /// Validate Table
    /// @brief Validates the table axis values increase linearly
    /// @return True if the table axis values are valid
    bool validate(){
        bool valid = true;
        int lastValue = 0;
        bool first = true;
        // Check x axis
        for (unsigned int i = 0; i < xSize; i++){
            if(first){
                lastValue = axisX[i];
                first = false;
            }else{
                // the forward value is smaller then the previous
                if(axisX[i] < lastValue){
                    valid = false;
                }else{
                    lastValue = axisX[i];
                }
            }
        }
        // check y axis
        lastValue = 0;
        first = true;
        for (unsigned int i = 0; i < ySize; i++){
            if(first){
                lastValue = axisY[i];
            }else{
                // the forward value is smaller then the previous
                if(axisY[i] < lastValue){
                    valid = false;
                }else{
                    lastValue = axisY[i];
                }
            }
        }
        // remember result
        this->valid = valid;
        // finally
        return valid;
    }

    /**
     * Data - Public
     */
    static constexpr int getDataSize(){
        // Values, xAxis, yAxis
        return xSize*ySize*sizeof(T) + xSize*sizeof(int) + ySize*sizeof(int) + 1;
    }
    char data[getDataSize()] = {0};
    void resetData(){
        for(unsigned int i = 0; i < getDataSize(); i++) data[i] = 0x00;
        dataPointer = 0;
    }

    /**
     * Other
     */
    void invalidateCache(){
        cacheIsValid = false;
    }
    
private:
    /**
     * Data - Private
     */
    unsigned int dataPointer = 0;
    constexpr void* allocData(unsigned int size){
        char* ptr = nullptr;
        if(size < getDataSize()-dataPointer){
            ptr = &data[dataPointer];
        }
        dataPointer+=size;
        return ptr;
    }

        /** 
     * Bi-Linear Interpolation Alg
     * 
     *   |
     * y2|-Q12----R2----Q22
     *   |  !     !      !
     *  y|--------P--------
     *   |  !     !      !
     * y1|-Q11----R1----Q21
     *   |__!_____!______!_
     *      x1    x     x2
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

    /** 
     * y1|-Q11----R1----Q21
     *   |__!_____!______!_
     *      x1    x     x2
     */
    static double linearInterpolation(const double q11, const double q21, const int x1,  const int x2, const int x)
    {
        return q11 + ((q21-q11)/(x2-x1)) * (x-x1);
    }

public:
    // table values
    T* values; // x * ySize + y
    int* axisX;
    int* axisY;

private:
    bool valid = false;
    // caching
    int lastX_in;
    int lastY_in;
    double lastOutput;
    bool cacheIsValid;
};

#endif // TABLE_H