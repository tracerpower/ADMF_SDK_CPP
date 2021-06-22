#pragma once
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#define DYNAMIC_OUTPUT __declspec(dllexport)
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__GLIBC__)
#define DYNAMIC_OUTPUT __attribute__ ((visibility("default")))
#else
#define DYNAMIC_OUTPUT
#endif
#include <string>

namespace  CHANGE_COLOR {
	struct DYNAMIC_OUTPUT Result
	{
		float bottomS, bottomV, meanS, meanV, kS, kV;
	};
	Result DYNAMIC_OUTPUT wrap(const std::string& loadPath);
	Result DYNAMIC_OUTPUT changeColor(unsigned char* data, int picWidth, int picHeight, int picChannel);
}
