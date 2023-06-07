#include "HelperFunctions.h"


int* assignBatches(int num_procs, int num_elems)
{
    auto batch_count = new int[num_procs];

    for(int i=0; i<num_elems%num_procs; i++) batch_count[i] = ceil(num_elems/((float)num_procs));
    for(int i=num_elems%num_procs; i<num_procs; i++) batch_count[i] = floor(num_elems/((float)num_procs));

    return batch_count;
}

int* assignDisplacements(int* batch_count, int num_procs, int num_elems)
{
    int* displ_count = new int[num_procs];
    displ_count[0] = 0;
    for(int i=1; i<num_procs; i++) {
        displ_count[i] = displ_count[i-1]+batch_count[i-1];
    }

    return displ_count;
}

void mergeSortedArrays(Array1D& arr1, Array1D& arr2, Array1D& arr_out)
{
    int idx_out = 0;
    int idx1 = 0;
    int idx2 = 0;
    float temp = 0;
    while(1)
    {
        temp = arr1.at(idx1)<arr2.at(idx2) ? arr1.at(idx1++) : arr2.at(idx2++);
        arr_out.set(idx_out++, temp);

        if(idx1 ==  arr1.size()){
            for(idx_out; idx_out<arr_out.size(); idx_out++) arr_out.set(idx_out, arr2.at(idx2++));
            break;
        }
        if(idx2 ==  arr2.size() ){
            for(idx_out; idx_out<arr_out.size(); idx_out++) arr_out.set(idx_out, arr1.at(idx1++));
            break;
        }
    }
}

Array1DPtr SendAndReceive(Array1DPtr& my_arr, int my_rank, int num_proc, int merge_num, bool& merge_done)
{
    int mod_number = pow(2, merge_num+1);
    int send_number = pow(2, merge_num);
    merge_done = false;
    
    if(my_rank%mod_number == send_number) {
        //Send the array_size on tag 0
        int my_arr_size = my_arr->size();
        MPI_Send(&my_arr_size, 1, MPI_FLOAT, my_rank-send_number,0, MPI_COMM_WORLD);
        //Then send the actual array on tag 1
        MPI_Send(my_arr->arrayPtrGet(0), my_arr_size, MPI_FLOAT, my_rank-send_number,1, MPI_COMM_WORLD);
        merge_done = true;
    }
    
    if( (my_rank != num_proc - 1) && (my_rank%mod_number == 0) && (my_rank+send_number < num_proc)) {
        int other_arr_size = 0;
        MPI_Recv(&other_arr_size, 1, MPI_FLOAT, my_rank+send_number,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        auto other_arr = Array1DPtr{new Array1D{other_arr_size}};
        auto combined_arr = Array1DPtr{new Array1D(my_arr->size()+other_arr_size)};

        MPI_Recv(other_arr->arrayPtrGet(0), other_arr_size, MPI_FLOAT, my_rank+send_number,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        mergeSortedArrays(*my_arr, *other_arr, *combined_arr);

        my_arr = combined_arr;
    }

    return my_arr;
}


void maxElementForProcessors(Array1DPtr& sorted_arr, std::vector<float>& maxProcElements, int num_procs)
{
    //will calculate the largest element each processor should store by dividing the data into N buckets
    //where N is the number of processors
    maxProcElements.clear();

    //rank 1 stores all zeros (0 precipitation in a region)
    int last_0_idx = sorted_arr->findLast(0);

    int eachBucketSize = ceil((float) (sorted_arr->size()-last_0_idx)/(num_procs-1)); //rank 0 will not store any extra data

    maxProcElements.push_back(0);
    maxProcElements.push_back(0);

    for(int i=1; i<num_procs-1; i++) {
        maxProcElements.push_back(sorted_arr->at((i)*eachBucketSize-1+last_0_idx) );
    }
    
    maxProcElements.push_back(sorted_arr->at(sorted_arr->size()-1));
}


void distributeSortedArray(Array1DPtr& sorted_arr,  MergeMetaData& meta_data)
{
    int send_idx = 0;
    int prev_send_idx = 0;
    int num_elems_send = send_idx - prev_send_idx;


    for(int i=1; i<meta_data.num_procs; i++){
        send_idx = sorted_arr->findLast(meta_data.maxProcElements[i]);

        if(i==meta_data.num_procs-1) send_idx = sorted_arr->size();

        num_elems_send = send_idx - prev_send_idx;

        MPI_Send(&num_elems_send, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        
        if(num_elems_send !=0)
            MPI_Send(sorted_arr->arrayPtrGet(prev_send_idx), num_elems_send, MPI_FLOAT, i, 1, MPI_COMM_WORLD);

        meta_data.numElementsPerBucket[i] += num_elems_send;

        prev_send_idx = send_idx;
    }

}



void recieveSortedArray(Array1DPtr& storage_elems,int run_number, int my_rank)
{
    int num_elems_recv = 0;
    MPI_Recv(&num_elems_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    if(num_elems_recv != 0)
    {
        Array1DPtr new_storage_elems = Array1DPtr{new Array1D{num_elems_recv}};
        MPI_Recv(new_storage_elems->arrayPtrGet(0), num_elems_recv, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if(run_number == 0){
            storage_elems = new_storage_elems;
        }
        else{
            Array1DPtr out_storage_elems = Array1DPtr{ new Array1D{storage_elems->size()+new_storage_elems->size()}};
            mergeSortedArrays(*storage_elems, *new_storage_elems, *out_storage_elems);

            storage_elems = out_storage_elems;
        }
    }

}


float getQuartile(int q, Array1DPtr& storage_elems, MergeMetaData& meta_data)
{
    //index 0  is rank that stores q2, index 1 is rank's array index that with the median
    int Q_position[2];

    if( meta_data.my_rank == 0)
    {
        std::vector<long int> cumulative_stored_vals;
        cumulative_stored_vals.push_back(0);
        for(int i=1; i<meta_data.numElementsPerBucket.size(); i++)
        {
            auto cumulative = meta_data.numElementsPerBucket[i]+cumulative_stored_vals[i-1];
            cumulative_stored_vals.push_back(cumulative);
        }

        long int total_stored_vals = cumulative_stored_vals[meta_data.numElementsPerBucket.size()-1];
        auto Q_idx = floor(q*total_stored_vals/4.0);

        //find rank that has the 1st cumulative value greater than Q_idx ie. the rank storing
        //the desired quartile
        for(int i=0; i<meta_data.numElementsPerBucket.size(); i++)
        {
            if(cumulative_stored_vals[i]>Q_idx)
            {
                Q_position[0] = i;
                Q_position[1] = Q_idx-cumulative_stored_vals[i-1];
                break;
            }
        }
 
        MPI_Bcast(&Q_position,2,MPI_INT, 0 ,MPI_COMM_WORLD);

        float val = 0;
        MPI_Recv(&val, 1, MPI_FLOAT, Q_position[0], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return val;
    }
    else
    {
        MPI_Bcast(&Q_position,2,MPI_INT, 0 ,MPI_COMM_WORLD);

        if(meta_data.my_rank == Q_position[0]) {
            float val = storage_elems->at(Q_position[1]);
            MPI_Send(&val, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
        }

    }
    return -1.0;
}


float getAbsMin(Array1DPtr& storage_elems, MergeMetaData& meta_data)
{
    //If zeros are included in the dataset then the rank 1 will store the lowest value
    //If zeros are not included in the dataset then rank 2 will store the lowest value
    int rank_with_lowest = 1;

    if(meta_data.my_rank==0)
    {
        //it's possible that zeros are allowed in the dataset but none are stored. In this case
        //the rank with the lowest is rank 0;
        if(meta_data.filterZeros ) rank_with_lowest = 2;
        if(meta_data.numElementsPerBucket[1] == 1) rank_with_lowest = 2;

        MPI_Bcast(&rank_with_lowest, 1, MPI_INT, 0, MPI_COMM_WORLD);

        float val = 0;
        MPI_Recv(&val, 1, MPI_FLOAT, rank_with_lowest, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        return val;
    }
    else{
        MPI_Bcast(&rank_with_lowest, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(meta_data.my_rank == rank_with_lowest)
        {
            float val = storage_elems->at(0);
            MPI_Send(&val, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
        }

    }

    return -1.0;
}

float getAbsMax(Array1DPtr& storage_elems, MergeMetaData& meta_data)
{
    if(meta_data.my_rank==0)
    {
        float val = 0;
        MPI_Recv(&val, 1, MPI_FLOAT, meta_data.num_procs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        return val;
    }
    if(meta_data.my_rank==meta_data.num_procs-1)
    {
        float val = storage_elems->at( storage_elems->size()-1 );
        MPI_Send(&val, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    return -1.0;
}