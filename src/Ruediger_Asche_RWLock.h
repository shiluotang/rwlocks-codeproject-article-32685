#pragma once
 
#include <windows.h>

#ifdef _MSC_VER
#   include <intrin.h>
#   pragma intrinsic(_InterlockedDecrement, _InterlockedIncrement)
#else
#   if defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__)
#       include <x86intrin.h>
#       define _InterlockedDecrement InterlockedDecrement
#       define _InterlockedIncrement InterlockedIncrement
#   endif
#endif

//#define _HAVE_WRITER_CRITICAL_SECRION_LOCK_

class Ruediger_Asche_RWLock
{
public:
	void acquireLockShared()
	{
		if (::_InterlockedIncrement(&iCounter) == 0)
		{ 
			::WaitForSingleObject(hMutex, INFINITE);
			::SetEvent(hReaderEvent);
		}
		else
		{
		}
		::WaitForSingleObject(hReaderEvent,INFINITE);
	}

	void acquireLockExclusive()
	{
#ifdef _HAVE_WRITER_CRITICAL_SECRION_LOCK_
		::EnterCriticalSection(&m_cs);
#else
		::WaitForSingleObject(hWriterMutex,INFINITE);
#endif
		::WaitForSingleObject(hMutex, INFINITE);
	}

	void releaseLockShared()
	{
		if (::_InterlockedDecrement(&iCounter) < 0)
		{ 
			::ResetEvent(hReaderEvent);
			::SetEvent(hMutex);
		}
		else
		{
		}
	}

	void releaseLockExclusive()
	{
		::SetEvent(hMutex);
#ifdef _HAVE_WRITER_CRITICAL_SECRION_LOCK_
		::LeaveCriticalSection(&m_cs);
#else
		::ReleaseMutex(hWriterMutex);
#endif
	}

	Ruediger_Asche_RWLock()
	{
		hReaderEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
		hMutex = CreateEvent(NULL,FALSE,TRUE,NULL);
#ifdef _HAVE_WRITER_CRITICAL_SECRION_LOCK_
		::InitializeCriticalSection(&m_cs);
#else
		hWriterMutex = CreateMutex(NULL,FALSE,NULL);
#endif
	  iCounter = -1;
	}
	
	~Ruediger_Asche_RWLock()
	{
		::CloseHandle(hReaderEvent);
		::CloseHandle(hMutex);
#ifdef _HAVE_WRITER_CRITICAL_SECRION_LOCK_
		::DeleteCriticalSection(&m_cs);
#else
		::CloseHandle(hWriterMutex);
#endif
	}
private:
	HANDLE hReaderEvent;
	HANDLE hMutex;
#ifdef _HAVE_WRITER_CRITICAL_SECRION_LOCK_
	CRITICAL_SECTION m_cs;
#else
	HANDLE hWriterMutex;
#endif
	LONG iCounter;
};
