#include "common.h"

int get_ele(double *val, double *v, int i){
    if(i >= 0 && i < MAX){
        *val = v[i];
        return 1;
    }
    return 0;
}
void benchmark(double *v, double *dest){
    for(int i = 0; i < MAX; i++){
        double val;
        if(get_ele(&val, v, i)){
            *dest = *dest * val;
        }
    }
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