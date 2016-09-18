#pragma once
 
#include <windows.h>


// Here's the pseudocode for what is going on:
//
// Lock for Reader:
// Lock the mutex
// Bump the count of readers
// If this is the first reader, lock the data
// Release the mutex
//
// Unlock for Reader:
// Lock the mutex
// Decrement the count of readers
// If this is the last reader, unlock the data
// Release the mutex
//
// Lock for Writer:
// Lock the data
//
// Unlock for Reader:
// Unlock the data

///////////////////////////////////////////////////////


class Jim_B_Robert_Wiener_RWLock
{
public:
	void acquireLockShared()
	{
		if (!MyWaitForSingleObject(hMutex))
		{
			FatalError("Fatal Error - acquireLockShared!");
			return;
		}
		else
		{
		}

		if(++nReaderCount == 1)
		{
			if(!MyWaitForSingleObject(hDataLock))
			{
				FatalError("Fatal Error - acquireLockShared!");
			}
			else
			{
			}

		}
		else
		{
		}
		
		::ReleaseMutex(hMutex);
	}

	void acquireLockExclusive()
	{
		if(!MyWaitForSingleObject(hDataLock))
		{
			FatalError("Fatal Error - acquireLockExclusive!");
		}
		else
		{
		}

	}

	void releaseLockShared()
	{
		LONG lPrevCount;

		if (!MyWaitForSingleObject(hMutex))
		{
			FatalError("Fatal Error - releaseLockShared!");
			return; 
		}
		else
		{
		}

		if (--nReaderCount == 0)
		{
			::ReleaseSemaphore(hDataLock, 1, &lPrevCount);
		}
		else
		{
		}

		::ReleaseMutex(hMutex);
	}

	void releaseLockExclusive()
	{
		LONG lPrevCount;

		::ReleaseSemaphore(hDataLock, 1, &lPrevCount);
		if (lPrevCount != 0)
		{
			FatalError("ReleaseWriteLock — Semaphore was not locked!");
		}
		else
		{
		}
	}

	Jim_B_Robert_Wiener_RWLock()
	{
		nReaderCount = 0;
		hDataLock = ::CreateSemaphore(NULL, 1, 1, NULL);
		hMutex = ::CreateMutex(NULL, FALSE, NULL);
	}
	
	~Jim_B_Robert_Wiener_RWLock()
	{
		DWORD result = ::WaitForSingleObject(hDataLock, 0);
		if (result == WAIT_TIMEOUT)
		{
			FatalError("~Jim_B_Robert_Wiener_RWLock — Can't destroy object, it's locked!");
		}
		else
		{
		}
		::CloseHandle(hMutex);
		::CloseHandle(hDataLock);
	}

private:

	// If we wait more than 2 seconds, then something is probably wrong!
	enum {MAXIMUM_TIMEOUT = 2000};

	BOOL MyWaitForSingleObject(HANDLE hObject)
	{
		DWORD result;

		result = WaitForSingleObject(hObject, MAXIMUM_TIMEOUT);
		// Comment this out if you want this to be non-fatal
		if (result != WAIT_OBJECT_0)
		FatalError("MyWaitForSingleObject — Wait failed, you probably forgot to call release!");
		return (result == WAIT_OBJECT_0);
	}

	/*
	* Error handler
	*/
	BOOL FatalError(char const *s)
	{
	fprintf(stdout, "%s\n", s);
	// Comment out exit() to prevent termination
	exit(EXIT_FAILURE);
	return FALSE;
	}

	// Handle to a mutex that allows
	// a single reader at a time access
	// to the reader counter.
	HANDLE hMutex;

	// Handle to a semaphore that keeps
	// the data locked for either the
	// readers or the writers.
	HANDLE hDataLock;

	// The count of the number of readers.
	// Can legally be zero or one while
	// a writer has the data locked.
	int nReaderCount;
};
