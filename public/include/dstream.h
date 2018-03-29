#if !defined(__DSTREAM_H__)
#define __DSTREAM_H__

#include <iostream>
#include <iomanip>
#include <string>

namespace util
{

using namespace std;


#if defined(DEBUG)

#define dout cout

#else



class dstream
{
public:
	dstream& operator<<(char c) { return *this; }
	dstream& operator<<(signed char c) { return *this; }
	dstream& operator<<(unsigned char c) { return *this; }
	dstream& operator<<(wchar_t c) { return *this; }
	dstream& operator<<(const wchar_t* c) { return *this; }
	dstream& operator<<(const char* c) { return *this; }
	dstream& operator<<(const signed char* c) { return *this; }
	dstream& operator<<(int c) { return *this; }
	dstream& operator<<(long c) { return *this; }
#if defined(_LONG_LONG)
	dstream& operator<<(long long c) { return *this; }
#endif
	dstream& operator<<(double c) { return *this; }
	dstream& operator<<(long double c) { return *this; }
	dstream& operator<<(float c) { return *this; }
	dstream& operator<<(unsigned int c) { return *this; }
	dstream& operator<<(unsigned long c) { return *this; }
#if defined(_LONG_LONG)
	dstream& operator<<(unsigned long long c) { return *this; }
#endif
	dstream& operator<<(const void* c) { return *this; }
	dstream& operator<<(streambuf* c) { return *this; }
	dstream& operator<<(ostream& (*f)(ostream&)) { return *this; }
	dstream& operator<<(ios& (*f)(ios&)) { return *this; }
	dstream& operator<<(short c) { return *this; }
	dstream& operator<<(unsigned short c) { return *this; }
	dstream& operator<<(const string& s) { return *this; }
};

#if !defined(__linux__)

template<class _Tm>
#if defined(__hpux)
dstream& operator<<(dstream& _O, const smanip<_Tm>& _M)
#else
dstream& operator<<(dstream& _O, const _Smanip<_Tm>& _M)
#endif
{
	return (_O);
}

#else

extern dstream& operator<<(dstream& _O, _Setfill<char> _M);
extern dstream& operator<<(dstream& _O, _Setiosflags _M);
extern dstream& operator<<(dstream& _O, _Resetiosflags _M);
extern dstream& operator<<(dstream& _O, _Setbase _M);
extern dstream& operator<<(dstream& _O, _Setprecision _M);
extern dstream& operator<<(dstream& _O, _Setw _M);

#endif

extern dstream dout;

#endif

}

#endif

