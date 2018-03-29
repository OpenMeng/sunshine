#if !defined(__PERF_STAT_H__)
#define __PERF_STAT_H__

#include <map>
#include <string>

namespace util
{

using namespace std;

struct stat_t
{
	clock_t clocks;
	time_t tv_sec;
	long tv_usec;

	stat_t()
	{
		clocks = 0;
		tv_sec = 0;
		tv_usec = 0;
	}
};

class perf_stat
{
public:
	perf_stat(const string& perf_name_, int print_batch_ = 1000);
	~perf_stat();

private:
#if defined(PERF_STAT)
	string perf_name;
	clock_t begin_clock;
	timeval begin_time;
	int print_batch;
	static map<string, stat_t> stat_map;
	static map<string, int> count_map;
#endif
};

}

#endif

