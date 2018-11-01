#include "common.h"
#include <immintrin.h>
#include <omp.h>
//using SIMD and loop unrolling 8*1 and multi-threads
void benchmark(double *v, double *dest){
    const int THREAD_NUM = 32;
    omp_set_num_threads(THREAD_NUM);
    __m256d sum[THREAD_NUM];
    double ret[THREAD_NUM];
    for(int i = 0; i < THREAD_NUM; i++) sum[i] = _mm256_setzero_pd();
#pragma omp parallel
{
    int i;
    int tid = omp_get_thread_num();
    for(i = tid * (MAX / THREAD_NUM); i < (tid + 1) * (MAX / THREAD_NUM); i += 32){
        sum[tid] = _mm256_mul_pd(sum[tid], _mm256_mul_pd( \
                        _mm256_mul_pd( \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i), _mm256_loadu_pd(v + i + 4)), \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i + 8), _mm256_loadu_pd(v + i + 12))), \
                        _mm256_mul_pd( \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i + 16), _mm256_loadu_pd(v + i + 20)), \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i + 24), _mm256_loadu_pd(v + i + 28)))));
    }
    double d[4];
    _mm256_storeu_pd(d, sum[tid]);
    ret[tid] = d[0] * d[1] * d[2] * d[3];
    for(; i < (tid + 1) * (MAX / THREAD_NUM); i++) ret[tid] = ret[tid] * v[i];
}
    double res = 1;
    for(int i = 0; i < THREAD_NUM; i++) res = res * ret[i];
    *dest = *dest * res;
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