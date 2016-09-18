#pragma once
#include <stdexcept>
class Performance_counter_meter
{
public:
	Performance_counter_meter()
	{
		m_counter = 0L;
		m_frequency.QuadPart = m_performance_counter_start.QuadPart = m_duration.QuadPart = 0L;
		if ( FALSE == ::QueryPerformanceFrequency(&m_frequency) )
		{
			throw std::runtime_error("Failed to initialize performance counter."); 
		}
	}
	bool start(void)
	{
		return (::QueryPerformanceCounter(&m_performance_counter_start) != FALSE);
	}
	bool end(void)
	{
		LARGE_INTEGER performance_counter;
		bool rc = (::QueryPerformanceCounter(&performance_counter) != FALSE);
		m_duration.QuadPart += performance_counter.QuadPart - m_performance_counter_start.QuadPart;
		return rc;
	}
	double get_duration(void)
	{
		return (double)m_duration.QuadPart / m_frequency.QuadPart;
	}
	bool get_current_duration(double& duration)
	{
		LARGE_INTEGER performance_counter, counter_duration;
		bool rc = (::QueryPerformanceCounter(&performance_counter) != FALSE);
		counter_duration.QuadPart = performance_counter.QuadPart - m_performance_counter_start.QuadPart;
		duration = (double)counter_duration.QuadPart / m_frequency.QuadPart;
		return rc;
	}
	LONGLONG get_counter(void)
	{
		return m_counter;
	}
	void set_iteration_done(LONGLONG iteration_done)
	{
		m_counter = iteration_done;
	}

private:
	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_performance_counter_start;
	LARGE_INTEGER m_duration;
	LONGLONG m_counter;
};
