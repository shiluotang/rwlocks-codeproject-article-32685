#pragma once

#include <windows.h>

class RWLockFavorNeither
{ 
public:
    RWLockFavorNeither();

	~RWLockFavorNeither();

	void acquireLockShared();

	void acquireLockExclusive();

	void releaseLockShared();

	void releaseLockExclusive();

	DWORD numberReaderRacesLost() const
	{
#if defined(TRACK_READER_RACES)

		return numReaderRacesLost_;

#else

		return 0;

#endif
	}

	DWORD numberReaderWakeups() const
	{
#if defined(TRACK_READER_RACES)

		return numReaderWakeups_;

#else
		return 0;

#endif
	}

private:
    LONG numWritersWaiting_;
    LONG numReadersWaiting_;

    // HIWORD is writer active flag;
    // LOWORD is readers active count;
    DWORD activeWriterReaders_;

    HANDLE hReadyToRead_;
    HANDLE hReadyToWrite_;
    CRITICAL_SECTION cs_;

	DWORD numReaderRacesLost_;

	DWORD numReaderWakeups_;
};

