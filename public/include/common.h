#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <cmath>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libgen.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <regex.h>
#include <dirent.h>
#include <algorithm>
#include <functional>
#include <netdb.h>
#include <fcntl.h>
#include <stdarg.h>
#ifdef HPUX
#include <sys/param.h>
#include <sys/pstat.h>
#endif
#ifdef TRUE64
#include <sys/procfs.h>
#endif
#include "database.h"
#include "user_exception.h"
#include "log_file.h"
#include "user_assert.h"
#include "macro.h"

namespace util
{

using namespace std;

//
enum round_mode {
	FLOOR = '0', 
	ROUND,
	CEIL 
};

struct param {
	string user_name;
	string password;
	string connect_string;
	int port;
	key_t ipc_key;
	string log_path;
	log_file::switch_mode mode;
	string host_id;
	string module_id;
	string proc_id;
	string thread_id;
	database *db;
	//
	char* sys_param;

	//save command line parameters
	map<string, string> *args;

};

class common {
public:
	// Encrypt the first parameter to the second.
	static void encrypt(const char* text, char* cipher);
	// Decrypt the first parameter to the second.
	static void decrypt(const char* cipher, char* text);
	// Put string devided by a charactor into array.
	static void string_to_array(const string& the_str, char delimiter, vector<string>& the_vector, bool del_space=true);
	// Read database connection infomations from file .passwd.
	static void read_db_info(string& user_name, string& password, string& connect_string);
	// Write database connection infomations to file .passwd.
	static void write_db_info(const string& user_name, const string& password, const string& connect_string);
	// Read ini value.
	static bool read_profile(const string& file_name, const string& section, const string& key, string& value);
	// Trim left charactors in set identified by the second parameter.
	// If a charactor is found not in the set, then will return.
	static void ltrim(char* the_str, const char* fill);
	// Trim left charactors equal to the second parameter.
	// If a charactor is found not in the set, then will return.
	static void ltrim(char* the_str, char ch);
	// Trim right charactors in set identified by the second parameter.
	// If a charactor is found not in the set, then will return.
	static void rtrim(char* the_str, const char* fill);
	// Trim right charactors equal to the second parameter.
	// If a charactor is found not in the set, then will return.
	static void rtrim(char* the_str, char ch);
	// Trim both sides' charactors in set identified by the second parameter.
	// If a charactor is found not in the set, then will return.
	static void trim(char* the_str, const char* fill);
	// Trim both sides' charactors equal to the second parameter.
	// If a charactor is found not in the set, then will return.
	static void trim(char* the_str, char ch);
	// convert the string to upper. It modifies the string's value and return
	// the first address of the string.
	static void upper(char* the_str);
	static void upper(std::string& the_str);
	// convert the string to lower. It modifies the string's value and return
	// the first address of the string.
	static void lower(char* the_str);
	static void lower(std::string& the_str);
	//read the data from the buffer according to the fmt string
	static int super_sscanf(const char *buf,const char *fmt,...);
	// Get file sn(range from 0) from database.
//	static unsigned int get_file_sn(database& db, const string& sn_name = "file_sequence");
	// Get database password according to user_name and connect_string.
	static bool get_passwd(database& db, const string& user_name, const string& connect_string, string& passwd);
	static void load_param(param& para);
	// Round double value.
	static void round(int mode, int precision, double& value);
	// Get ip address.
	static void get_ip(string& ip);
	// Rename file between any file system.
	static bool rename(const char* src, const char* dst);
	// Check program status.
	static bool is_prog_run();
	//Get the greatest common divisor(GCD) of two positive integers.
	static int grt_com_div(int a ,int b);
	//Get the GCD of several positive integers.
	static int grt_com_div(vector<int>& numbers);
	//Get the lease common multiple(LCM) of two positive intergers.
	static int ls_com_mul(int a, int b);
	//Get the LCM of several positive intergers.
	static int ls_com_mul(vector<int>& numbers);

	//if list is null : replace environment variables of linux OS in the src_str,
	//    and output the result of dst_str.
	//if list is not null: replace variables in the list
	static void expand_var(string& dst_str, const string& src_str, map<string, string> list /*= map<string, string>()*/);

};

// Base class of date, ptime and datetime.
class calendar {
public:
	// Get time zone and time in seconds since the Epoch.
	explicit calendar(time_t dd = 0);
	// Set time in seconds since the Epoch.
	calendar& operator=(time_t dd);
	// Add time in seconds since the Epoch.
	calendar& operator+=(const calendar& rhs);
	calendar& operator+=(time_t dd);
	// Subtract time in seconds since the Epoch.
	calendar& operator-=(const calendar& rhs);
	calendar& operator-=(time_t dd);
	// Subtract time in seconds since the Epoch, and return the interval between.
	time_t operator-(const calendar& rhs) const;
	// Compare two calendars.
	bool operator>(const calendar& rhs) const;
	bool operator>=(const calendar& rhs) const;
	bool operator<(const calendar& rhs) const;
	bool operator<=(const calendar& rhs) const;
	bool operator==(const calendar& rhs) const;
	// Return time in seconds since the Epoch.
	time_t duration() const;
	// Get the last day of this month.
	int last_day() const;

protected:
	// Check whether it's a correct date.
	static void check_date(int y, int m, int d);
	// Check whether it's a correct time.
	static void check_time(int h, int m, int s);

	// Time in seconds since the Epoch.
	time_t duration_;
	// Time in seconds of time zone, it's a positive value.
	time_t time_zone;
};

class date : public calendar {
public:
	// Generate date object according to year,month,and day.
	date(int y, int m, int d);
	// Generate date object according to a string.
	// It can have any format below:
	// YYYYMMDD
	// YYYY:MM:DD
	// YYYY-MM-DD
	date(const string& ds);
	// Generate date object according to another date object.
	date(const date& d);
	// Generate date object according to time in seconds since the Epoch.
	explicit date(time_t dd = 0);
	// Set date to the provided date object value.
	date& operator=(const date& d);
	// Set date to the provided time in seconds since the Epoch.
	date& operator=(time_t dd);
	// Return year of local time.
	int year() const;
	// Return month of local time.
	int month() const;
	// Return day of local time.
	int day() const;
	// Return week day of local time, Monday - Sunday : 1 - 7.
	int week() const;
	// Set date to next (n) months, and pointer to the first day of that month.
	void next_month(int n = 1);
	// Set date to next (n) month.
	void add_months(int n);
	// Get months between two date.
	double months_between(const date& rhs);
	// Output string of format YYYYMMDD.
	void iso_string(string& ds) const;
	// Output string of format YYYY-MM-DD.
	void iso_extended_string(string& ds) const;
};

class ptime : public calendar {
public:
	// Generate ptime object according to hour,minute,and second.
	ptime(int h, int m, int s);
	// Generate ptime object according to a string.
	// It can have any format below:
	// HHMISS
	// HH:MI:SS
	// HH-MI-SS
	ptime(const string& ts);
	// Generate ptime object according to another ptime object.
	ptime(const ptime& t);
	// Generate ptime object according to time in seconds since "00:00:00".
	explicit ptime(time_t dd = 0);
	// Set ptime to the provided ptime object value.
	ptime& operator=(const ptime& t);
	// Set ptime to the provided time in seconds since "00:00:00".
	ptime& operator=(time_t dd);
	// Return hour of local time.
	int hour() const;
	// Return minute of local time.
	int minute() const;
	// Return second of local time.
	int second() const;
	// Output string of format HHMISS.
	void iso_string(string& ds) const;
	// Output string of format HH-MI-SS.
	void iso_extended_string(string& ds) const;
};

class datetime : public calendar {
public:
	// Generate datetime object according to year,month,day,hour,minute,second.
	datetime(int y, int m, int d, int h, int mi, int s);
	// Generate datetime object according to a string.
	// It can have any format below:
	// YYYYMMDDHH24MISS
	// YYYY:MM:DD:HH:MI:SS
	// YYYY-MM-DD-HH-MI-SS
	datetime(const string& dts);
	datetime(const string& dts,const string& src_full_name);
	// Generate datetime object according to another datetime object.
	datetime(const datetime& dt);
	// Generate datetime object according to time in seconds since the Epoch.
	explicit datetime(time_t dd = 0);
	// Set datetime to the provided datetime object value.
	datetime& operator=(const datetime& dt);
	// Generate datetime object according to time in seconds since the Epoch.
	datetime& operator=(time_t dd);
	datetime& operator=(const string& dts);
	datetime& operator=(const char* pdatestr);
	datetime& assign(const string& dts, const char* format);
	// Return datetime of current datetime after seconds provided.
	datetime operator+(time_t dd) const;
	// Return datetime of current datetime before seconds provided.
	datetime operator-(time_t dd) const;
	// Return year of local time.
	int year() const;
	// Return month of local time.
	int month() const;
	// Return day of local time.
	int day() const;
	// Return hour of local time.
	int hour() const;
	// Return minute of local time.
	int minute() const;
	// Return second of local time.
	int second() const;
	// Return week day of local time, Monday - Sunday : 1 - 7.
	int week() const;
	// Return date of local time.
	time_t date() const;
	bool is_leap() const;
	// Return time of local time.
	time_t time() const;
	// Set datetime to next month.
	void next_month(int n = 1);
	void next_day(int n = 1);
	void next_hour(int n = 1);
	void add_months(int n);
	void last_day_end();
	void subtract_months(int n);
	// Return hours array between current datetime and datetime after seconds provided.
	void hours_cover(time_t dd, vector<int>& hours) const;
	// Return days array between current datetime and datetime after seconds proviced.
	void days_cover(time_t dd, vector<int>& days) const;
	// Return months array between current datetime and datetime after seconds proviced.
	void month_cover(time_t dd, vector<int>& months) const;
	// Output string of format YYYYMMDD24HHMISS
	void iso_string(string& dts) const;
	// Output string of format YYYY-MM-DD-HH-MI-SS
	void iso_duration(time_t& dts) const;
	// Output seconds from 19700101
	void iso_extended_string(string& dts) const;
};

inline otl_stream &operator>>(otl_stream &s,datetime &v){
    otl_datetime odt;
    s>>odt;
    v=datetime(odt.year,odt.month,odt.day,odt.hour,odt.minute,odt.second);
    return s;
}

inline otl_stream &operator<<(otl_stream &s,const datetime &v){
    otl_datetime odt;
    odt.year=v.year();
    odt.month=v.month();
    odt.day=v.day();
    odt.hour=v.hour();
    odt.minute=v.minute();
    odt.second=v.second();
    s<<odt;
    return s;
}

}

#endif

