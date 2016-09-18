#pragma once

#include <stdexcept>

class PerformanceCounterMeter {
public:
	PerformanceCounterMeter() {
		_M_counter = 0L;
		_M_frequency.QuadPart = _M_perf_counter_start.QuadPart = _M_duration.QuadPart = 0L;
		if (FALSE == ::QueryPerformanceFrequency(&_M_frequency))
			throw std::runtime_error("Failed to initialize performance counter."); 
	}

	bool start() {
		return (::QueryPerformanceCounter(&_M_perf_counter_start) != FALSE);
	}

	bool end() {
		LARGE_INTEGER performance_counter;
		bool rc = (::QueryPerformanceCounter(&performance_counter) != FALSE);
		_M_duration.QuadPart += performance_counter.QuadPart - _M_perf_counter_start.QuadPart;
		return rc;
	}

	double get_duration() {
		return (double) _M_duration.QuadPart / _M_frequency.QuadPart;
	}

	bool get_current_duration(double& duration) {
		LARGE_INTEGER performance_counter, counter_duration;
		bool rc = (::QueryPerformanceCounter(&performance_counter) != FALSE);
		counter_duration.QuadPart = performance_counter.QuadPart - _M_perf_counter_start.QuadPart;
		duration = (double) counter_duration.QuadPart / _M_frequency.QuadPart;
		return rc;
	}

	LONGLONG get_counter() {
		return _M_counter;
	}

	void set_iteration_done(LONGLONG iteration_done) {
		_M_counter = iteration_done;
	}

private:
	LARGE_INTEGER _M_frequency;
	LARGE_INTEGER _M_perf_counter_start;
	LARGE_INTEGER _M_duration;
	LONGLONG _M_counter;
};
