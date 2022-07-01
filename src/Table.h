#ifndef TABLE_H
#define TABLE_H

#include <Arduino.h>

#define TABLE_HEAP_DEFAULT_SIZE 512 // Set a upper heap size limit, if a table can not fit, then return nullptr.

static char _table_heap[TABLE_HEAP_DEFAULT_SIZE];
static int _heap_pointer = 0;
/**
 * Allocates memory on the table heap
 **/
static void* heap_alloc(int size)
{
    char* value = nullptr;
    if (size < (TABLE_HEAP_DEFAULT_SIZE - _heap_pointer))
    {
        value = &_table_heap[_heap_pointer];
        _heap_pointer += size;
    }
    return value;
}
static void reset_heap()
{
    _heap_pointer = 0;
}

//The shift amount used for the 3D table calculations
#define TABLE_SHIFT_FACTOR  8
#define TABLE_SHIFT_POWER   (1UL<<TABLE_SHIFT_FACTOR)

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
template<typename T>
class Table3D {
public:
    void initilise(char x, char y)
    {
        this->values = (T**)heap_alloc(x * sizeof(T*));
        for (char i = 0; i < y; i++) { this->values[i] = (T*)heap_alloc(y * sizeof(T)); }

        this->axisX = (int*)heap_alloc(x * sizeof(int));
        this->axisY = (int*)heap_alloc(y * sizeof(int));
        this->xSize = x;
        this->ySize = y;
        this->lastXInput = 1; // some what hacky...
        this->lastYInput = 1;
        this->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
    }
    T getValue(int X_in, int Y_in)
    {
        int X = X_in;
        int Y = Y_in;

        T tableResult = 0;
        //Loop through the X axis bins for the min/max pair
        //Note: For the X axis specifically, rather than looping from tableAxisX[0] up to tableAxisX[max], we start at tableAxisX[Max] and go down.
        //      This is because the important tables (fuel and injection) will have the highest RPM at the top of the X axis, so starting there will mean the best case occurs when the RPM is highest (And hence the CPU is needed most)
        int xMinValue = this->axisX[0];
        int xMaxValue = this->axisX[this->xSize - 1];
        char xMin = 0;
        char xMax = 0;

        //If the requested X value is greater/small than the maximum/minimum bin, reset X to be that value
        if (X > xMaxValue) { X = xMaxValue; }
        if (X < xMinValue) { X = xMinValue; }

        //0th check is whether the same X and Y values are being sent as last time. If they are, this not only prevents a lookup of the axis, but prevents the interpolation calcs being performed
        if ((X_in == this->lastXInput) && (Y_in == this->lastYInput) && (this->cacheIsValid == true))
        {
            return this->lastOutput;
        }

        //Commence the lookups on the X and Y axis

        //1st check is whether we're still in the same X bin as last time
        if ((X <= this->axisX[this->lastXMax]) && (X > this->axisX[this->lastXMin]))
        {
            xMaxValue = this->axisX[this->lastXMax];
            xMinValue = this->axisX[this->lastXMin];
            xMax = this->lastXMax;
            xMin = this->lastXMin;
        }
        //2nd check is whether we're in the next RPM bin (To the right)
        else if (((this->lastXMax + 1) < this->xSize) && (X <= this->axisX[this->lastXMax + 1]) && (X > this->axisX[this->lastXMin + 1])) //First make sure we're not already at the last X bin
        {
            xMax = this->lastXMax + 1;
            this->lastXMax = xMax;
            xMin = this->lastXMin + 1;
            this->lastXMin = xMin;
            xMaxValue = this->axisX[this->lastXMax];
            xMinValue = this->axisX[this->lastXMin];
        }
        //3rd check is to look at the previous bin (to the left)
        else if ((this->lastXMin > 0) && (X <= this->axisX[this->lastXMax - 1]) && (X > this->axisX[this->lastXMin - 1])) //First make sure we're not already at the first X bin
        {
            xMax = this->lastXMax - 1;
            this->lastXMax = xMax;
            xMin = this->lastXMin - 1;
            this->lastXMin = xMin;
            xMaxValue = this->axisX[this->lastXMax];
            xMinValue = this->axisX[this->lastXMin];
        }
        else
            //If it's not caught by one of the above scenarios, give up and just run the loop
        {
            for (int x = this->xSize - 1; x >= 0; x--)
            {
                //Checks the case where the X value is exactly what was requested
                if ((X == this->axisX[x]) || (x == 0))
                {
                    xMaxValue = this->axisX[x];
                    xMinValue = this->axisX[x];
                    xMax = x;
                    this->lastXMax = xMax;
                    xMin = x;
                    this->lastXMin = xMin;
                    break;
                }
                //Normal case
                if ((X <= this->axisX[x]) && (X > this->axisX[x - 1]))
                {
                    xMaxValue = this->axisX[x];
                    xMinValue = this->axisX[x - 1];
                    xMax = x;
                    this->lastXMax = xMax;
                    xMin = x - 1;
                    this->lastXMin = xMin;
                    break;
                }
            }
        }

        //Loop through the Y axis bins for the min/max pair
        int yMaxValue = this->axisY[0];
        int yMinValue = this->axisY[this->ySize - 1];
        char yMin = 0;
        char yMax = 0;

        //If the requested Y value is greater/small than the maximum/minimum bin, reset Y to be that value
        if (Y > yMaxValue) { Y = yMaxValue; }
        if (Y < yMinValue) { Y = yMinValue; }

        //1st check is whether we're still in the same Y bin as last time
        if ((Y >= this->axisY[this->lastYMax]) && (Y < this->axisY[this->lastYMin]))
        {
            yMaxValue = this->axisY[this->lastYMax];
            yMinValue = this->axisY[this->lastYMin];
            yMax = this->lastYMax;
            yMin = this->lastYMin;
        }
        //2nd check is whether we're in the next MAP/TPS bin (Next one up)
        else if ((this->lastYMin > 0) && (Y <= this->axisY[this->lastYMin - 1]) && (Y > this->axisY[this->lastYMax - 1])) //First make sure we're not already at the top Y bin
        {
            yMax = this->lastYMax - 1;
            this->lastYMax = yMax;
            yMin = this->lastYMin - 1;
            this->lastYMin = yMin;
            yMaxValue = this->axisY[this->lastYMax];
            yMinValue = this->axisY[this->lastYMin];
        }
        //3rd check is to look at the previous bin (Next one down)
        else if (((this->lastYMax + 1) < this->ySize) && (Y <= this->axisY[this->lastYMin + 1]) && (Y > this->axisY[this->lastYMax + 1])) //First make sure we're not already at the bottom Y bin
        {
            yMax = this->lastYMax + 1;
            this->lastYMax = yMax;
            yMin = this->lastYMin + 1;
            this->lastYMin = yMin;
            yMaxValue = this->axisY[this->lastYMax];
            yMinValue = this->axisY[this->lastYMin];
        }
        else
            //If it's not caught by one of the above scenarios, give up and just run the loop
        {

            for (int y = this->ySize - 1; y >= 0; y--)
            {
                //Checks the case where the Y value is exactly what was requested
                if ((Y == this->axisY[y]) || (y == 0))
                {
                    yMaxValue = this->axisY[y];
                    yMinValue = this->axisY[y];
                    yMax = y;
                    this->lastYMax = yMax;
                    yMin = y;
                    this->lastYMin = yMin;
                    break;
                }
                //Normal case
                if ((Y >= this->axisY[y]) && (Y < this->axisY[y - 1]))
                {
                    yMaxValue = this->axisY[y];
                    yMinValue = this->axisY[y - 1];
                    yMax = y;
                    this->lastYMax = yMax;
                    yMin = y - 1;
                    this->lastYMin = yMin;
                    break;
                }
            }
        }


        /*
        At this point we have the 4 corners of the map where the interpolated value will fall in
        Eg: (yMin,xMin)  (yMin,xMax)

            (yMax,xMin)  (yMax,xMax)

        In the following calculation the table values are referred to by the following variables:
                  A          B

                  C          D

        */
        T A = this->values[yMin][xMin];
        T B = this->values[yMin][xMax];
        T C = this->values[yMax][xMin];
        T D = this->values[yMax][xMax];

        //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
        if ((A == B) && (A == C) && (A == D)) { tableResult = A; }
        else
        {
            //Create some normalised position values
            //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis


            //Initial check incase the values were hit straight on

            unsigned long p = (long)X - xMinValue;
            if (xMaxValue == xMinValue) { p = (p << TABLE_SHIFT_FACTOR); }  //This only occurs if the requested X value was equal to one of the X axis bins
            else { p = ((p << TABLE_SHIFT_FACTOR) / (xMaxValue - xMinValue)); } //This is the standard case

            unsigned long q;
            if (yMaxValue == yMinValue)
            {
                q = (long)Y - yMinValue;
                q = (q << TABLE_SHIFT_FACTOR);
            }
            //Standard case
            else
            {
                q = long(Y) - yMaxValue;
                q = TABLE_SHIFT_POWER - ((q << TABLE_SHIFT_FACTOR) / (yMinValue - yMaxValue));
            }

            T m = ((TABLE_SHIFT_POWER - p) * (TABLE_SHIFT_POWER - q)) >> TABLE_SHIFT_FACTOR;
            T n = (p * (TABLE_SHIFT_POWER - q)) >> TABLE_SHIFT_FACTOR;
            T o = ((TABLE_SHIFT_POWER - p) * q) >> TABLE_SHIFT_FACTOR;
            T r = (p * q) >> TABLE_SHIFT_FACTOR;
            tableResult = ((A * m) + (B * n) + (C * o) + (D * r)) >> TABLE_SHIFT_FACTOR;
        }

        //Update the tables cache data
        this->lastXInput = X_in;
        this->lastYInput = Y_in;
        this->lastOutput = tableResult;
        this->cacheIsValid = true;

        return tableResult;
    }

    // table size
    char xSize;
    char ySize;
    // table values
    T** values;
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

template<typename T>
class Table2D {
public:
    void initilise(char x)
    {
        this->values = (T*)heap_alloc(x * sizeof(T*));
        this->axisX = (int*)heap_alloc(x * sizeof(int));
        this->xSize = x;
        this->lastInput = 1; // some what hacky...
    }

    T getValue(int X_in)
    {
        //Orig memory usage = 5414
        T returnValue = 0;
        bool valueFound = false;

        int X = X_in;
        int xMinValue, xMaxValue;
        int xMin = 0;
        int xMax = this->xSize - 1;

        //Check whether the X input is the same as last time this ran
        if ((X_in == this->lastInput) )//&& (this->cacheTime == currentStatus.secl))
        {
            returnValue = this->lastOutput;
            valueFound = true;
        }
        //If the requested X value is greater/small than the maximum/minimum bin, simply return that value
        else if (X >= this->getAxisValue(xMax))
        {
            returnValue = this->getRawValue(xMax);
            valueFound = true;
        }
        else if (X <= this->getAxisValue(xMin))
        {
            returnValue = this->getRawValue(xMin);
            valueFound = true;
        }
        //Finally if none of that is found
        else
        {
            //this->cacheTime = currentStatus.secl; //As we're not using the cache value, set the current secl value to track when this new value was calc'd

            //1st check is whether we're still in the same X bin as last time
            xMaxValue = this->getAxisValue(this->lastXMax);
            xMinValue = this->getAxisValue(this->lastXMin);
            if ((X <= xMaxValue) && (X > xMinValue))
            {
                xMax = this->lastXMax;
                xMin = this->lastXMin;
            }
            else
            {
                //If we're not in the same bin, loop through to find where we are
                xMaxValue = this->getAxisValue(this->xSize - 1); // init xMaxValue in preparation for loop.
                for (int x = this->xSize - 1; x > 0; x--)
                {
                    xMinValue = this->getAxisValue(x - 1); // fetch next Min

                    //Checks the case where the X value is exactly what was requested
                    if (X == xMaxValue)
                    {
                        returnValue = this->getRawValue(x); //Simply return the coresponding value
                        valueFound = true;
                        break;
                    }
                    else if (X > xMinValue)
                    {
                        // Value is in the current bin
                        xMax = x;
                        this->lastXMax = xMax;
                        xMin = x - 1;
                        this->lastXMin = xMin;
                        break;
                    }
                    // Otherwise, continue to next bin
                    xMaxValue = xMinValue; // for the next bin, our Min is their Max
                }
            }
        } //X_in same as last time

        if (valueFound == false)
        {
            float m = X - xMinValue;
            float n = xMaxValue - xMinValue;

            T yMax = this->getRawValue(xMax);
            T yMin = this->getRawValue(xMin);

            //Float version
            
            T yVal = (m / n) * (abs(yMax - yMin));
            

            //Non-Float version
            // int16_t yVal = ((long)(m << 6) / n) * (abs(yMax - yMin));
            // yVal = (yVal >> 6);

            if (yMax > yMin) { yVal = yMin + yVal; }
            else { yVal = yMin - yVal; }

            returnValue = yVal;
        }

        this->lastInput = X_in;
        this->lastOutput = returnValue;

        return returnValue;
    }
private:
    int getAxisValue(char X_index)
    {
        return this->axisX[X_index];
    }
    T getRawValue(char X_index)
    {
        return this->values[X_index];
    }

public:
    // table size
    char axisSize;
    char xSize;
    // table values
    T* values;
    int* axisX;
    //Store the last X and Y coordinates in the table. This is used to make the next check faster
    int lastXMax;
    int lastXMin;
    //Store the last input and output for caching
    int lastInput;
    T lastOutput;
    char cacheTime;
};

#endif // TABLE_H