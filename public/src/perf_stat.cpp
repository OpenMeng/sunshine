#include <iostream>
#include "perf_stat.h"

namespace util
{

using namespace std;

#if defined(PERF_STAT)
map<string, stat_t> perf_stat::stat_map;
map<string, int> perf_stat::count_map;
#endif

perf_stat::perf_stat(const string& perf_name_, int print_batch_)
{
#if defined(PERF_STAT)
	perf_name = perf_name_;
	print_batch = print_batch_;
	count_map[perf_name]++;
	begin_clock = clock();
	gettimeofday(&begin_time, NULL);
#endif
}

perf_stat::~perf_stat()
{
#if defined(PERF_STAT)
	clock_t end_clock = clock();
	timeval end_time;
	gettimeofday(&end_time, NULL);
	stat_t& value = stat_map[perf_name];
	value.clocks += end_clock - begin_clock;
	value.tv_sec += end_time.tv_sec - begin_time.tv_sec;
	value.tv_usec += end_time.tv_usec - begin_time.tv_usec;
	if (value.tv_usec < 0) {
		value.tv_sec--;
		value.tv_usec += 1000000;
	} else {
		value.tv_sec += value.tv_usec / 1000000;
		value.tv_usec = value.tv_usec % 1000000;
	}

	int count = count_map[perf_name];
	if (count % print_batch == 0) {

		cout << "perf_name = [" << perf_name << "] loop count = [" << count
			<< "] clock seconds elapsed = [" << (static_cast<double>(value.clocks) / CLOCKS_PER_SEC)
			<< "s] seconds elapsed = [" << value.tv_sec << "s " << value.tv_usec << "us" << "]"
			<< endl;
	}
#endif
}

}

