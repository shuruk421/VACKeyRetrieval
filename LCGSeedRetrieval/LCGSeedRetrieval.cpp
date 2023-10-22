#include <iostream>
#include <intrin.h>
#include <stdlib.h>

#pragma intrinsic(__rdtsc)

#define MODULUS 2147483647
#define MULTIPLIER 16807

int VAClcg_orig(int seed[100])
{
    int seed0; // esi
    int seed1; // ebx
    int* v4; // ebp
    int i; // ebx
    int something; // ecx
    int some_other_thing; // edx
    int seed0_1; // esi
    int result; // eax

    seed0 = *seed;
    if (*seed <= 0 || (seed1 = seed[1]) == 0)
    {
        seed0 = -seed0;
        v4 = seed + 41;
        if (seed0 < 1)
            seed0 = 1;
        for (i = 39; i >= 0; --i)
        {
            something = 16807 * seed0 - 0x7FFFFFFF * (seed0 / 127773);
            seed0 = something + 0x7FFFFFFF;
            if (something >= 0)
                seed0 = something;
            if (i < 32)
                *v4 = seed0;
            --v4;
        }
        seed1 = seed[2];
        seed[1] = seed1;
    }
    // https://en.wikipedia.org/wiki/Lehmer_random_number_generator#Schrage's_method
    // https://chromium.googlesource.com/native_client/nacl-newlib/+/master/newlib/libc/stdlib/rand_r.c
    // https://dl.acm.org/doi/10.1145/63039.63042
    // https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/random.tcc#L43C30-L43C30
    some_other_thing = 16807 * seed0 - 0x7FFFFFFF * (seed0 / 127773);
    seed0_1 = some_other_thing + 0x7FFFFFFF;
    if (some_other_thing >= 0)
        seed0_1 = some_other_thing;
    // long long test = (16807 * (long long)seed0) % 2147483647; // need long long because of potential overflow from 16807 * seed0
    //std::cout << (test == seed0_1) << std::endl;
    // int test2 = (16807 * (seed0 % 127773)) - 2836 * (seed0 / 127773); // this works! (Schrage's method, avoids overflow)
    
    seed[1] = seed[seed1 / 0x4000000 + 2];
    seed[seed1 / 0x4000000 + 2] = seed0_1;
    result = seed[1];
    *seed = seed0_1;
    return result;
}

int VAClcg(int* seed)
{
    int seed0; // esi
    int seed1; // ebx
    int* v4; // ebp
    int i; // ebx
    int something; // ecx
    int after_lcg; // edx
    int new_seed; // esi
    int result; // eax

    seed0 = seed[0];
    if (*seed <= 0 || (seed1 = seed[1]) == 0)
    {
        // this is first time
        seed0 = -seed0;
        v4 = seed + 41;
        if (seed0 < 1)
            seed0 = 1;
        for (i = 39; i >= 0; --i)
        {
            something = (MULTIPLIER * seed0) % MODULUS;
            seed0 = something + 0x7FFFFFFF;
            if (something >= 0)
                seed0 = something;
            if (i < 32)
                *v4 = seed0;
            --v4;
        }
        seed1 = seed[2];
        seed[1] = seed1;
    }
    // not first time
    after_lcg = (MULTIPLIER * seed0) % MODULUS;
    new_seed = after_lcg + 0x7FFFFFFF;
    if (after_lcg >= 0)
        new_seed = after_lcg;
    seed[1] = seed[seed1 / 0x4000000 + 2];
    seed[seed1 / 0x4000000 + 2] = new_seed;
    *seed = new_seed;
    return seed[1];
}

int VAClcg_notfirst(int* seed) {
    // not first time
    int after_lcg = 16807 * seed[0] - 0x7FFFFFFF * (seed[0] / 127773);
    int new_seed = after_lcg + 0x7FFFFFFF;
    if (after_lcg >= 0)
        new_seed = after_lcg;
    seed[1] = seed[seed[1] / 0x4000000 + 2];
    seed[seed[1] / 0x4000000 + 2] = new_seed;
    *seed = new_seed;
    return seed[1];
}

// A naive method to find modular
// multiplicative inverse of 'A'
// under modulo 'M'
long long modInverse(long long A, int M)
{
    for (int X = 1; X < M; X++)
        if (((A % M) * (X % M)) % M == 1)
            return X;
}

int reverseOne(long long next) {
    int modInversed = 0x53e77248; // modInverse(MULTIPLIER, MODULUS);
    long long reversed = (next * modInversed) % MODULUS;
    return reversed;
}


// https://math.stackexchange.com/questions/3846942/reversing-an-lcg
int main()
{
    int seed[100] = {0};
    *seed = 0xF03650B6;
    std::cout << "init seed: " << std::hex << *seed << std::endl;
    //std::cout << "original seed: " << std::hex << *seed << std::endl;
    int res0 = VAClcg_orig(seed);
    std::cout << "after first run seed: " << std::hex << *seed << std::endl;
    for (int i = 0; i < 0x20; i++) {
        res0 = VAClcg_orig(seed);
        std::cout << "old seed prediction: " << std::hex << reverseOne(*seed) << std::endl;
        std::cout << "new seed:            " << std::hex << *seed << std::endl;
    }

    // sent to server
    for (int i = 0; i < 0x22; i++) {
        int res0 = VAClcg_orig(seed);
        std::cout << "sent to server: " << std::hex << res0 << std::endl;
    }
}