# 微计算机系统设计第二次实验——Optimization

> 完成人：马少楠 2018320823

[TOC]

---

## 实验目标

- 使用不同的程序优化手段，优化`combine`的代码
- 通过实验来比较每种优化带来的性能提升，掌握各种程序优化手段
- 掌握编译器和CPU可以做的优化，以及需要程序员手动做的优化



## 实验过程

### 测试方法

```c
//warm up
benchmark(a, &ans);
start = wall_time();
for(int i = 0; i < n_iterator; i++){
    benchmark(a, &ans);
}
end = wall_time();
printf("%d iterators: runtime of this algorimth is: %.4f s\n", n_iterator, (end-start));
```

当前的实验测试的编译选项为`CFLAGS = -fopenmp -lpthread -mavx2`，未使用`O2`等优化选项。

### opt-1

首先实现一个最基础的版本，基本仿照课本上的实现，对于每次获取元素，都会判断是否越界，然后每次计算都会访问两次`dest`的内存区域。

#### 算法实现

```c
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
```

`benchmark`函数对数组`v`进行乘积运算，运算过程中会调用`get_ele`判断是否越界，每次将计算结果存在`*dest`中。

#### 测试结果

```
20 iterators: runtime of this algorimth is: 6.6844 s
```



### opt-2

取消每次在循环中调用边界判断，循环中仅计算结果。

#### 算法实现

```c
//not judge the boundary
void benchmark(double *v, double *dest){
    for(int i = 0; i < MAX; i++){
        *dest = *dest * v[i];
    }
}
```

每次计算中依然访问两次`dest`的内存区域。

#### 测试结果

```
20 iterators: runtime of this algorimth is: 5.3796 s
```



### opt-3

减少每次在循环中计算时的两次访问`dest`内存，用一个临时变量暂存计算结果。

#### 算法实现

```c
//not read memory many times
void benchmark(double *v, double *dest){
    double sum = 1;
    for(int i = 0; i < MAX; i++){
        sum = sum * v[i];
    }
    *dest = *dest * sum;
}
```

每次计算将结果存储在`sum`这个临时变量中，理论上会对性能有所提高。

#### 测试结果

```
20 iterators: runtime of this algorimth is: 5.4202 s
```



#### 结果分析

发现`opt-3`相比`opt-2`几乎没有多少提高，猜测编译器可能做了一些优化。

使用`gcc -S opt-2.c`和`gcc -S opt-3.c`来观察两个算法的汇编代码：

opt-2汇编：

```assembly
benchmark:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L5
.L6:
	movq	-32(%rbp), %rax
	movsd	(%rax), %xmm1
	movl	-4(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movsd	(%rax), %xmm0
	mulsd	%xmm1, %xmm0
	movq	-32(%rbp), %rax
	movsd	%xmm0, (%rax)
	addl	$1, -4(%rbp)
.L5:
	cmpl	$99999999, -4(%rbp)
	jle	.L6
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
```

opt-3汇编：

```assembly
benchmark:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	pxor	%xmm0, %xmm0
	movsd	%xmm0, -8(%rbp)
	movl	$0, -12(%rbp)
	jmp	.L5
.L6:
	movl	-12(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movsd	(%rax), %xmm0
	movsd	-8(%rbp), %xmm1
	mulsd	%xmm1, %xmm0
	movsd	%xmm0, -8(%rbp)
	addl	$1, -12(%rbp)
.L5:
	cmpl	$99999999, -12(%rbp)
	jle	.L6
	movq	-32(%rbp), %rax
	movsd	-8(%rbp), %xmm0
	movsd	%xmm0, (%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
```

循环内的部分在`L6`段，发现两个算法的`L6`部分如下：

```assembly
//opt-2
.L6:
	movq	-32(%rbp), %rax
	movsd	(%rax), %xmm1    //读取*dest
	movl	-4(%rbp), %eax   //i
	cltq
	leaq	0(,%rax,8), %rdx //一个double 8Byte
	movq	-24(%rbp), %rax  
	addq	%rdx, %rax       //v + 8*i
	movsd	(%rax), %xmm0    //读取v[i]
	mulsd	%xmm1, %xmm0     // *dest * v[i]
	movq	-32(%rbp), %rax  
	movsd	%xmm0, (%rax)    //存回dest的内存区域
	addl	$1, -4(%rbp)
//opt-3
.L6:
	movl	-12(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax       // v + 8*i
	movsd	(%rax), %xmm0    //读取v[i]
	movsd	-8(%rbp), %xmm1  //读取sum
	mulsd	%xmm1, %xmm0     // sum * v[i]
	movsd	%xmm0, -8(%rbp)  // 存回sum
	addl	$1, -12(%rbp)
```

看汇编代码可以发现`opt-2`是比`opt-3`在循环内的每一次计算，多读取了一次内存以及多写回了一次内存，但是性能并没有降低的原因，猜测可能是因为`Cache`的原因。



### opt-4

因为现代`CPU`有超标量技术，所以可以利用流水线来实现指令级的并行。当指令之间没有依赖关系的时候，就可以实现指令级并行，`opt-3`的实现中，每一轮循环之间有数据依赖，`sum`依赖前一轮的计算结果，所以无法做到指令级并行，需要手动实现来消除依赖。

使用循环展开来消除数据间的依赖，首先尝试`2×1`的循环展开。

#### 算法实现

```c
//loop unrolling 2*1
void benchmark(double *v, double *dest){
    double sum = 1;
    int i; 
    for(i = 0; i < MAX; i += 2){
        sum = (sum * v[i]) * v[i + 1];
    }
    for(; i < MAX; i++) sum = sum * v[i];
    *dest = *dest * sum;
}
```

展开后先计算`sum * v[i]`，这样做依然有数据依赖，先测试一下结果。

#### 测试结果

```
20 iterators: runtime of this algorimth is: 3.2891 s
```

比`opt-3`还是有不少提高。



### opt-5

针对前面的循环展开，修改计算顺序，充分使用流水线来实现指令级并行。

#### 算法实现

```c
//loop unrolling 2*1a
void benchmark(double *v, double *dest){
    double sum = 1;
    int i; 
    for(i = 0; i < MAX; i += 2){
        sum = sum * (v[i] * v[i + 1]);
    }
    for(; i < MAX; i++) sum = sum * v[i];
    *dest = *dest * sum;
}
```

先计算`v[i] * v[i + 1]`使得上一个循环的`sum * (temp)`可以和下一轮的`v[i] * v[i + 1]`并行。

#### 测试结果

```
20 iterators: runtime of this algorimth is: 2.6468 s
```

相比`opt-4`提高有限，看一下反汇编后的代码：

```assembly
//opt-4
.L6:
	movl	-12(%rbp), %eax     //i
	cltq
	leaq	0(,%rax,8), %rdx    //8*i
	movq	-24(%rbp), %rax
	addq	%rdx, %rax          //v+8*i
	movsd	(%rax), %xmm0       //读取v[i]
	mulsd	-8(%rbp), %xmm0     //sum * v[i]
	movl	-12(%rbp), %eax     // i
	cltq
	addq	$1, %rax            // i + 1
	leaq	0(,%rax,8), %rdx    
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movsd	(%rax), %xmm1       //读取v[i + 1]
	mulsd	%xmm1, %xmm0        
	movsd	%xmm0, -8(%rbp)
	addl	$2, -12(%rbp)
//opt-5
.L6:
	movl	-12(%rbp), %eax     //i
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax          // v+8*i
	movsd	(%rax), %xmm1       //v[i]
	movl	-12(%rbp), %eax     //i
	cltq
	addq	$1, %rax            //i + 1
	leaq	0(,%rax,8), %rdx    
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movsd	(%rax), %xmm0       //v[i + 1]
	mulsd	%xmm1, %xmm0        //v[i] * v[i + 1]
	movsd	-8(%rbp), %xmm1
	mulsd	%xmm1, %xmm0        // sum *
	movsd	%xmm0, -8(%rbp)
	addl	$2, -12(%rbp)
```

看汇编码这两个实现确实按照前面想的进行了汇编，但是无法看到运行时具体流水线是怎么操作的。



### opt-6 && opt-7 && opt-8 && opt-9 && opt-10

使用`2×2` `4×1` `4×2` `8×1` `16×1`的循环展开来进行测试。

#### 算法实现

```c
//loop unrolling 2*2
sum1 = sum1 * v[i];
sum2 = sum2 * v[i + 1];
//loop unrolling 4*1
sum = sum * ((v[i] * v[i + 1]) * (v[i + 2] * v[i + 3]));
//loop unrolling 4*2
sum1 = sum1 * (v[i] * v[i + 1]);
sum2 = sum2 * (v[i + 2] * v[i + 3]);
```

`8×1` `16×1`和`4×1`的实现类似，可以比较测试结果。

#### 测试结果

```
//2×2
20 iterators: runtime of this algorimth is: 2.9851 s
//4×1
20 iterators: runtime of this algorimth is: 1.8008 s
//4×2
20 iterators: runtime of this algorimth is: 2.4076 s
//8×1
20 iterators: runtime of this algorimth is: 1.5250 s
//16×1
20 iterators: runtime of this algorimth is: 1.3936 s
```

测试结果表明，使用两个临时变量进行展开，对性能没有提高，所以后续算法实现都是用一个临时变量存储。

循环展开的路数越大，性能提升的空间越小。



### opt-11

使用`AVX2`指令集来实现`SIMD`

#### 算法实现

```c
//using SIMD
void benchmark(double *v, double *dest){
    __m256d sum = _mm256_setzero_pd();
    int i;
    for(i = 0; i < MAX; i += 4){
        sum = _mm256_mul_pd(sum, _mm256_loadu_pd(v + i));
    }
    double d[4];
    _mm256_storeu_pd(d, sum);
    double ret = d[0] * d[1] * d[2] * d[3];
    for(; i < MAX; i++) ret = ret * v[i];
    *dest = *dest * ret;
}
```

因为数组内元素的类型是`double`，一个`__m256d`类型的长度是`256bit`，正好可以放`4`个`double`，用`SIMD`可以同时4个元素相乘，下标为`4*i`的元素之积记录在`sum`的前`64bit`中，下标为`4*i+1`的元素之积记录在`sum`的第二个`64bit`中，下标为`4*i+2`的元素之积记录在`sum`的第三个`64bit`中，下标为`4*i+3`的元素之积记录在`sum`的第四个`64bit`中。

最后把`sum`变量存储到一个`double`类型的数组中，将`4`个`double`值相乘就是最后结果。

#### 测试结果

```
20 iterators: runtime of this algorimth is: 1.7944 s
```

性能相比和前面的`4*1`循环展开差不多，因为我一次指令计算`4`个`double`的乘法，但是因为数据有依赖性，所以不能把指令用流水线处理，所以一次就是计算`4`个`double`，和`4*1`的循环展开从效果上来说是等价的。



### opt-12

使用`AVX2`指令集加上`4*1`的循环展开

#### 算法实现

```c
//using SIMD and loop unrolling 4*1
void benchmark(double *v, double *dest){
    __m256d sum = _mm256_setzero_pd();
    int i;
    for(i = 0; i < MAX; i += 16){
        sum = _mm256_mul_pd(sum, _mm256_mul_pd( \
                                 _mm256_mul_pd(_mm256_loadu_pd(v + i), _mm256_loadu_pd(v + i + 4)), \
                                 _mm256_mul_pd(_mm256_loadu_pd(v + i + 8), _mm256_loadu_pd(v + i + 12))));
    }
    double d[4];
    _mm256_storeu_pd(d, sum);
    double ret = d[0] * d[1] * d[2] * d[3];
    for(; i < MAX; i++) ret = ret * v[i];
    *dest = *dest * ret;
}
```

把循环展开和`SIMD`结合，一次读取`4`个`__m256d`类型的数进行计算

#### 测试结果

```
20 iterators: runtime of this algorimth is: 0.7090 s
```

效果比起`opt-11`有显著提高



### opt-13

使用`AVX2`指令集加上`8*1`的循环展开

#### 算法实现

```c
//using SIMD and loop unrolling 8*1
void benchmark(double *v, double *dest){
    __m256d sum = _mm256_setzero_pd();
    int i;
    for(i = 0; i < MAX; i += 32){
        sum = _mm256_mul_pd(sum, _mm256_mul_pd( \
                        _mm256_mul_pd( \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i), _mm256_loadu_pd(v + i + 4)), \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i + 8), _mm256_loadu_pd(v + i + 12))), \
                        _mm256_mul_pd( \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i + 16), _mm256_loadu_pd(v + i + 20)), \
                        _mm256_mul_pd(_mm256_loadu_pd(v + i + 24), _mm256_loadu_pd(v + i + 28)))));
    }
    double d[4];
    _mm256_storeu_pd(d, sum);
    double ret = d[0] * d[1] * d[2] * d[3];
    for(; i < MAX; i++) ret = ret * v[i];
    *dest = *dest * ret;
}
```

#### 测试结果

```
20 iterators: runtime of this algorimth is: 0.6679 s
```

增加展开的路数带来的提高已经不是很大了。



### opt-14

使用`openmp`多线程结合`AVX2`指令集以及`8*1`循环展开

#### 算法实现

```c
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
```

将结果存到`THREAD_NUM`个变量中，然后对于每个线程，划分`[tid * (MAX / THREAD_NUM), (tid + 1) * (MAX / THREAD_NUM) - 1]`是该线程的计算范围，然后下面使用类似`opt-13`的实现。

#### 测试结果

对于不同线程数进行了多次测试

```
//4 threads
20 iterators: runtime of this algorimth is: 0.6466 s
//8 threads
20 iterators: runtime of this algorimth is: 0.4853 s
//16 threads
20 iterators: runtime of this algorimth is: 0.3454 s
//32 threads
20 iterators: runtime of this algorimth is: 0.2649 s
//64 threads
20 iterators: runtime of this algorimth is: 0.2426 s
//128 threads
20 iterators: runtime of this algorimth is: 0.2302 s
//256 threads
20 iterators: runtime of this algorimth is: 0.2829 s
```

起初，随着线程数增加，运行时间降低，到`32`线程以上，性能提升变得特别小，资源开销比不上性能提高，到`256`线程的时候性能出现了降低。设置在`32`线程的时候比较好。



### 编译参数的选择

前面的测试使用了`CFLAGS = -fopenmp -lpthread -mavx2`的编译参数，没有使用编译优化选项`O2`等，下面测试`O1、O2、O3、Ofast`等选项。

#### 未使用优化选项

```
//opt1	6.6844 s	origin
//opt2	5.3796 s 	not judge the boundary
//opt3	5.4202 s	not read memory many times
//opt4	3.2891 s	loop unrolling 2*1
//opt5	2.6468 s	loop unrolling 2*1a
//opt6	2.9851 s	loop unrolling 2*2
//opt7	1.8008 s	loop unrolling 4*1
//opt8	2.4076 s	loop unrolling 4*2
//opt9	1.5250 s	loop unrolling 8*1
//opt10 1.3936 s	loop unrolling 16*1
//opt11 1.7944 s	using SIMD
//opt12 0.7090 s	using SIMD and loop unrolling 4*1
//opt13 0.6679 s	using SIMD and loop unrolling 8*1
//opt14 0.2649 s	using SIMD and loop unrolling 8*1 and multi-threads
```

#### 使用O1优化

`O1`提供基础级别的优化，比如：

- 在函数完成时如何进行操作
- 编译器试图合并相同的常量
- 如何处理汇编代码中的条件和非条件分支
- 通过优化如何生成汇编语言中的循环
- 减少或者删除条件分支
- 减少实现if-then语句所需的条件分支
- 试图根据指令周期时间重新安排指令
- 试图确定条件分支最可能的结果
- 编译器执行第二次检查以便减少调度依赖性

使用`O1`优化后测试的结果为：

```
//opt1	4.0407 s	origin
//opt2	4.0973 s	not judge the boundary
//opt3	1.8086 s	not read memory many times
//opt4	1.8048 s	loop unrolling 2*1
//opt5	0.9009 s	loop unrolling 2*1a
//opt6	0.9006 s	loop unrolling 2*2
//opt7	0.4465 s	loop unrolling 4*1
//opt8	0.4471 s	loop unrolling 4*2
//opt9	0.2483 s	loop unrolling 8*1
//opt10 0.2512 s	loop unrolling 16*1
//opt11 0.4461 s	using SIMD
//opt12 0.1116 s	using SIMD and loop unrolling 4*1
//opt13 0.0793 s	using SIMD and loop unrolling 8*1
//opt14 0.0425 s	using SIMD and loop unrolling 8*1 and multi-threads
```

`O1`优化测试出来效果比不用优化选项提高不少，能够把一些因为开发人员失误写慢的部分进行优化，使得每一步优化带来的变得更加明显，比如`2-3`，在不使用优化选项的时候，几乎没有提高，但是用了`O1`出现了显著提高。



#### 使用O2优化

`O2`提供了更加高级的代码优化，但是会占用更长的编译时间：

- 在任何指令使用变量前, 强制把存放再内存位置中的所有变量都复制到寄存器中
- 处理相关的递归的函数调用
- 对循环执行优化并且删除迭代变量
- 消除冗余的代码段
- 通用子表达式消除技术扫描跳转指令

除了以上优化，还有很多优化细节，其中第四点优化在本次实验中深有体会，因为进行计算得到的结果`ans`并未使用，所以使用了`O2`优化后计算时间直接为`0s`，原因是计算`ans`的代码未使用，所以被认为是冗余代码段而消除。

修改正确后，测量得到的运行时间如下：

```
//opt1	1.8012 s	origin
//opt2	1.8030 s	not judge the boundary
//opt3	1.7976 s	not read memory many times
//opt4	1.7996 s	loop unrolling 2*1
//opt5	0.8983 s	loop unrolling 2*1a
//opt6	0.8992 s	loop unrolling 2*2
//opt7	0.4484 s	loop unrolling 4*1
//opt8	0.4512 s	loop unrolling 4*2
//opt9	0.2417 s	loop unrolling 8*1
//opt10 0.2545 s	loop unrolling 16*1
//opt11 0.4485 s	using SIMD
//opt12 0.1116 s	using SIMD and loop unrolling 4*1
//opt13 0.0825 s	using SIMD and loop unrolling 8*1
//opt14 0.0449 s	using SIMD and loop unrolling 8*1 and multi-threads
```

测试结果和`O1`优化基本相同，除了前两个算法，因为`O2`会做一些比较激进的优化，所以直接把前两个算法优化到了和第三个一样快，后面的加速效果类似。

`O2`已经没有太多提高了，所以之后没有测试`O3、Ofast`等编译优化选项。



## 实验总结

- 本次实验通过从一个简单的原始的算法写起，经过一步步优化，实现了算法性能的`100`倍提高。
- 通过每一步优化以及性能测试，清楚地认识到了每一步优化带来的性能提高。
- 通过查阅汇编，以及使用不同的编译选项，了解了编译器能够为程序员做的事情。
- 学习了使用`SIMD`向量指令编程的方法，以及使用`openmp`进行多线程编程的方法。



