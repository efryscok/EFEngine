#pragma once

namespace efe {
	namespace timer {
		unsigned long long getCount();
		double getSeconds(unsigned long long startCount, unsigned long long stopCount);
	}
}