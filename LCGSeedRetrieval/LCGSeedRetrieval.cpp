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
		// this is first run
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
	seed[1] = seed[result_index]; // this is some previous seed, we need to find out which one
	seed[result_index] = seed0_1;
	result = seed[1];
	*seed = seed0_1;
	return result;
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

// https://math.stackexchange.com/questions/3846942/reversing-an-lcg
// this gets the previous seed from the input seed
int reverseOne(long long next) {
	int modInversed = 0x53e77248; // modInverse(MULTIPLIER, MODULUS);
	long long reversed = (next * modInversed) % MODULUS;
	return reversed;
}

void retrieve_key(int sentVals[100], int ret_buff_out_size) {
	std::map<int, int> result_index_map;
	bool dupeIndex = false;

	int seed0;
	int seed0Index;

	for (int i = 0; i < 0x22; i++) {
		if (dupeIndex) {
			// this is a dupe result, so the return value is a seed generated on a known index
			seed0 = sentVals[i];
			std::cout << "seed0: " << std::hex << seed0 << std::endl;
			break;
		}

		int next_result_index = (sentVals[i] / 0x4000000) + 2; // the result index of the next run
		auto it = result_index_map.find(next_result_index);
		if (it != result_index_map.end()) {
			// already contains this result index
			// this means that when we do result = seed[result_index]; the returned value will be a previous seed
			// because on the last run with the same result index: seed[result_index] = seed0;
			// if we know on what iteration that happened (i) we can reverse the original seed using reverseOne();
			// this will happen for sure because we run more times than array size: (actually not sure about that since VAC modules send less, but seems to work)
			// arrSize = 0x22 - 2 (indexes 0, 1 which aren't used as result index), we run 0x22 times.

			seed0Index = it->second + 1;
			std::cout << "list already contains index!!!!: " << next_result_index << " i: " << seed0Index << std::endl;
			dupeIndex = true;
		}
		// add result index to map
		result_index_map[next_result_index] = i + 1;
	}

	// use reverseOne to reverse the original seed
	int reversed = seed0;
	// this is just for calculating how many values were generated before the values sent to the server,
	// we use this to know how many reverseOne() calls we need until we get the original seed.
	int moduleConst = 0x422 - (ret_buff_out_size / 4) + 26;
	for (int j = 0; j < seed0Index + 0x1f + moduleConst; j++)
	{
		reversed = reverseOne(reversed);
		// std::cout << std::hex << -reversed << std::endl;
	}
	// on the first VAClcg() the original seed is negated so we negate it back to get the original
	reversed = -reversed;
	std::cout << "Retrieved initial seed: " << reversed << std::endl;

	// use original seed to retrieve key, by generating it
	int retrievedSeed[100] = { 0 };
	retrievedSeed[0] = reversed;
	// first call
	int res = VAClcg_orig(retrievedSeed);
	for (int i = 0; i <= 2; i++) {
		res = VAClcg_orig(retrievedSeed);
		if (i == 1 || i == 2) { // key is 3rd and 4th random calls
			std::cout << "retrieved key: " << std::hex << res << std::endl;
		}
	}
	// the key is char[8], we print it in two parts (could be easily converted to int64_t etc...)
}

int main()
{
	// simulate what the VAC module does (generate key from seed, and generate sentVals)
	//int seed[100] = { 0 };
	//*seed = 0x94079E2E; // VAC does: -std::abs(__rdtsc());

	//std::cout << "init seed: " << std::hex << *seed << std::endl;
	//int res0 = VAClcg_orig(seed);
	//std::cout << "after first run seed: " << std::hex << *seed << std::endl;
	//for (int i = 0; i < 0x1f; i++) {
	//	res0 = VAClcg_orig(seed);
	//	if (i == 1 || i == 2) { // key is 3rd and 4th random calls
	//		std::cout << "key: " << std::hex << res0 << std::endl;
	//	}

	//	// std::cout << "result= " << res0 << std::endl;
	//	// std::cout << "old seed prediction: " << std::hex << reverseOne(*seed) << std::endl;
	//	// std::cout << "new seed:            " << std::hex << *seed << std::endl;
	//}

	//int sentVals[100] = { 0 };
	//// generate sent to server
	//for (int i = 0; i < 0x22; i++) {
	//	int res = VAClcg_orig(seed);
	//	sentVals[i] = res;
	//	std::cout << "sent to server: " << std::hex << res << std::endl;
	//}

	// this is module_return_buffer[1043] until end,
	// these are the random values than the module sends to the server for it to retrieve the encryption key.
	int sentVals[100] =
		{ 0x1755E699,0x6AEF761E, 0x0A2C7BE3, 0x0F33E26B, 0x7F04C3B3, 0x2B4DE1E5, 0x4127D259, 0x4B1D36A8,0x89652C7, 0x4374FDD7, 0x5B0B2623, 0x24E18B42, 0x39E67DAB, 0x5175FEB3, 0x45148B38 };
	
	// retrive key from sentVals
	int ret_buff_out_size = 0xC00; // this is a const for module B330ABA9 (first one that runs when game is launched), most modules use 0x1000
	retrieve_key(sentVals, ret_buff_out_size);

	return 0;
}
