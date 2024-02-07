#include <random>
#include <chrono>
#include <iostream>

struct uuid
{
	uint64_t first;
	uint64_t last;
};

uuid gen_uuid()
{
	uuid ret;
	typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
	uint32_t seed_val = std::chrono::system_clock::now().time_since_epoch().count();
	MyRNG rng;
	rng.seed(seed_val);
	std::uniform_int_distribution<uint64_t> uint_dist;
	ret.first = uint_dist(rng);
	ret.last = uint_dist(rng);
	std::cout << ret.first << std::endl;
	std::cout << ret.last << std::endl;
	return ret;
}
