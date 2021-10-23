#include "PCH.hpp"
#include "Timing.hpp"

namespace Timing
{
	Duration::Duration(const MicroSeconds elapsed) :
		H(std::chrono::duration_cast<Timing::Hours>(elapsed)),
		M(std::chrono::duration_cast<Minutes>(elapsed - H)),
		S(std::chrono::duration_cast<Seconds>(elapsed - H - M)),
		Ms(std::chrono::duration_cast<MilliSeconds>(elapsed - H - M - S)),
		Us(std::chrono::duration_cast<MicroSeconds>(elapsed - H - M - S - Ms))
	{
	}

	std::ostream& operator << (std::ostream& os, Days s) { return os << s.count() << "d"; }
	std::ostream& operator << (std::ostream& os, Hours s) { return os << s.count() << "h"; }
	std::ostream& operator << (std::ostream& os, Minutes s) { return os << s.count() << "m"; }
	std::ostream& operator << (std::ostream& os, Seconds s) { return os << s.count() << "s"; }
	std::ostream& operator << (std::ostream& os, MilliSeconds ms) { return os << ms.count() << "ms"; }
	std::ostream& operator << (std::ostream& os, MicroSeconds us) { return os << us.count() << "us"; }
	std::ostream& operator << (std::ostream& os, const Duration& x) { return os << x.H << ' ' << x.M << ' ' << x.S << ' ' << x.Ms << ' ' << x.Us; }

	MicroSeconds Timer::ElapsedMicroSeconds() const
	{
		const auto diff = std::chrono::high_resolution_clock::now() - _start;
		return std::chrono::duration_cast<MicroSeconds>(diff);
	}

	Duration Timer::Elapsed() const
	{
		return Duration(ElapsedMicroSeconds());
	}

	Timer::~Timer()
	{
		std::cout << Elapsed() << std::endl;
	}
}