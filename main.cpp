#include <omp.h>
#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <time.h>
#include <cmath>
#include "mpi.h"
#include <string>
#include <algorithm>
#include <array>
#include "Array1D.h"
#include "HelperFunctions.h"
#include <memory>
#include <malloc.h>
#include "FileReader.h"

using Array1DPtr = std::shared_ptr<Array1D>;

void ErrorCheck(MergeMetaData& meta_data);


int main(int argc, char **argv)
{
    auto meta_data = MergeMetaData();
    meta_data.dataset = argv[1];
    meta_data.startYear = std::stoi(argv[2]);
    meta_data.endYear = std::stoi(argv[3]);
    meta_data.filterZeros = std::stoi(argv[4]);

    float precipData[360 * 720*FileReader::NUM_RECS_READ];

    meta_data.filePrefix = meta_data.dataset;

    ErrorCheck(meta_data);

    MPI_Init(NULL, NULL);

    //declare all containers
    auto storage_elems = Array1DPtr{new Array1D{1}};
    auto all_data = Array1DPtr{new Array1D{1}};
    auto sub_elems = Array1DPtr{new Array1D{1}};
    auto filteredData = Array1DPtr{new Array1D{1}};
    int run_number = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &meta_data.num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &meta_data.my_rank);

    auto t_start = omp_get_wtime();
    
    if(meta_data.my_rank == 0)
    {
        std::cout << "Processing weather data for years: " << meta_data.startYear << "-" << meta_data.endYear << std::endl;
        std::cout << "Dataset: " << meta_data.dataset << std::endl;
        auto str = meta_data.filterZeros? std::string("No.") : "Yes.";
        std::cout << "Missing data values ignored. Zeros in the data included? " << str << std::endl;
        std::cout << "Number of processes: " << meta_data.num_procs << std::endl;
        std::cout << "Processing..." << std::endl << std::endl;
    }

    //iterate over years
    for (int year = meta_data.startYear; year <= meta_data.endYear; year++)
    {
        string fileName = meta_data.filePrefix + std::to_string(year) + ".nc";

        //for each year iterate over batches of the data
        FileReader fileReader{fileName};
        for (run_number = 0; run_number < meta_data.NREC; run_number+=FileReader::NUM_RECS_READ)
        {
            if (meta_data.my_rank == 0)
            {
                fileReader.getPrecipitation(precipData, meta_data.NLAT, meta_data.NLON, run_number + 1);
                filteredData = fileReader.filterData(precipData, meta_data.NLAT, meta_data.NLON, meta_data.filterZeros);
                all_data = filteredData;
                meta_data.NUM_ELEMS = all_data->size();
            }
        //rank 0 assigns batches and displacements based off of its own data
            auto batch_count = assignBatches(meta_data.num_procs, meta_data.NUM_ELEMS);
            auto displ_count = assignDisplacements(batch_count, meta_data.num_procs, meta_data.NUM_ELEMS);
        //rank 0 broadcasts what other ranks will receive
            MPI_Bcast(batch_count, meta_data.num_procs, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(displ_count, meta_data.num_procs, MPI_INT, 0, MPI_COMM_WORLD);

            auto my_batch_size = batch_count[meta_data.my_rank];
            sub_elems = Array1DPtr{new Array1D{my_batch_size}};
        //rank 0 scatters its data
            MPI_Scatterv(all_data->arrayPtrGet(0), batch_count, displ_count, MPI_FLOAT,
                         sub_elems->arrayPtrGet(0), my_batch_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

            //each process sorts its own chunk of data
            sub_elems->sort(0, sub_elems->size());
            meta_data.NUM_ELEMS = sub_elems->size();

            //the individual sorted arrays are then merged together iteratively
            //merging occurs until all the data is back in rank 0
            auto other_sub_elems = Array1DPtr{new Array1D(meta_data.NUM_ELEMS / meta_data.num_procs)};
            auto combined_elems = Array1DPtr{new Array1D(2 * meta_data.NUM_ELEMS / meta_data.num_procs)};

            int merge_num = 0;
            bool merge_done = false;

            while (!merge_done) //sub_elems->size() < all_data.size())
            {
                sub_elems = SendAndReceive(sub_elems, meta_data.my_rank, meta_data.num_procs, merge_num, merge_done);
                ++merge_num;

                if (sub_elems->size() == all_data->size())
                    break;
            }

            //storing the data is done in preparation for the next batch of data
            //if this is the first data iteration then rank - must find the maximum
            //value that each processor will store
            if (run_number == 0)
            {
                maxElementForProcessors(sub_elems, meta_data.maxProcElements, meta_data.num_procs);
                meta_data.numElementsPerBucket = std::vector<int>(meta_data.num_procs);
            }
            //distributing and storing data
            if (meta_data.my_rank == 0)
                distributeSortedArray(sub_elems, meta_data);
            if (meta_data.my_rank != 0)
                recieveSortedArray(storage_elems, run_number, meta_data.my_rank);

            MPI_Barrier(MPI_COMM_WORLD);
            delete batch_count;
            delete displ_count;
            if (meta_data.my_rank == 0)
                all_data.reset();
            sub_elems.reset();
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    //explicit synchronisation for final part of data processing
    MPI_Barrier(MPI_COMM_WORLD);

    auto Q1 = getQuartile(1, storage_elems, meta_data);
    auto Q2 = getQuartile(2, storage_elems, meta_data);
    auto Q3 = getQuartile(3, storage_elems, meta_data);
    auto abs_max = getAbsMax(storage_elems, meta_data);
    auto abs_min = getAbsMin(storage_elems, meta_data);
    auto box_plot_max = (Q2 + 1.5 * (Q3 - Q1)) < abs_max ? Q2 + 1.5 * (Q3 - Q1) : abs_max;
    auto box_plot_min = (Q2 - 1.5 * (Q3 - Q1)) > abs_min ? Q2 - 1.5 * (Q3 - Q1) : abs_min;
    if (box_plot_min < 0)
        box_plot_min = 0;

    if (meta_data.my_rank == 0)
    {
        std::cout << "The box and whisker plot for the daily rainfall (mm/day): " << std::endl <<std::endl;
        std::cout << std::left;
        std::cout << std::setw(15) <<  abs_min << std::setw(15) << box_plot_min
            << std::setw(15) <<Q1 << std::setw(15) << Q2 <<std::setw(15) << Q3 <<
            std::setw(15) << box_plot_max<<std::setw(15) << abs_max << std::endl;
        printf("|              |--------------|______________|_____________|---------------|              | \n\n");
    }

    auto t_end = omp_get_wtime();
   if(meta_data.my_rank==0) std::cout << "Total run time: " << t_end-t_start << "s" << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}


void ErrorCheck(MergeMetaData& meta_data)
{
    if (meta_data.dataset == "full_data_daily_v2020")
    {
        if (meta_data.startYear < 1982 || meta_data.startYear > 2019 || meta_data.endYear < 1982 || meta_data.endYear > 2019)
            std::cerr << "Invalid year range. Must be within 1982 - 2019" << endl;

        meta_data.NLAT = 180;
        meta_data.NLON = 360;

        meta_data.filePrefix = "/data/"+ meta_data.dataset + "/" + meta_data.dataset + "_10_";
    }
    else if (meta_data.dataset == "full_data_daily_V2018_05")
    {
        if (meta_data.startYear < 1982 || meta_data.startYear > 2016 || meta_data.endYear < 1982 || meta_data.endYear > 2016)
            std::cerr << "Invalid year range. Must be within 1982 - 2016" << endl;

        meta_data.NLAT = 360;
        meta_data.NLON = 720;

        meta_data.filePrefix = "/data/"+ meta_data.dataset + "/full_data_daily_v2018_05" + "_";
    }
    else
    {
        throw(std::invalid_argument("Invalid dataset given" ));
    }

}