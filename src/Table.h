#ifndef EPICECU_TABLE_H
#define EPICECU_TABLE_H

/**
 * Table.
 * 
 * A Table implementation with bilinear interpolation support between points.
 * 
 * Author: David Cedar
 * Email: david@epicecu.com
 * URL: https://github.com/epicecu/table
 * 
 * Copyright © 2022 David Cedar. All rights reserved.
 * Licensed under the LGPL License (see LICENSE file)
 */
template<typename T, unsigned int xSize, unsigned int ySize = 1, typename XAxisT = int, typename YAxisT = int>
class Table {
public:
    /**
     * Initialises the Table object.
     */
    void initialise() {
        resetData();
        values = (T*)allocData(xSize * ySize * sizeof(T));
        axisX = (XAxisT*)allocData(xSize * sizeof(XAxisT));
        axisY = (YAxisT*)allocData(ySize * sizeof(YAxisT));
        cacheIsValid = false;
        if(ySize == 1) axisY[0] = 1;
        lastX_in=0;
        lastY_in=0;
    }
    
    /**
     * Gets the value table value by x,y axis value/s.
     * @param X_in The x-axis value.
     * @param Y_in The y-axis value.
     * @returns The table value. -1 if out of bounds.
     */
    double getValue(const XAxisT X_in, const YAxisT Y_in) {
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
            unsigned int xMinIdx = 0;
            unsigned int yMinIdx = 0;
            unsigned int xMaxIdx = 0;
            unsigned int yMaxIdx = 0;
            XAxisT xMin = 0; 
            XAxisT xMax = 0; 
            YAxisT yMin = 0; 
            YAxisT yMax = 0;
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

    /**
     * Retrieves the value of a specific position.
     * @param X_in The x-axis value.
     * @returns The value at the specified position. If the position is outside of the table bounds, returns a default value.
     */
    double getValue(const XAxisT X_in) {
        return getValue(X_in, 1);
    }

    /**
     * Sets the value of a specific position in the table.
     * @param X_in The x-axis value.
     * @param Y_in The y-axis value.
     * @param value The new value to set at the specified (x,y) position.
     * @returns True if the value was set successfully, False otherwise. 
     */
    bool setValue(const XAxisT X_in, const YAxisT Y_in, const T value) {
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
            return true;
        }
        return false;
    }

    /**
     * Sets the value of a specific position in the table using only the x-axis value.
     * @param X_in The x-axis value.
     * @param value The new value to set at the specified (x,y) position.
     * @returns True if the value was set successfully, False otherwise. 
     */
    bool setValue(const XAxisT X_in, const T value){
        int x = -1;
        for (unsigned int i = 0; i < xSize; i++){
            if(X_in == axisX[i]) x = i;
        }
        // Direct cell found, return the direct value
        if (x >= 0){
            setValueByIndex(x, value);
            return true;
        }
        return false;
    }

    /**
     * Set Value by X and Y Index.
     * @param x index of the row in the table.
     * @param y index of the column in the table.
     * @param value to set.
     * @returns True if the value was set successfully, False otherwise.
     */
    bool setValueByIndex(const unsigned int x, const unsigned int y, const T value){
        if(x >= xSize || y >= ySize){
            return false;
        }
        values[x * ySize + y] = value;
        return true;
    }

    /**
     * Set Value by X Index.
     * @param x index of the row in the table.
     * @param value to set.
     * @returns True if the value was set successfully, False otherwise.
     */
    bool setValueByIndex(const unsigned int x, const T value){
        return setValueByIndex(x, 0, value);
    }

    /**
     * Get Value by X and Y index.
     * @param x index of the row in the table.
     * @param y index of the column in the table.
     * @return value at index (x,y).
     */
    T getValueByIndex(const unsigned int x, const unsigned int y){
        return values[x * ySize + y];
    }

    /**
     * Get Value by X index.
     * @param x index of the row in the table.
     * @return value at index x.
     */
    T getValueByIndex(const unsigned int x){
        return getValueByIndex(x, 0);
    }

    /**
     * Set X Axis Value by Index.
     * @param x index of the row in the table.
     * @param value to set.
     * @returns True if the value was set successfully, False otherwise.
     */
    bool setXAxisValueByIndex(const unsigned int x, const XAxisT value){
        if(x >= xSize){
            return false;
        }
        axisX[x] = value;
        return true;
    }

    /**
     * Set Y Axis Value by Index.
     * @param y index of the column in the table.
     * @param value to set.
     * @returns True if the value was set successfully, False otherwise.
     */
    bool setYAxisValueByIndex(const unsigned int y, const YAxisT value){
        if(y >= ySize){
            return false;
        }
        axisY[y] = value;
        return true;
    }

    /**
     * Load table data from a buffer.
     * @param buffer pointer to the data buffer.
     * @param size size of the buffer in bytes.
     * @returns true if data was loaded successfully.
     */
    bool loadData(const char* buffer, size_t size) {
        if (size != getDataSize()) {
            return false;
        }
        
        // Copy the entire data buffer
        for (size_t i = 0; i < size; i++) {
            data[i] = buffer[i];
        }
        
        // Re-initialize pointers
        dataPointer = 0;
        values = (T*)allocData(xSize * ySize * sizeof(T));
        axisX = (XAxisT*)allocData(xSize * sizeof(XAxisT));
        axisY = (YAxisT*)allocData(ySize * sizeof(YAxisT));
        
        // Reset cache
        cacheIsValid = false;
        return true;
    }

    /**
     * Save table data to a buffer.
     * @param buffer pointer to the output buffer.
     * @param size size of the buffer in bytes.
     * @returns true if data was saved successfully.
     */
    bool saveData(char* buffer, size_t size) const {
        if (size != getDataSize()) {
            return false;
        }
        
        // Copy the entire data buffer
        for (size_t i = 0; i < size; i++) {
            buffer[i] = data[i];
        }
        
        return true;
    }

    /**
     * Get Data Size.
     * @return size of the data in bytes.
     */
    static constexpr int getDataSize(){
        // Values, xAxis, yAxis
        return xSize*ySize*sizeof(T) + xSize*sizeof(XAxisT) + ySize*sizeof(YAxisT) + 1;
    }
    
    // The data is stored here.
    char data[getDataSize()] = {0};

    /**
     * Reset the data to zero.
     */
    void resetData(){
        for(unsigned int i = 0; i < getDataSize(); i++){
            data[i] = 0x00;  
        }
        dataPointer = 0;
    }

    /**
     * Invalidate the cache.
     * This will force a recalculation of the output when it is next requested.
     */
    void invalidateCache(){
        cacheIsValid = false;
    }

protected:
    // table values.
    T* values; // x * ySize + y
    XAxisT* axisX;
    YAxisT* axisY;
    
private:
    // caching.
    XAxisT lastX_in;
    YAxisT lastY_in;
    double lastOutput;
    bool cacheIsValid;

    /**
     * Data Ptr.
     */
    unsigned int dataPointer = 0;

    /**
     * Alloca Data.
     * @param size Size of data to allocate.
     * @returns Pointer to allocated data or nullptr if allocation failed.
     */
    constexpr void* allocData(unsigned int size){
        char* ptr = nullptr;
        if(size < getDataSize()-dataPointer){
            ptr = &data[dataPointer];
        }
        dataPointer+=size;
        return ptr;
    }

    /** 
     * Bi-Linear Interpolation Alg.
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
     * Linear Interpolation Alg.
     * 
     * y1|-Q11----R1----Q21
     *   |__!_____!______!_
     *      x1    x     x2
     */
    static double linearInterpolation(const double q11, const double q21, const int x1,  const int x2, const int x)
    {
        return q11 + ((q21-q11)/(x2-x1)) * (x-x1);
    }
};

#endif // EPICECU_TABLE_H