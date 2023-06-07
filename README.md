# Project - ELEN4020
---
Aaron Fainman (1386259), Nathan Jones (1619191) & Taliya Weinstein (1386891)

## Program Overview

The aim of this program is to analyse sets of daily precipitation data. The data is made available by the Global Precipitation Climatology Centre [(GPCC)](https://www.dwd.de/EN/ourservices/gpcc/gpcc.html). The program will output all the values necessary for a box-and-whisker plot. Processing has been implemented using [MPI](https://www.mpich.org/). The outputs are given in mm/day. The program accepts inputs for the dataset, range of years to be analysed, and whether to include zeros in the analysis. The necessary error checks are performed on the input.

## Statistical Definitions
The box-and-whisker plot is a useful tool for understanding the distribution of data in a dataset. The definition of an outlier used in this program is any value greater or less than 1.5*IQR, where IQR is the inter-quartile range (the difference between quartile 3 and quartile 1). The box-and-whisker plot ouput consists of 7 values: 
* Absolute minimum
     * this is the smallest value in the dataset
     * the value may or may not be an outlier
* The non-outlier minimum
    * this is the smallest value that is not classified as an outlier
    * the value is taken as Q2 - 1.5*IQR (or 0 if if Q2-1.5*IQR is negative)
    * it may or may not exist in the dataset
* Q1 value
    * the lowest 25% of the data
* Q2 value
    * the midpoint of the data
* Q3 value
    * the highest 25% of the data
* The non-outlier maximum
    * this is the maximum value that is not classified as an outlier
    * the value equals Q2+1.5*IQR
    * it may or may not exist in the dataset
* The absolute maximum
    * this is the largest value in the dataset
    * it may or may not be an outlier
It is worth noting that the dataset is not completely populated. There may be missing values in the data. These have been ignored in the program analysis. Values of 0 are included in the dataset unless specified by the user.

## Data Format
The data format used is [NetCDF](https://www.unidata.ucar.edu/software/netcdf/). Variables included in the dataset are longitude, latitude, time, precipitation, interpolation error, and gauge number. Missing values in the dataset are given by -9999. This program automatically filters out these values. There are different datasets for 0.5º and 1º longitude and latitude resolution. For more on the dataset, including curators and monthly updates see the [GPCC Full Data Monthly Product Version](http://dx.doi.org/10.5676/DWD_GPCC/FD_M_V2020_100).

## Requirements

* Unix/Linux terminal
* GNU Make 3.81+
* GCC 10.1.0+
* netcdf-cxx4
* MPICH

Note that the netcdf-cxx4 library needs to be installed using the [VCPKG](https://vcpkg.io/en/index.html) package manager. The instruction set for doing this can be found [here](https://vcpkg.io/en/getting-started.html). Be careful to note the directory in which you install this package.

## Building and Running

Bash scripts can be used to automate the compilation process. To run the script, enter the following in a standard Linux terminal from within the root directory:

```sh
bash build.sh
```
The results for the box and whisker plot will be displayed in the terminal.

#### Compiling

The program can also be compiled manually by running the following commands from the root directory:

```sh
mkdir build
cmake -B ./build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build ./build/
```
In the case of our group, the path to vcpkg is `/home/group01/vcpkg`. Note that running `mkdir build` is not necessary if the script has already been run, or if it has been manually entered at least once before.

#### Compiled Output

The compilation process will produce the executable `ELEN4020_Project`, which accepts four command line inputs - *dataset*, *startYear*, *endYear* and *filterZeros*. Assuming this program is being run in the Jaguar1 cluster, SLURM will be used to allocate the number of cores (c) and processes (n).
The template command for running the executable is shown below:

```sh
srun -c <cores> -n <processes> --mpi=pmi2 build/ELEN4020_Project <dataset> <startYear> <endYear> <filterZeros>
```

There are only two datasets available on the cluster, each with their own year range as follows:

|          Dataset         |  Year range |
|:------------------------:|:-----------:|
|   full_data_daily_v2020  | 1982 - 2019 |
| full_data_daily_V2018_05 | 1982 - 2016 |

Finally, filterZeros should be 1 or 0, which indicate either "yes" and "no" to whether instances of zero rainfall should be excluded in the analysis. For example, the following command will execute the program with **2** cores and **4** processors using the **full_data_daily_v2020** dataset between the years **1998** to **2016** while **excluding** instances of zero rainfall:

```sh
srun -c 2 -n 4 --mpi=pmi2 build/ELEN4020_Project full_data_daily_v2020 1998 2016 1
```

## Some Functionality Notes
The data is read a single week at a time, ie. for a fully year there will be 52 iterations of reading and processing. Processing consists of a merge sort on the batch of data. Between the iterations, the sorted data is distributed and stored amongst the processes. After all the iterations, the final box-plot values are derived.


## Key Source Files

> **Array1D.cpp**  
> The data class that has been used to store the extracted NetCDF data.
> Sorts, finds, sets, and outputs the data.

> **FileReader.cpp**  
> Extracts the NetCDF data into Array1D format.

> **HelperFunctions.cpp**  
> Contains functions necessary for running the program.
> Assigns batches of data, distributes sorted batches to be stored by threads, 
> and merges batches of sorted data together. It also coordinates the final
> analysis of the data by finding the box-and-whisker values.

> **main.cpp**  
> Coordinates the reading, sorting, storing and analysing of the data. It also 
> recording the execution times for the program. 
> Necessary error checks on the input are also handled here.

# Division of Labour
* Aaron Fainman (1386259)
    * Wrote the sorting algorithm, together with Taliya Weinstein
    * Coordinated the different parts of the program in MPI, together with Taliya Weinstein
* Nathan Jones (1619191) 
    * Handled extraction and filtering of the NetCDF data
    * Wrote the CMake files
    * Wrote the build script
* Taliya Weinstein (1386891)
    * Wrote the sorting algorithm, together with Aaron Fainman
    * Coordinated the different parts of the program in MPI, together with Aaron Fainman
* All members of the group wrote the report.

---
# Data-Intensive-Computing-Weather-Processing
