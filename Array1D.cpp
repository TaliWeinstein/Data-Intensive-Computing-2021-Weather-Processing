#include "Array1D.h"

Array1D::Array1D(int arr_size, float default_value)
    : array_size{arr_size},
      array_ptr{new float[arr_size]},
      first_filler_index{0}
{
    if (arr_size <= 0)
        throw(ArraySizeNonPositive{});

    for (int idx = 0; idx < array_size; idx++)
        set(idx, default_value);
}

Array1D::Array1D(const Array1D &arr)
    : array_size{arr.array_size}, array_ptr{new float[arr.array_size]}, first_filler_index{arr.first_filler_index}
{
    for (int idx = 0; idx < array_size; idx++)
        set(idx, arr.at(idx));
}

Array1D::Array1D(int idx, const Array1D &arr)
    : array_size{idx + 1},
      array_ptr{new float[idx + 1]}
{
    if (idx >= arr.array_size || idx < 0)
        throw(std::out_of_range("Index out of Array Bounds"));

    for (int i = 0; i <= idx; i++)
    {
        set(i, arr.at(i));
    }
}

float Array1D::at(int idx) const
{
    if (idx >= array_size || idx < 0)
        throw(std::out_of_range("Index out of Array Bounds"));

    return array_ptr[idx];
}

void Array1D::set(int idx, float val)
{
    if (idx >= array_size || idx < 0)
        throw(std::out_of_range("Index out of Array Bounds"));

    array_ptr[idx] = val;
}

int Array1D::size() const
{
    return array_size;
}

void Array1D::sort(int start_idx, int end_idx)
{
    //Built in sort with O(Nlog(N)) time
    std::sort(&array_ptr[start_idx], &array_ptr[end_idx]);
}

int Array1D::findFirst(float val)
{
    for (int i = 0; i < array_size; i++)
        if (array_ptr[i] >= val)
            return i;

    return array_size;
}

int Array1D::findLast(float val)
{
    for (int i = 0; i < array_size; i++)
        if (array_ptr[i] > val)
            return i;

    return array_size;
}

float *Array1D::arrayPtrGet(int idx)
{
    if (idx >= array_size || idx < 0)
        throw(std::out_of_range("Index out of Array Bounds"));

    return &array_ptr[idx];
}

bool Array1D::operator==(const Array1D &compObj) const
{
    if (array_size != compObj.array_size)
        return false;

    for (int idx = 0; idx < array_size; idx++)
    {
        if (array_ptr[idx] != compObj.array_ptr[idx])
            return false;
    }

    return true;
}

std::ostream &operator<<(std::ostream &os, const Array1D &obj)
{
    for (auto idx = 0; idx < obj.size(); idx++)
    {
        os << std::setw(5) << obj.array_ptr[idx] << " ";
    }

    return os;
}

void Array1D::setNext(float val)
{
    if(first_filler_index==array_size) throw(ArrayLengthExhausted{});

    set(first_filler_index, val);
    // for (int i = 0; i < array_size; i++)
    // {
    //     if ((at(i) - FILLER) < TOLERANCE)
    //     {
    //         set(i, val);
    //         return;
    //     }
    // }

    ++first_filler_index;
}

int Array1D::sizeNoFiller() const
{
    int count = 0;
    for (int i = 0; i < array_size; i++)
    {
        if ((at(i) - FILLER) < TOLERANCE)
            continue;
        else
            count++;
    }
    return count;
}
