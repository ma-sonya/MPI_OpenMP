#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "sequential.h"
using namespace std;

int WORLD_SIZE;
const int SEND_JOB_TAG = 0;
const int RECEIVE_ANS_TAG = 1;

char* FILE_PATH = "./input/5.in";

int N;
int *xs;
int *ys;
int *sorted_by_x;
int *sorted_by_y;
int *id_to_x_pos;
// This array is to temporarily used in subcalls.
int *old_id_to_new_id;


void read_input() {
    ifstream input_file;
    input_file.open(FILE_PATH);

    input_file >> N;
    cout << "N = " << N << endl;
    xs = new int[N];
    ys = new int[N];
    sorted_by_x = new int[N];
    sorted_by_y = new int[N];
    id_to_x_pos = new int[N];

    for(int i = 0 ; i < N; i++) {
        int id, x, y;
        input_file >> id >> x >> y;
        xs[id] = x;
        ys[id] = y;
    }
    for(int i = 0; i < N; i++) {
        int id;
        input_file >> id;
        sorted_by_x[i] = id;
        id_to_x_pos[id] = i;
    }
    for(int i = 0; i < N; i++) {
        int id;
        input_file >> id;
        sorted_by_y[i] = id;
    }
    input_file.close();
}

void send_child_the_job(
    int child_rank,
    int n, 
    int *xs,
    int *ys,
    int *sorted_by_xs, 
    int *sorted_by_ys,
    int *id_to_x_pos
) {
    // So here we make sure that ids are in the range [0..n-1].
    // The new ids are in the order of being sorted by x.
    const int DATA_SIZE = 4 * n + 1;
    int *data_to_send = new int[DATA_SIZE];
    int *new_xs = data_to_send + 1;
    int *new_ys = data_to_send + n + 1;
    int *new_sorted_by_xs = data_to_send + 2*n + 1;
    int *new_sorted_by_ys = data_to_send + 3*n + 1;

    data_to_send[0] = n;
    for(int i = 0; i < n; i++) {
        new_xs[i] = xs[sorted_by_xs[i]];
        new_ys[i] = ys[sorted_by_xs[i]];

        old_id_to_new_id[sorted_by_xs[i]] = i;
    }
    for(int i = 0; i < n; i++) {
        new_sorted_by_xs[i] = i;
        new_sorted_by_ys[i] = old_id_to_new_id[sorted_by_ys[i]];    
    }

    MPI_Send(data_to_send, DATA_SIZE, MPI_INT, child_rank, SEND_JOB_TAG, MPI_COMM_WORLD);
}

void receive_data_from_parent(
    int parent_rank
) {
    // The process should receive:
    // n -- the number of the subarray
    // xs, ys -- the coordinates of the elements in the subarray
    // sorted_by_x -- the coordinates of the elements, sorted by x.
    // sorted_by_y -- the coordinates of the elements, sorted by y.
    MPI_Status status;

    MPI_Probe(parent_rank, SEND_JOB_TAG, MPI_COMM_WORLD, &status);

    int message_len;
    // Find out the number of elements in the message -> size goes to "n"
    MPI_Get_count(&status, MPI_INT, &message_len);
    int n = (message_len - 1)/4;
   
    int *data = new int[message_len];

    xs = (data + 1);
    ys = (data + n + 1);
    sorted_by_x = (data + 2*n + 1);
    sorted_by_y = (data + 3*n + 1);

    id_to_x_pos = new int[message_len];
    // Receive the message. ignore the status
    MPI_Recv(data, message_len, MPI_INT, parent_rank, SEND_JOB_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for(int i = 0; i < n; i++) {
        id_to_x_pos[sorted_by_x[i]] = i;
    }
    N = data[0];
}

long double receive_result_from_child(
    int child_rank
) {
    long double result;
    MPI_Recv(&result, 1, MPI_LONG_DOUBLE, child_rank, RECEIVE_ANS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return result;
}

void send_result_to_parent(int parent_rank, long double result) {
    MPI_Send(&result, 1, MPI_LONG_DOUBLE, parent_rank, RECEIVE_ANS_TAG, MPI_COMM_WORLD);
}

long double solve(
    int rank,
    int h,
    int n, 
    int *xs,
    int *ys,
    int *sorted_by_xs, 
    int *sorted_by_ys,
    int *id_to_x_pos
) {
    if(n == 2) {
        return point_dist(sorted_by_xs[0], sorted_by_xs[1], xs, ys);
    }
    if(n == 1) {
        // For simplicity, we will use `inf`
        // for distance within a set of one point
        return inf;
    }

    int median = n/2;
    int median_id = sorted_by_xs[median];

    int *left_part_y = new int[median];
    int *right_part_y = new int[n - median];

    int median_pos_in_sorted_array = id_to_x_pos[median_id];

    int *cur_left_y_ptr = left_part_y;
    int *cur_right_y_ptr = right_part_y;
    for(int i = 0; i < n; i++) {
        int cur_position = id_to_x_pos[sorted_by_ys[i]];

        if(cur_position < median_pos_in_sorted_array) {
            *cur_left_y_ptr = sorted_by_ys[i];
            cur_left_y_ptr++;
        } else {
            *cur_right_y_ptr = sorted_by_ys[i];
            cur_right_y_ptr++;
        }
    }

    int child_rank = (h | rank);

    long double d1;
    long double d2;
    
    if(child_rank >= WORLD_SIZE) {
        // No more kids are possible.
        // We need to solve everything fully sequentially.
        d1 = sequential_solve(median, xs, ys, sorted_by_xs, left_part_y, id_to_x_pos);
        d2 = sequential_solve(n - median, xs, ys, sorted_by_xs + median, right_part_y, id_to_x_pos);
    
    } else {
        send_child_the_job(child_rank, median, xs, ys, sorted_by_xs, left_part_y, id_to_x_pos);
        d2 = solve(rank, h * 2, n - median, xs, ys, sorted_by_xs + median, right_part_y, id_to_x_pos);
        d1 = receive_result_from_child(child_rank);
    }

    long double d = min(d1,d2);
    long double result = d;

    int *strip;
    strip = new int[n];
    
    // Real stip size
    int strip_size = 0;

    for(int i = 0; i < n; i++) {
        //The stip must be sorted by y:
        int id = sorted_by_ys[i];
        if(abs(xs[id] - xs[median_id]) < d) {
            strip[strip_size++] = id;
        }
    }
    
    for(int i = 0; i < strip_size; i++) {
        int id_i = strip[i];
        // It is proven that this cycle can not run longer than 7 iterations
        for(int j = i + 1; j < strip_size; j++) {
            int id_j = strip[j];
            if(abs(ys[id_i] - ys[id_j]) <= d) {
                result = min(result, point_dist(id_i, id_j, xs, ys));
            } else {
                break;
            }
        }
    }

    return result;
}

int get_h(int id) {
    if(id == 0) {
        throw "this method should be used only for non-zero id";
    }

    int max_pow_two = -1;

    while(id) {
        max_pow_two += 1;
        id /= 2;
    }
    return (1 << max_pow_two);
}

void begin_solve(int rank) {
    if(rank != 0) {
        int h = get_h(rank);
        int parent = rank & (h - 1);
        receive_data_from_parent(parent);
        old_id_to_new_id = new int[N];
        
        long double result = solve(rank, h * 2, N, xs, ys, sorted_by_x, sorted_by_y, id_to_x_pos);
        send_result_to_parent(parent, result);
    } else {
        old_id_to_new_id = new int[N];

        long double result = solve(rank, 1, N, xs, ys, sorted_by_x, sorted_by_y, id_to_x_pos);
        show_result(result);
    }
}

int main(int argc, char** argv) {

    
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &WORLD_SIZE);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
        // The first process should read the input.
        read_input();
    }
    auto start_time = std::chrono::high_resolution_clock::now();

    begin_solve(rank);

    // Finalize the MPI environment.
    MPI_Finalize();

    if(rank == 0 ) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::ratio<1,1> > time_s = end_time - start_time;

        cout << "Execution time: " << time_s.count() << " seconds" <<endl;
    }
}
