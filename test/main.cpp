#include "../src/stdafx.h" 
#include <windows.h>

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include "../src/perf_counter.h"
#include "../src/rwlock.h"
#include "../src/RWCriticalSection.h"
#include "../src/RWLockFavorNeither.h"
#include "../src/RWLockFavorWriters.h"
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
#   include "../src/SlimRWLock.h"
#endif
#include "../src/Ruediger_Asche_RWLock.h"
#include "../src/Jim_B_Robert_Wiener_RWLock.h"

#include <typeinfo>
#ifdef HAVE_GCC_ABI_DEMANGLE
#   include <cxxabi.h>
#   include <cstdlib>
#endif

std::string demangle_name(char const* const mangled_name) {
#ifdef HAVE_GCC_ABI_DEMANGLE
    int status = 0;
    char *demangled = NULL;
    std::string name;
    demangled = abi::__cxa_demangle(mangled_name, 0, 0, &status);
    name.assign(demangled);
    std::free(demangled);
    return name;
#else
    return mangled_name;
#endif
}

HANDLE g_start_event;
volatile long g_stop_test;

const int writer_test_iterations = 1000000;
const int number_of_writers = 1;
const int reader_test_iterations = 4000000;
const int number_of_readers = 4;

template <typename lock_type> 
class test_value_t {
public:
	typedef ref_exclusive_lock_t<lock_type> exclusive_lock;
	typedef ref_shared_lock_t<lock_type> shared_lock;

	bool read() {
		shared_lock lock(_M_lock);
		return (_M_checksum == checksum(_M_buffer, sizeof(_M_buffer)));
	}

	bool write() {
		exclusive_lock lock(_M_lock);
		set_buffer();
		return true;
	}

	test_value_t() {
		set_buffer();
	}

    lock_type const& get_lock() const { return _M_lock; }
    lock_type& get_lock() { return _M_lock; }
private:
	USHORT checksum(LPVOID data, int len) {
        // The function checksum came from http://ava.org.ua/?2&17&get=87&read=87
		ULONG sum = 0;
		USHORT * dp =  (USHORT*) data;
		USHORT sum_s;
		int words = len >> 1;
		while(words--)
            sum += * dp ++;
		if(len & 1)
            sum += *(UCHAR*) dp;
		sum   = (USHORT) sum + ((sum >> 16) & 0xffff);
		sum_s = (USHORT) sum + (USHORT)(sum >> 16);
		return sum_s != 0xffff ? ~sum_s : sum_s;
	}

	void set_buffer() {
		for(size_t i = 0U; i < sizeof(_M_buffer) / sizeof(_M_buffer[0]); ++i)
			_M_buffer[i] = rand();
		_M_checksum = checksum(_M_buffer, sizeof(_M_buffer));
	}

	enum { BUF_SIZE = 64 };
	int _M_buffer[BUF_SIZE];
	USHORT _M_checksum;
	lock_type _M_lock;
};

//test_value_t <RWLockFavorWriters> g_value;
test_value_t <RWLockFavorNeither> g_value;
//test_value_t <RWCriticalSection> g_value;
//test_value_t <Jim_B_Robert_Wiener_RWLock> g_value;
//test_value_t <Ruediger_Asche_RWLock> g_value;

#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
    //test_value_t <SlimRWLock> g_value;
#endif


template <const int test_iteration>
class TestWorker {
public:
    virtual ~TestWorker() { }

	HANDLE start() {
		::ResumeThread(_M_handle);
		return _M_handle;
	}

	double test_duration() {
		return _M_perf_counter.get_duration();
	}

	LONGLONG test_iteration_done() {
		return _M_perf_counter.get_counter();
	}

private:
	static DWORD WINAPI worker_proc(LPVOID lpParam) {
		TestWorker& worker = *reinterpret_cast<TestWorker*>(lpParam);
		::WaitForSingleObject(g_start_event, INFINITE );

		worker._M_perf_counter.start();
        int i;
		for (i = 0; i < test_iteration; ++i) {
			if(!worker.simulate_work())
				::RaiseException( STATUS_NONCONTINUABLE_EXCEPTION, 0, 0, 0);
			if (TRUE == g_stop_test)
				break;
		}
		
		worker._M_perf_counter.end();
		::InterlockedExchange(&g_stop_test, TRUE);
		worker._M_perf_counter.set_iteration_done(i);
		return 0;
	}

	virtual bool simulate_work() = 0;

	HANDLE _M_handle;
	PerformanceCounterMeter _M_perf_counter;
	TestWorker& operator = (TestWorker const&) {}
protected:
	TestWorker() {
		_M_handle = ::CreateThread(NULL, 0, worker_proc, (LPVOID)this, CREATE_SUSPENDED, NULL);
	}
};

class Writer: public TestWorker <writer_test_iterations> {
	virtual bool simulate_work() {
		return g_value.write();
	}
};

class Reader: public TestWorker <reader_test_iterations> {
	virtual bool simulate_work() {
		return g_value.read();
	}
};

int _tmain(int argc, _TCHAR* argv[]) {
	g_start_event = ::CreateEvent(NULL,TRUE,FALSE,NULL);

	HANDLE threads[number_of_writers + number_of_readers];

	Writer* writer[number_of_writers];
	for (int i = 0; i < number_of_writers; i++)
	{
		writer[i] = new Writer();
		threads[i] = writer[i]->start();
	}

	Reader* reader[number_of_readers];
	for (int i = 0; i < number_of_readers; i++)
	{
		reader[i] = new Reader();
		threads[i + number_of_writers] = reader[i]->start();
	}

	::SetEvent(g_start_event);
	::WaitForMultipleObjects(sizeof(threads) / sizeof(threads[0]), threads, TRUE, INFINITE);
    std::printf("type of g_value.get_lock() is %s\n", demangle_name(typeid(g_value.get_lock()).name()).c_str());

	for (int i = 0; i < number_of_readers; i++)
	{
        std::printf("reader iterations done | per operation(ms),%10ld,%17f\n",
			static_cast<long>(reader[i]->test_iteration_done()),
			1000 * reader[i]->test_duration() / reader[i]->test_iteration_done());

	}

	for (int i = 0; i < number_of_writers; i++)
	{
        std::printf("writer iterations done | per operation(ms),%10ld,%17f\n",
			static_cast<long>(writer[i]->test_iteration_done()),
			1000 * writer[i]->test_duration() / writer[i]->test_iteration_done());
	}
    for (int i = 0; i < number_of_readers; ++i) {
        delete reader[i];
        reader[i] = NULL;
    }
    for (int i = 0; i < number_of_writers; ++i) {
        delete writer[i];
        writer[i] = NULL;
    }
    for (int i = 0, n = number_of_readers + number_of_writers; i < n; ++i) {
        if (threads[i] != INVALID_HANDLE_VALUE)
            ::CloseHandle(threads[i]);
        threads[i] = INVALID_HANDLE_VALUE;
    }
    if (g_start_event == INVALID_HANDLE_VALUE) {
        if (g_start_event != INVALID_HANDLE_VALUE)
            ::CloseHandle(g_start_event);
        g_start_event = INVALID_HANDLE_VALUE;
    }
	return EXIT_SUCCESS;
}
