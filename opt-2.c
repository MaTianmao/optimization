#include "common.h"

//not judge the boundary
void benchmark(double *v, double *dest){
    for(int i = 0; i < MAX; i++){
        *dest = *dest * v[i];
    }
}

int main(){
    double *a = (double *)malloc(sizeof(double) * MAX);
    memset(a, 0, sizeof(a));
    double ans = 1;
    double start, end;

    //warm up
    benchmark(a, &ans);
    start = wall_time();
    for(int i = 0; i < n_iterator; i++){
        benchmark(a, &ans);
    }
    end = wall_time();
    printf("%d iterators: runtime of this algorimth is: %.4f s\n", n_iterator, (end-start));
    free(a);
    return 0;
}