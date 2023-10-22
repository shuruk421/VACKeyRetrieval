#include <iostream>
#include <intrin.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <map>

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
	// std::cout << (test == seed0_1) << std::endl;
	// int test2 = (16807 * (seed0 % 127773)) - 2836 * (seed0 / 127773); // this works! (Schrage's method, avoids overflow)

	int result_index = seed1 / 0x4000000 + 2; // this is based on last result
	// std::cout << "result_index: " << result_index << " based on " << seed1 << std::endl;
	seed[1] = seed[result_index];
	seed[result_index] = seed0_1;
	result = seed[1];
	*seed = seed0_1;
	return result;

	// want to get seed[0], got seed[1]
	// got seed[result_index]
	// reverse(seed[1]) = reverse(seed[result_index]) = seed0 of some iteration
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

void retrieve_key(int sentVals[100]) {
	std::map<int, int> result_index_map;
	bool dupeIndex = false;

	int seed0;
	int seed0Index;
	
	for (int i = 0; i < 0x22; i++) {
		if (dupeIndex) {
			seed0 = sentVals[i];
			std::cout << "seed0: " << seed0 << std::endl;
			break;
		}
		int next_result_index = (sentVals[i] / 0x4000000) + 2;
		auto it = result_index_map.find(next_result_index);
		if (it != result_index_map.end()) {
			// already contains index
			seed0Index = it->second + 1;
			// this will happen for sure because we run more times than array size
			std::cout << "list already contains index!!!!: " << next_result_index << " i: " << seed0Index << std::endl;
			dupeIndex = true;
		}
		result_index_map[next_result_index] = i + 1;
	}

	int reversed = seed0;
	for (int j = 0; j < seed0Index + 0x1f + 40 + 1; j++)
	{
		reversed = reverseOne(reversed);
		// std::cout << reversed << std::endl;
	}
	reversed = -reversed;
	std::cout << "Retrieved initial seed: " << reversed << std::endl;

	int retrievedSeed[100] = { 0 };
	retrievedSeed[0] = reversed;
	int res = VAClcg_orig(retrievedSeed);
	for (int i = 0; i <= 2; i++) {
		res = VAClcg_orig(retrievedSeed);
		if (i == 1 || i == 2) { // key is 3rd and 4th random calls
			std::cout << "retrieved key: " << std::hex << res << std::endl;
		}
	}
}

// https://math.stackexchange.com/questions/3846942/reversing-an-lcg
int main()
{
	int seed[100] = { 0 };
	*seed = 0x94079E2E;
	std::cout << "init seed: " << std::hex << *seed << std::endl;
	int res0 = VAClcg_orig(seed);
	std::cout << "after first run seed: " << std::hex << *seed << std::endl;
	for (int i = 0; i < 0x1f; i++) {
		res0 = VAClcg_orig(seed);
		if (i == 1 || i == 2) { // key is 3rd and 4th random calls
			std::cout << "key: " << std::hex << res0 << std::endl;
		}

		std::cout << "result= " << res0 << std::endl;
		// std::cout << "old seed prediction: " << std::hex << reverseOne(*seed) << std::endl;
		std::cout << "new seed:            " << std::hex << *seed << std::endl;
	}

	std::cout << "last seed: " << std::hex << *seed << std::endl;

	int sentVals[100] = { 0 };
	// generate sent to server
	for (int i = 0; i < 0x22; i++) {
		int res = VAClcg_orig(seed);
		sentVals[i] = res;
		std::cout << "sent to server: " << std::hex << res << std::endl;
	}

	// retrive key from sentVals
	retrieve_key(sentVals);
}
