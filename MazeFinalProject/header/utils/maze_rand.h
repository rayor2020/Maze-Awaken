#pragma once
#include <random>
class rand_generator {
private:
	int seed;
	int last;
	std::mt19937 engine;
public:
	rand_generator(int seed = time(0)) : seed(seed), last(0){
		engine.seed(seed);
	}
	int operator()(int rmin, int rmax) {
		int ret = last;
		while (ret == last)
			ret = std::uniform_int_distribution<int>(rmin, rmax)(engine);
		last = ret;
		return ret;
	}
};