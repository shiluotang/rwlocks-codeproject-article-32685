#pragma once
 
#include <windows.h>

class RWCriticalSection
{
public:
	void acquireLockShared()
	{
		::EnterCriticalSection(&m_cs);
	}

	void acquireLockExclusive()
	{
		::EnterCriticalSection(&m_cs);
	}

	void releaseLockShared()
	{
		::LeaveCriticalSection(&m_cs);
	}

	void releaseLockExclusive()
	{
		::LeaveCriticalSection(&m_cs);
	}

	RWCriticalSection()
	{
		::InitializeCriticalSection(&m_cs);
	}
	
	~RWCriticalSection()
	{
		::DeleteCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
};
