#include "stdafx.h"
#include "SystemError.h"
#include "RWLockFavorWriters.h"

RWLockFavorWriters::RWLockFavorWriters()
    : numWritersWaiting_(0)
    , numReadersWaiting_(0)
    , activeWriterReaders_(0)
#if defined(TRACK_READER_RACES)

	, numReaderRacesLost_(0)
	, numReaderWakeups_(0)

#endif

{
    InitializeCriticalSection(&cs_);

    hReadyToRead_ = CreateEvent(NULL, 
                                TRUE, 
                                FALSE, 
                                NULL);

    _ASSERTE(hReadyToRead_ != NULL);

	if (hReadyToRead_ == NULL)
	{
		throw SystemError(GetLastError());
	}

    hReadyToWrite_ = CreateSemaphore(NULL, 
                                     0, 
                                     1, 
                                     NULL);

    _ASSERTE(hReadyToWrite_ != NULL);

	if (hReadyToWrite_ == NULL)
	{
		DWORD lastError = GetLastError();

		CloseHandle(hReadyToRead_);

		throw SystemError(lastError);
	}
}

RWLockFavorWriters::~RWLockFavorWriters()
{
	if (hReadyToRead_ != NULL)
		CloseHandle(hReadyToRead_);

	if (hReadyToWrite_ != NULL)
		CloseHandle(hReadyToWrite_);

	DeleteCriticalSection(&cs_);
}

void RWLockFavorWriters::acquireLockShared()
{
    bool fNotifyReaders = false;

    EnterCriticalSection(&cs_);

	// Block readers from acquiring the lock if 
	// there are any writers waiting or if a writer
	// has already acquired the lock.

    if ((numWritersWaiting_ > 0) || (HIWORD(activeWriterReaders_) > 0))
    {
        ++numReadersWaiting_;

        _ASSERTE(numReadersWaiting_ > 0);

        while (true)
        {
            ResetEvent(hReadyToRead_);

            LeaveCriticalSection(&cs_);

            WaitForSingleObject(hReadyToRead_, INFINITE);

            EnterCriticalSection(&cs_);

#if defined(TRACK_READER_RACES)

			++numReaderWakeups_;

#endif

			// The reader is only allowed to read if there aren't
			// any writers waiting and if a writer doesn't own the
			// lock.
			if ((numWritersWaiting_ == 0) && (HIWORD(activeWriterReaders_) == 0))
            {
                break;
            }

#if defined(TRACK_READER_RACES)

			++numReaderRacesLost_;

#endif
        }

		// Reader is done waiting.
        --numReadersWaiting_;

		_ASSERTE(numReadersWaiting_ >= 0);

		// Reader can read.
	    ++activeWriterReaders_;
    }
    else
    {
		// Reader can read.
		if ((++activeWriterReaders_ == 1) && (numReadersWaiting_ != 0))
		{
			// Set flag to notify other waiting readers
			// outside of the critical section
			// so that they don't when the threads
			// are dispatched by the scheduler they
			// don't immediately block on the critical
			// section that this thread is holding.
			fNotifyReaders = true;
		}
    }

    _ASSERTE(HIWORD(activeWriterReaders_) == 0);

    LeaveCriticalSection(&cs_);

    if (fNotifyReaders)
    {
        SetEvent(hReadyToRead_);
    }
}

void RWLockFavorWriters::acquireLockExclusive()
{
    EnterCriticalSection(&cs_);

	// Are there active readers?
    if (activeWriterReaders_ != 0)
    {
        ++numWritersWaiting_;

        _ASSERTE(numWritersWaiting_ > 0);

        LeaveCriticalSection(&cs_);

        WaitForSingleObject(hReadyToWrite_, INFINITE);

		// Upon wakeup theirs no need for the writer
		// to acquire the critical section.  It 
		// already has been transfered ownership of the
		// lock by the signaler.
    }
	else
	{
		_ASSERTE(activeWriterReaders_ == 0);

		// Set that the writer owns the lock.
		activeWriterReaders_ = MAKELONG(0, 1);;

		LeaveCriticalSection(&cs_);
	}
}

void RWLockFavorWriters::releaseLockShared()
{
    EnterCriticalSection(&cs_);

	// Assert that the lock isn't held by a writer.
	_ASSERTE(HIWORD(activeWriterReaders_) == 0);

	// Assert that the lock is held by readers.
	_ASSERTE(LOWORD(activeWriterReaders_ > 0));

	// Decrement the number of active readers.
    if (--activeWriterReaders_ == 0)
	{
		ResetEvent(hReadyToRead_);
	}

	// if writers are waiting and this is the last reader
	// hand owneership over to a writer.
    if ((numWritersWaiting_ != 0) && (activeWriterReaders_ == 0))
    {
		// Decrement the number of waiting writers
		--numWritersWaiting_;

		// Pass ownership to a writer thread.
		activeWriterReaders_ = MAKELONG(0, 1);
		ReleaseSemaphore(hReadyToWrite_, 1, NULL);
    }

    LeaveCriticalSection(&cs_);
}

void RWLockFavorWriters::releaseLockExclusive()
{
    bool fNotifyWriter = false;
    bool fNotifyReaders = false;

    EnterCriticalSection(&cs_);

	// Assert that the lock is owned by a writer.
	_ASSERTE(HIWORD(activeWriterReaders_) == 1);

	// Assert that the lock isn't owned by one or more readers
	_ASSERTE(LOWORD(activeWriterReaders_) == 0);

    if (numWritersWaiting_ != 0)
    {
		// Writers waiting, decrement the number of
		// waiting writers and release the semaphore
		// which means ownership is passed to the thread
		// that has been released.
		--numWritersWaiting_;
        fNotifyWriter = true;
    }
    else
    {
		// There aren't any writers waiting
		// Release the exclusive hold on the lock.
		activeWriterReaders_ = 0;

        // if readers are waiting set the flag
		// that will cause the readers to be notified
		// once the critical section is released.  This
		// is done so that an awakened reader won't immediately
		// block on the critical section which is still being
		// held by this thread.
        if (numReadersWaiting_ != 0)
        {
            fNotifyReaders = true;
        }
    }

    LeaveCriticalSection(&cs_);

    if (fNotifyWriter)
    {
        ReleaseSemaphore(hReadyToWrite_, 1, NULL);
    }
    else if (fNotifyReaders)
    {
        SetEvent(hReadyToRead_);
    }
}

