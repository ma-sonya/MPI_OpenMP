#include <iostream>
#include <chrono>
#include <fstream>
#include <math.h>
#include "sequential.h"
#include <omp.h>

using namespace std;

char* FILE_PATH = "./input/5.in";
#define NUM_THREADS 32

long double solve(
    int n, 
    int threads,
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

    // points 0..median-1 will go to the left part.
    // the rest will go to the right part.

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

    long double d1; 
    long double d2;

    #pragma omp task shared(d1) if (threads > 1)
    {
        d1 = solve(median, threads / 2, xs, ys, sorted_by_xs, left_part_y, id_to_x_pos);
    }
    #pragma omp task shared(d2) if (threads > 1) 
    {
        d2 = solve(n - median, threads - threads / 2, xs, ys, sorted_by_xs + median, right_part_y, id_to_x_pos);
    }
    #pragma omp taskwait

    long double d = min(d1,d2);
    long double result = d;

    int *strip = new int[n];
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

int N;
int *xs;
int *ys;
int *sorted_by_x;
int *sorted_by_y;
int *id_to_x_pos;

void read_input() {
    ifstream input_file;
    input_file.open(FILE_PATH);

    input_file >> N;
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

int main(int argc, char**argv) {
    omp_set_dynamic(0);              /** Explicitly disable dynamic teams **/
    omp_set_num_threads(NUM_THREADS); /** Use N threads for all parallel regions **/


    read_input();
    cout << "N = " << N << endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    long double result;
    #pragma omp parallel
    {
        #pragma omp single
        result = solve(N, NUM_THREADS, xs, ys, sorted_by_x, sorted_by_y, id_to_x_pos);
    }   
    show_result(result);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1,1> > time_s = end_time - start_time;

    cout << "Execution time: " << time_s.count() << " seconds" <<endl;

    delete xs;
    delete ys;
    delete sorted_by_x;
    delete sorted_by_y;
    delete id_to_x_pos;
}

