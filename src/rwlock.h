#pragma once 

template <typename lock_t>
class ref_shared_lock_t
{
public:
	ref_shared_lock_t(lock_t& lock) : _M_lock(lock)
	{
		_M_lock.acquireLockShared();
	}
	~ref_shared_lock_t()
	{
		_M_lock.releaseLockShared();
	}
private:
	lock_t& _M_lock;
	ref_shared_lock_t& operator=(const ref_shared_lock_t&); //C4512 warning
};

template <typename lock_t>
class ref_exclusive_lock_t
{
public:
	ref_exclusive_lock_t(lock_t& lock) : _M_lock(lock)
	{
		_M_lock.acquireLockExclusive();
	}
	~ref_exclusive_lock_t()
	{
		_M_lock.releaseLockExclusive();
	}
private:
	lock_t& _M_lock;
	ref_exclusive_lock_t& operator=(const ref_exclusive_lock_t&); //C4512 warning
};
