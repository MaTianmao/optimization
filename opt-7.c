#include "common.h"

//loop unrolling 4*1
void benchmark(double *v, double *dest){
    double sum = 1;
    int i; 
    for(i = 0; i < MAX; i += 4){
        sum = sum * ((v[i] * v[i + 1]) * (v[i + 2] * v[i + 3]));
    }
    for(; i < MAX; i++) sum = sum * v[i];
    *dest = *dest * sum;
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
    printf("%.2f\n", ans);
    free(a);
    return 0;
}