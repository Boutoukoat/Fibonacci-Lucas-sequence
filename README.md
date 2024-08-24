# The Fibonacci suite

Everyone knows the Fibonacci sequence, from math courses, from computer courses, or from reading the book "Da Vinci Code".

F(0) = 0
F(1) = 1
F(n+1) = F(n) + F(n-1)

# Compile and run

A few examples, timings are in msecs.

```
$ gcc -O3 fl.cpp -lgmp
$ ./a.out 1 2 3 4 5 6 7 8 9 10
F(1) = 1
F(2) = 1
F(3) = 2
F(4) = 3
F(5) = 5
F(6) = 8
F(7) = 13
F(8) = 21
F(9) = 34
F(10) = 55
$ ./a.out 300
F(300) = 222232244629420445529739893461909967206666939096499764990979600
$ ./a.out -s -t 1048575 1048576 1048577 1048578
F(     1048575) -->        3.976 msecs (      727964 bits)
F(     1048576) -->        2.382 msecs (      727965 bits)
F(     1048577) -->        3.558 msecs (      727965 bits)
F(     1048578) -->        2.455 msecs (      727966 bits)
```

# Sequential calculation of the Fibonacci suite

It is easy to show that the size of the nth term is about (0.69 * n) bits

With an ALU of w bits, computing the kth term knowing the 2 preceding terms would cost 
about (n * 0.69) / w  alu operations and 3 times this number in memory operations (2 read and 1 write)

The computation of growing numbers of the sequence from 1 to n would cost n*(n+1)/2 * 0.69/w alu operations

With a 64 bit alu this can be simplified as

```
O(n^2 / 185 + O(n)) alu operations
```

and 3 times more memory operations (2 read and 1 add per alu operation).

# A formula for a "double and add" addition chain

From this generic formula 

```
F(n+m) = F(n) * F(m-1) + F(n+1) * F(m) 
```

- replace m with 1 to get f(n+1)
- replace n with m-1 to get f(m+m-1) == f(2*m-1)
- replace m with n  to get F(n+n) == F(2*n)
- replace m with n+1  to get F(n+n+1) == F(2*n+1)
- replace m with 2n to get f(3*n)
- replace n with 2m to get f(3*m)

and so on ...

```
F(n+1) = F(n) + F(n-1)
....
F(2n-1) = F(n) ^2 + F(n-1) ^2
F(2n) = 2 * F(n) * F(n+1) - F(n) ^2 = ( F(n) + F(n+1) ) ^2 - F(n) ^ 2
F(2n) = 2 * F(n) * F(n-1) - F(n) ^2 = ( F(n) + F(n-1) ) ^2 + F(n) ^ 2
F(2n+1) = F(n) ^2 + F(n+1) ^2
....
F(3n) = ....
```

There are many ways to combine these formula and choose the right ones to implement a "double and add" addition chain ( https://en.wikipedia.org/wiki/Addition_chain )

```
Function F(n)
    if n = 0 then return 0
    if n = 1 then return 1
    f0 = 0;
    f1 = 1;
    mask = first power of 2 larger than n 
    while mask > 1 do
        mask /= 2
        // double
        (f0, f1) = (f0^2 + f1^2, (f0 + f1)^2 - f0^2 )
        if (n & mask) != 0
            // add
            (f0, f1) = (f1, f0 + f1)
    return f0; 
```

Use similar formula, choose the way which limits the number of temporary variables and memory trashing. This depends on language, compiler and hardware.

Note that the size of the numbers double at each iteration, and the whole processing time is approximately the time to run the 2 last iterations.

With a textbook multiplication and when a multiplier is available and is w bits wide;

a multiplication of k-bits numbers costs M = (k/w)^2 alu multiplications 

a square of a k-bits number costs S = M/2 = (k/w)^2/2 alu multiplications

a square of a k/2-bits number costs S/4 multiplications

there are 3 squares per iterations

The last iteration costs (3 * S) squares

The second-last iteration costs 3 * S/4 squares,  total is 15/4 * S

The third-last iteration costs 3 * S/16 squares, total is 63/16 * S

The fourth-last iteration costs 3 * S/64 squares, total is 255/64 * S

.... this converge to a cost of (4 * S) squares

This is about 4 * (n * 0.69 / w)^2   alu multiplications when reaching F(n) and when the multiplier width is 64

```
O(n^2 / 17206 + O(n)) alu multiplications 
```

In fact, it is 1 multiplication + 1 addition + 2 memory read + 1 memory write on x86-64, where all instructions implement pipelining and OOO execution, all these operations would run in parallel with a throughput of 1 cycle.

# Compare

```
O(n^2 / 185 + O(n)) alu operations
O(n^2 / 17206 + O(n)) alu multiplications 
```

The "double and add" method has the same big O() complexity than the "sequential" method, with a O(100) constant improvement.


# A faster calculation of Fibonacci sequence terminated with Lucas sequence

By setting n = s + 2^k and using Lucas sequences, the end of the calculation can be simplified.

## 1 : compute F(s+1) and F(s) as above,

if (k == 0) this is done.

## 2 : go thru Lucas sequence

(https://en.wikipedia.org/wiki/Lucas_sequence#Relations_between_sequences_with_different_parameters)

L(s) = 2 * F(s+1) - F(s)

## 3 : double (if k > 1)

F(2s) = F(s) * L(s)
L(2s) = L(s) ^2 + 2 

## 4 : repeat k-2 times this simplified iteration (1 multiplications, 1 square)

F(2n) = F(n) * L(n)
L(2n) = L(n) ^2 - 2

## 5 : last double (1 multiplication)

F(2n) = F(n) * L(n)

Note that the number size doubles at each iteration, it is very important to reduce the complexity at the end of iteration. 
Here the last iteration is a single multiplication.

## Complexity assuming a O(n^2) multiplication cost

if we were running fibonacci on a power of 2, the number of operations would be

Assuming a multiplication costs 2 squares and, at every iteration, the size of intermediate numbers doubles.

The last iteration is a multiplication and costs 2 * S squares

The second-last iteration costs 3 * S/4 squares,  total is 11/4 * S

The third-last iteration costs 3 * S/16 squares, total is 47/16 * S

The fourth-last iteration costs 3 * S/64 squares, total is 191/64 * S

.... this converge to 3 * S

This is about 3 * (n * 0.69 / w)^2   multiplications when reaching F(n) and when w=64

```
O(n^2 / 22941 + O(n)) multiplications 
```
# Complexity assuming a n*log(n) multiplication cost

A formula like this can be handled with 3 direct transforms and 3 inverse transforms

F(2n) = 2 * F(n) * F(n-1) - F(n) ^2 = ( F(n) + F(n-1) ) ^2 - F(n) ^ 2
F(2n+1) = F(n) ^2 + F(n+1) ^2

A formula like this can be handled with 2 direct transforms and 2 inverse transforms

F(2n) = F(n) * L(n)
L(2n) = L(n) ^2 + 2 

A formula like this can be handled with 2 direct transforms and 1 inverse transforms

F(2n) = F(n) * L(n)

The following is very rough , no need to go to details due to n log(n) very harsh approximation (12 * n * log(2n) might be closer to the reality of such a multiplication)

```
last iteration costs 3 * n log2(n)
previous lucas iteration costs 4 * n/2 log2(n/2)
previous lucas iteration costs 4 * n/4 log2(n/4)
previous lucas iteration costs 4 * n/8 log2(n/8)
...
```

let k = log2(n), i.e n = 2^k and add the cost of all iterations

(3 * 2^k * k) + (4 * 2^(k-1) * (k-1)) + (4 * 2^(k-2) * (k-2)) + (4 * 2^(k-3) * (k-3)) + ... + (4 * 2^0 * 0)

= 4 * [sum(x * 2^x) from 1 to k] - k*2^k

the well known sum(x*2^x) with x from 1 to k is 2 + (k-1)2^(k+1)

= 4*(2 + (k-1)*2^(k+1)) - k*2^k

= 8*(k-1)*2^(k) - k*2^k + 8

= 7*k*2^(k) - 8*2^k + 8

= 7*(log2(n)*2^log2(n)-8*2^log2(n) + 8

= 7*log2(n)*n - 8*n + 8

= O(n * log2(n) + O(n))


This is about

```
O(n * log2(n) + O(n))  alu butterflies  (1 multiplication + 1 subtract + 1 add)
```

# Misc

The multiprecision calculations are based on GMP library. Failures on memory allocation on temporary numbers are handled the GMP way, i.e. by infinite loop, crashing or SIG_FPE signal. 





