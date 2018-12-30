#ifndef TIMER
#define TIMER

#include<chrono>
using namespace std::chrono;

class Timer
{
public:
	Timer() : m_begin(high_resolution_clock::now()) {}
	void reset() { m_begin = high_resolution_clock::now(); }


	//ƒ¨»œ ‰≥ˆ√Î
	double elapsed() const
	{
		return duration_cast<duration<double>>(high_resolution_clock::now() - m_begin).count();
	}
	//ƒ¨»œ ‰≥ˆ∫¡√Î
	int64_t elapsed_milli() const
	{
		return duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - m_begin).count();
	}

	//Œ¢√Î
	int64_t elapsed_micro() const
	{
		return duration_cast<std::chrono::microseconds>(high_resolution_clock::now() - m_begin).count();
	}

	//ƒ…√Î
	int64_t elapsed_nano() const
	{
		return duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - m_begin).count();
	}

	//√Î
	int64_t elapsed_seconds() const
	{
		return duration_cast<std::chrono::seconds>(high_resolution_clock::now() - m_begin).count();
	}

	//∑÷
	int64_t elapsed_minutes() const
	{
		return duration_cast<std::chrono::minutes>(high_resolution_clock::now() - m_begin).count();
	}

	// ±
	int64_t elapsed_hours() const
	{
		return duration_cast<std::chrono::hours>(high_resolution_clock::now() - m_begin).count();
	}

private:
	time_point<high_resolution_clock> m_begin;
};


#endif
