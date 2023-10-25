# VACKeyRetrieval
Retrieves VAC module <a href="https://en.wikipedia.org/wiki/ICE_(cipher)"> Ice </a> encryption key by reversing the LCG seed that it was generated with.
<br>
VAC modules generate the output Ice encryption key using the LCG algorithm, it then sends some generated values.
<br>
We can get the initial LCG seed and thus the Ice key from those values because the LCG algorithm is weak. (probably what they do server-side)

<h1> How It Works </h1>
The VAC module does something like this (not exactly, just pseudocode):

``` c++
int seed_arr[34] = {0};
seed_arr[0] = __rdtsc(); // https://c9x.me/x86/html/file_module_x86_id_278.html
int output_arr[4];
int i = 0;
do
    output_arr[i++] = random(seed_arr);
while ( i < 0x20 );

uint64_t ice_key = *((uint64_t *)&output_arr[2]); // ice_key is output_arr[2], output_arr[3]

for (int i = 0x10; i < 0x10; ++i )
      output_buffer[0x1000 + i] = random(seed_arr);
```
What this basically means is, the module generates the Ice key using the random() function with the array seed_arr, it then proceeds to generate some more values using the same seed_arr and send the to the output buffer (which goes to server after).
<br>
Obviously, the server is able to decrypt the output so it is possible using only the return buffer, but how can it be done?

<h2> Reversing LCGs </h2>
<br>
<a href="https://en.wikipedia.org/wiki/Linear_congruential_generator"> Learn what an LCG is. </a>
<br>
<a href="https://math.stackexchange.com/questions/3846942/reversing-an-lcg"> Learn how to reverse an lcg </a>
<br>
<br>

<h2> Specifics </h2>
Let's now look at VACs "custom" LCG generator:

```c++
int VAClcg_orig(int seed_arr[100])
{
	if (*seed <= 0 || (seed[1] = seed_arr[1]) == 0)
	{
		// this is first run
		seed_arr[0] = -seed_arr[0];
		v4 = seed + 41;
		if (seed_arr[0] < 1)
			seed_arr[0] = 1;
		for (i = 39; i >= 0; --i)
		{
			int num = 16807 * seed_arr[0] - 0x7FFFFFFF * (seed_arr[0] / 127773);
			seed_arr[0] = num + 0x7FFFFFFF;
			if (num >= 0)
				seed_arr[0] = num;
			if (i < 32)
				*v4 = seed0;
			--v4;
		}
		seed_arr[1] = seed_arr[2];
	}
	// https://en.wikipedia.org/wiki/Lehmer_random_number_generator#Schrage's_method
	// this is the same as (16807 * (long long)seed[0]) % 2147483647
	// this avoids overflow in 16807 * seed0
	int num = 16807 * seed_arr[0] - 0x7FFFFFFF * (seed_arr[0] / 127773);
	seed_arr[0] = num + 0x7FFFFFFF;
	if (num >= 0)
		seed[0] = num;

    // this is based on last result
	int result_index = seed[1] / 0x4000000 + 2; 
    // this is some previous seed, we need to find out which one
	seed_arr[1] = seed_arr[result_index]; 
	seed_arr[result_index] = seed[0];
	return seed_arr[1];
}
```

The if part is only on the first run, let's focus on the last part: <br>
The algorithm generates a number using an LCG algorithm, with seed_arr[0] used as the seed, we know how to reverse this operation (look above). 
<br> 
After that, the algorithm calculates some index result_index, and returns the current value of seed_arr[result_index] (which was generated in some iteration earlier), it also sets seed_arr[result_index] to be the number which we have generated this iteration.
<br>
In other words the algorithm scrambles the generated numbers "randomly" in the array.
<br>
<h2> The Problem </h2>
<br>
Because we know how to reverse the LCG generation process, one could think that we could just take the values the module stores in the output buffer, and reverse until the initial seed, but how would we know how many "generation" we need to go back to get to the initial seed?
<br>
If not for the array scrambling part of the algorithm, we would easily know how many "generation"s happened before the numbers sent in the output buffer were generated, and go back from there.
<br>
But how can we know how many "generation"s happened before the numbers in the output buffer, considering that the outputs were actually generated some number of generations earlier?
<br>
<h2> The Solution </h2>
<br>
First, let's observe that we can know the result_index value of the next iteration of the algorithm (by doing (result - 2) * 0x4000000), so we know where the next generated value will be stored.
<br>
We also have multiple generated numbers, which were generated one after another (look pseudocode above), so how could we use that?
<br>
If two "generation"s store in the same result_index, the second one returns the generated value of the first one, but we also know in which iteration that value was generated, because we know when the first iteration was called.
<br>
So we have all we need to know, we have a value that we know on which iteration it was generated, we can "go back" from this value (using LCG reversal) the needed amount to get the original seed (look in retrieve_key function for better understanding).
<br>
Now that we have the initial seed, we can easily generate the Ice key ourselves, <b> Profit! </b> 