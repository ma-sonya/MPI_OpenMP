#include <iostream>
#include <vector>
#include <algorithm>
#include <limits.h>
#include <random>
#include <fstream>
using namespace std;

// Algorithm that generates input data.
// Firstly, there is a list of triples:
// point_id, x, y
// Then, there is the list of these points sorted by x.
// Then, there is the list of these points sorted by y.

vector<pair<int, int> > xs;
vector<pair<int,int> > ys;

int MAX_COORD = 100000000;
int MIN_COORD = -MAX_COORD;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> rand_int(MIN_COORD,MAX_COORD);

int main (int argc, char**argv) {
    int N = atoi(argv[1]);
    char* file_name = argv[2];

    ofstream myfile;
    myfile.open (file_name);

    myfile << N << endl;
    for(int i = 0; i < N; i++) {
        int x = rand_int(rng);
        int y = rand_int(rng);

        xs.push_back(make_pair(x, i)); 
        ys.push_back(make_pair(y, i)); 
        myfile << i << ' ' << x << ' ' << y << endl;
    }

    sort(xs.begin(), xs.end());
    sort(ys.begin(), ys.end());

    for(auto p : xs) {
        myfile << p.second << ' ';
    }
    myfile << std::endl;
    for(auto p : ys) {
        myfile << p.second << ' ';
    }
    myfile << std::endl;

    myfile.close();


    return 0;
}