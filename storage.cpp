// #include <omp.h>
// #include <iostream>
// #include <cstdlib>
// #include <omp.h>
// #include <time.h>  
// #include <cmath>
// #include "mpi.h"
// #include <string>
// #include <algorithm>
// #include <array>
// #include "Array1D.h"

// const int NUM_ELEMS = 10000;

// void rand_doubles(Array1D& arr);

// //void bubble_sort(double arr[], int num_elems);

// void mergeSortedArrays(Array1D& arr1, Array1D& arr2, Array1D& arr_out);

// Array1D* SendAndReceive(Array1D* my_arr, int my_rank, int num_proc, int merge_num, bool& merge_done);

// int* assignBatches(int num_procs, int num_elems);

// int* assignDisplacements(int* batch_count, int num_procs, int num_elems);

// int main()
// {
//     MPI_Init(NULL, NULL);

//     auto rank = 0;
//     auto num_procs = 0;

//     MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//     auto all_data = Array1D(NUM_ELEMS);
//     if(rank == 0) {
//         rand_doubles(all_data);
//         // std::cout << all_data;
//         // std::cout << std::endl << std::endl;
//     }
//     auto start_time = omp_get_wtime();

//     auto batch_count = assignBatches(num_procs, NUM_ELEMS);
//     auto displ_count =  assignDisplacements(batch_count, num_procs, NUM_ELEMS);

//     auto my_batch_size = batch_count[rank];
//     auto sub_elems = new Array1D(my_batch_size);

//     MPI_Scatterv(all_data.array_ptr.get(), batch_count, displ_count, MPI_DOUBLE,
//                 sub_elems->array_ptr.get(), my_batch_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

// //    std::cout << rank << ": " << *sub_elems << std::endl;

//    //each process sorts its own chunk of data
//     sub_elems->sort(0,sub_elems->size());

//     auto other_sub_elems = new Array1D(NUM_ELEMS/num_procs);
//     auto combined_elems = new Array1D(2*NUM_ELEMS/num_procs);

//     int merge_num = 0;

//     bool merge_done = false;
//     while(!merge_done)//sub_elems->size() < all_data.size())
//     {
//         sub_elems = SendAndReceive(sub_elems, rank, num_procs, merge_num, merge_done);
//         ++merge_num;

//         if(sub_elems->size() == all_data.size()) break;
//     }
//     auto sort_time = omp_get_wtime()-start_time;

//     if(rank==0) std::cout << NUM_ELEMS << "," << num_procs << " --- "<< sort_time << std::endl;


//     MPI_Finalize();
// }

// #include <ctime>
// void rand_doubles(Array1D& arr)
// {
//     std::srand(std::time(NULL));
//     for(auto i=0; i<arr.size(); i++) {
//         arr.set(i, (double) (rand()%40)/ 10.0);
//     }

// }

// int* assignBatches(int num_procs, int num_elems)
// {
//     int* batch_count = new int[num_procs];

//     for(int i=0; i<num_elems%num_procs; i++) batch_count[i] = ceil(num_elems/((double)num_procs));
//     for(int i=num_elems%num_procs; i<num_procs; i++) batch_count[i] = floor(num_elems/((double)num_procs));

//     return batch_count;

// }

// int* assignDisplacements(int* batch_count, int num_procs, int num_elems)
// {
//     int* displ_count = new int[num_procs];
//     for(int i=1; i<num_procs; i++) {
//         displ_count[i] = displ_count[i-1]+batch_count[i-1];
//     }

//     return displ_count;
// }


// void bubble_sort(double arr[], int num_elems)
// {
//     double temp = 0;
//     for(int i=0; i<num_elems; i++)
//     {
//         for(int j=0; j<num_elems-1; j++)
//         {
//             if(arr[j] > arr[j+1] )
//             {
//                 temp=arr[j];
//                 arr[j] = arr[j+1];
//                 arr[j+1] = temp;
//             }
//         }
//     }

// }


// void mergeSortedArrays(Array1D& arr1, Array1D& arr2, Array1D& arr_out)
// {
//     int idx_out = 0;
//     int idx1 = 0;
//     int idx2 = 0;
//     double temp = 0;
//     while(1)
//     {
//         temp = arr1.at(idx1)<arr2.at(idx2) ? arr1.at(idx1++) : arr2.at(idx2++);
//         arr_out.set(idx_out++, temp);

//         if(idx1 ==  arr1.size()){
//             for(idx_out; idx_out<arr_out.size(); idx_out++) arr_out.set(idx_out, arr2.at(idx2++));
//             break;
//         }
//         if(idx2 ==  arr2.size() ){
//             for(idx_out; idx_out<arr_out.size(); idx_out++) arr_out.set(idx_out, arr1.at(idx1++));
//             break;
//         }
//     }
// }

// Array1D* SendAndReceive(Array1D* my_arr, int my_rank, int num_proc, int merge_num, bool& merge_done)
// {
//     int mod_number = pow(2, merge_num+1);
//     int send_number = pow(2, merge_num);
//     merge_done = false;

//     if(my_rank%mod_number == send_number) {
//         //Send the array_size on tag 0
//         int my_arr_size = my_arr->size();
//         MPI_Send(&my_arr_size, 1, MPI_DOUBLE, my_rank-send_number,0, MPI_COMM_WORLD);
//         //Then send the actual array on tag 1
//         MPI_Send(my_arr->array_ptr.get(), my_arr_size, MPI_DOUBLE, my_rank-send_number,1, MPI_COMM_WORLD);
//         merge_done = true;
//     }

//     if(my_rank != num_proc - 1 && my_rank%mod_number == 0) {
//         int other_arr_size = 0;
//         MPI_Recv(&other_arr_size, 1, MPI_DOUBLE, my_rank+send_number,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

//         auto other_arr = new Array1D(other_arr_size);
//         auto combined_arr = new Array1D(my_arr->size()+other_arr_size);

//         MPI_Recv(other_arr->array_ptr.get(), other_arr_size, MPI_DOUBLE, my_rank+send_number,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//         mergeSortedArrays(*my_arr, *other_arr, *combined_arr);

//         my_arr = combined_arr;
//     }

//     return my_arr;
// }

