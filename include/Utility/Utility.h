#pragma once

#include <chrono>
#include <source_location>
#include "Hash.h"
#include "BucketList.h"
#include "Bits.h"
#include "HashMap.h"
#include "Vector.h"
#include "Pool.h"
#include "UID.h"

namespace Utility {
	template<typename Functor>
	float measure_time(Functor functor, std::string before = "", std::string after = "") {
		auto start = std::chrono::high_resolution_clock::now();
		functor();
		std::chrono::duration<float, std::milli> delta = std::chrono::high_resolution_clock::now() - start;
		std::cout << before << delta.count() << after;
		return delta.count();
	}
}