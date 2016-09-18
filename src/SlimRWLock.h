#pragma once

class SlimRWLock
{
public:
	SlimRWLock()
	{
		::InitializeSRWLock(&srwLock_);
	}

	void acquireLockShared() 
	{
		::AcquireSRWLockShared(&srwLock_);
	}

	void acquireLockExclusive()
	{
		::AcquireSRWLockExclusive(&srwLock_);
	}

	void releaseLockShared()
	{
		::ReleaseSRWLockShared(&srwLock_);
	}

	void releaseLockExclusive()
	{
		::ReleaseSRWLockExclusive(&srwLock_);
	}

	DWORD numberReaderRacesLost() const
	{
		return 0;
	}

	DWORD numberReaderWakeups() const
	{
		return 0;
	}

	LONGLONG locked_acquiringLockExclusive(void) const
	{
		return 0L;
	}
	LONGLONG locked_acquiringLockShared(void) const
	{
		return 0L;
	}

private:
	SRWLOCK srwLock_;
};

