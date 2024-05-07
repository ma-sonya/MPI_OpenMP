#include <iostream>
#include <chrono>
#include <fstream>
#include <math.h>
#include "sequential.h"
using namespace std;

char* FILE_PATH = "./input/8.in";

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

    read_input();
    cout << "N = " << N << endl;

    auto start_time = std::chrono::high_resolution_clock::now();
    
    long double result = sequential_solve(N, xs, ys, sorted_by_x, sorted_by_y, id_to_x_pos);
    show_result(result);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1,1>> time_s = end_time - start_time;

    cout << "Execution time: " << time_s.count() << " seconds" <<endl;

    delete xs;
    delete ys;
    delete sorted_by_x;
    delete sorted_by_y;
    delete id_to_x_pos;
}

