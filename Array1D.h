#ifndef MAIN_CPP_ARRAY1D_H
#define MAIN_CPP_ARRAY1D_H

#include <ostream>
#include <memory>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <algorithm>

// Class for throwing error that array size is zero or negative
class ArraySizeNonPositive{};
class ArrayLengthExhausted{};

const float FILLER = -9999.f;
const float TOLERANCE= 0.00000001f;

class Array1D
{
public:
    Array1D(int arr_size, float default_value = FILLER);

    Array1D(const Array1D &arr);

    //Enables the construction of a new array from 0 of arr until idx of arr
    Array1D(int idx,const Array1D &arr );

    ~Array1D(){}

    float at(int idx) const;

    void set(int idx, float val);

    //Sets the value immediately after the last FILLER value (effectively like a vector push back). Throws 
    //ArrayLengthExhausted{} exception when want to add a value past the defined array size.
    void setNext(float val);

    //Returns the size of the array that does not have FILLER values
    int sizeNoFiller() const;

    int size() const;

    void sort(int start_idx, int end_idx);

    //return first index greater than or equal to the given value assuming the array is sorted
    //returns size of array if no val is found
    int findFirst(float val);

    //return first index greater than or equal to the given value assuming the array is sorted
    //returns size of array if no val is found
    int findLast(float val);

    float* arrayPtrGet(int idx);

    bool operator==(const Array1D &) const;

    friend std::ostream &operator<<(std::ostream &os, const Array1D &obj);

private:
    // Length of array
    long int array_size = 0;

    //container choice
    std::shared_ptr<float[]> array_ptr;

    int first_filler_index;
};

#endif //MAIN_CPP_ARRAY1D_H