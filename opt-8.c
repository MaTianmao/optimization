#include "common.h"

//loop unrolling 4*2
void benchmark(double *v, double *dest){
    double sum1 = 0;
    double sum2 = 0; 
    int i; 
    for(i = 0; i < MAX; i += 4){
        sum1 = sum1 * (v[i] * v[i + 1]);
        sum2 = sum2 * (v[i + 2] * v[i + 3]);
    }
    for(; i < MAX; i++) sum1 = sum1 * v[i];
    *dest = sum1 * sum2;
}

int main(){
    double *a = (double *)malloc(sizeof(double) * MAX);
    memset(a, 0, sizeof(a));
    double ans;
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