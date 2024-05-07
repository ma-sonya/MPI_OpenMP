#include <cmath>
using namespace std;
const long double inf = 1e18;

void show_result(long double result) {
    cout << "Result = " << result << endl;
}

long double point_dist(
    int id1, 
    int id2,
    int *xs,
    int *ys
) {
    long double x1 = xs[id1];
    long double y1 = ys[id1];

    long double x2 = xs[id2];
    long double y2 = ys[id2];

    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// Accepts an array of n points.
// Note, that `xs` and `ys` points to array
// with the lengths of all the points (i.e. their size can be more than n).
// While `sorted_by_xs` and `sorted_by_ys` arrays are of size n.
// `id_to_x_pos` is a mapping from point's "id", to its position in the 
// global `sorted_by_xs` array.
long double sequential_solve(
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

    long double d1 = sequential_solve(median, xs, ys, sorted_by_xs, left_part_y, id_to_x_pos);
    long double d2 = sequential_solve(n - median, xs, ys, sorted_by_xs + median, right_part_y, id_to_x_pos);

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
