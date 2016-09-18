#pragma once
 
#ifndef _WINDOWS_

	#include <windows.h>

#endif

#include <stdexcept>

#define _ASSERTE(X) (void(0))

//
// Exception object that is thrown from the various classes below.
//
class SystemError : public std::runtime_error
{
public:
	SystemError(const DWORD info) 
	:   std::runtime_error(""),
	    info_(info)
	{
	}

	virtual ~SystemError() throw()
	{
	};

	SystemError(const SystemError& rExcept) 
	:  std::runtime_error(rExcept),
	   info_(rExcept.info_) 
	{
	}

	SystemError& operator=(const SystemError& rExcept) 
	{
		//		std::runtime_error::operator=(rExcept);
		static_cast<std::runtime_error&>(*this) = rExcept;
		info_ = rExcept.info_; 
		return *this; 
	}

	DWORD info() const
	{
		return info_;
	}

private:
	DWORD info_;
};
