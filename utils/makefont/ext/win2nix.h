#pragma once
#include <cstdint>
#include <unistd.h>
#include <chrono>

// More basic standard stuffs defined
typedef bool		boolean;
typedef uint32_t	DWORD;
typedef uint16_t	WORD;
typedef uint8_t		BYTE;
typedef void*		HANDLE;
typedef char*		LPTSTR;

// Suspend defined
#define Sleep(ms)	usleep((ms)*1000)

// Timing defined
inline uint32_t timeGetTime() {
	using std::chrono::steady_clock;
	using std::chrono::milliseconds;
	static auto start = steady_clock::now();
	return std::chrono::duration_cast<milliseconds>(
		steady_clock::now() - start
	).count();
}
