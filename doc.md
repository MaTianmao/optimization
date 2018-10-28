# 微计算机系统设计第二次实验——Optimization

> 完成人：马少楠 2018320823



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
    double sum = 0;
    for(int i = 0; i < MAX; i++){
        sum = sum * v[i];
    }
    *dest = sum;
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
    double sum = 0;
    int i; 
    for(i = 0; i < MAX; i += 2){
        sum = (sum * v[i]) * v[i + 1];
    }
    for(; i < MAX; i++) sum = sum * v[i];
    *dest = sum;
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
    double sum = 0;
    int i; 
    for(i = 0; i < MAX; i += 2){
        sum = sum * (v[i] * v[i + 1]);
    }
    for(; i < MAX; i++) sum = sum * v[i];
    *dest = sum;
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

到八路和十六路展开的时候性能提升很小，猜测使用八到十六路循环展开可以最大化的消除数据间依赖性。



### opt-11

