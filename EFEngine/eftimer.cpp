#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include <Windows.h>
#include <efe\eftimer.h>

namespace efe {
	namespace timer {
		unsigned long long getCount() {
			LARGE_INTEGER time;
			QueryPerformanceCounter(&time);
			return static_cast<unsigned long long>(time.QuadPart);
		}

		double getSeconds(unsigned long long startCount, unsigned long long stopCount) {
			LARGE_INTEGER time;
			QueryPerformanceFrequency(&time);
			return static_cast<double>(stopCount - startCount) / static_cast<double>(time.QuadPart);
		}
	}
}