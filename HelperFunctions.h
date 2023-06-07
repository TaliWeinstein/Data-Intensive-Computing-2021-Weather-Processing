#ifndef MAIN_CPP_HELPERFUNCTIONS_H
#define MAIN_CPP_HELPERFUNCTIONS_H

#include "Array1D.h"
#include <stdlib.h>
#include <cmath>
#include <mpi.h>
#include <vector>
#include <iostream>
#include <memory>
#include <malloc.h>

using Array1DPtr = std::shared_ptr<Array1D>;

//struct on what each processor stores 
struct MergeMetaData
{
    std::string dataset;
    std::string filePrefix;
    int startYear;
    int endYear;
    bool filterZeros; //flag: whether to filter zeros from the data
    int NLAT; //latitude
    int NLON; //longitude
    int NREC = 365; //number of time recordings to process
    int NUM_ELEMS; //number of data elements 
    int num_procs = 1;

    int my_rank = 0;

    //stores the maximum value that each process stores in its bucket
    std::vector<float> maxProcElements;

    //stores the number of elements that each process is storing in its bucket
    std::vector<int> numElementsPerBucket;
};

void mergeSortedArrays(Array1D& arr1, Array1D& arr2, Array1D& arr_out);

Array1DPtr SendAndReceive(Array1DPtr& my_arr, int my_rank, int num_proc, int merge_num, bool& merge_done);

int* assignBatches(int num_procs, int num_elems);

int* assignDisplacements(int* batch_count, int num_procs, int num_elems);

//Used to divide the data into smaller buckets that can be stored by each core
//This avoids storing the entire data in a single core (which cannot be done for very large datasets)
void maxElementForProcessors(Array1DPtr& sorted_arr, std::vector<float>& maxProcElements, int num_procs);

void distributeSortedArray(Array1DPtr& sorted_arr, MergeMetaData& meta_data);

void recieveSortedArray(Array1DPtr& storage_elems, int run_number, int my_rank);

float getQuartile(int q, Array1DPtr& storage_elems, MergeMetaData& meta_data);

float getAbsMin(Array1DPtr& storage_elems, MergeMetaData& meta_data);

float getAbsMax(Array1DPtr& storage_elems, MergeMetaData& meta_data);




#endif