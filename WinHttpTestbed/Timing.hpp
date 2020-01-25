#pragma once

#include <chrono>
#include <iostream>

namespace Timing
{
	using Days = std::chrono::duration<uint64_t, std::ratio<86400, 1>>;
	using Hours = std::chrono::duration<uint64_t, std::ratio<3600, 1>>;
	using Minutes = std::chrono::duration<uint64_t, std::ratio<60, 1>>;
	using Seconds = std::chrono::duration<uint64_t, std::ratio<1>>;
	using MilliSeconds = std::chrono::duration<uint64_t, std::ratio<1, 1000>>;
	using MicroSeconds = std::chrono::duration<uint64_t, std::ratio<1, 1000'000>>;

	struct Duration
	{
		Duration(const MicroSeconds elapsed);

		const Hours H;
		const Minutes M;
		const Seconds S;
		const MilliSeconds Ms;
		const MicroSeconds Us;
	};

	std::ostream& operator << (std::ostream& os, Days s);
	std::ostream& operator << (std::ostream& os, Hours s);
	std::ostream& operator << (std::ostream& os, Minutes s);
	std::ostream& operator << (std::ostream& os, Seconds s);
	std::ostream& operator << (std::ostream& os, MilliSeconds ms);
	std::ostream& operator << (std::ostream& os, MicroSeconds us);
	std::ostream& operator << (std::ostream& os, const Duration& x);

	class Timer
	{
	public:
		MicroSeconds ElapsedMicroSeconds() const;
		Duration Elapsed() const;
		~Timer();
	private:
		const std::chrono::high_resolution_clock::time_point m_start = std::chrono::high_resolution_clock::now();
	};
}