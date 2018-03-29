#include "dstream.h"

namespace util
{

#if defined(DEBUG)

#define dout cout

#else

#if defined(__linux__)

dstream& operator<<(dstream& _O, _Setfill<char> _M)
{
        return (_O);
}

dstream& operator<<(dstream& _O, _Setiosflags _M)
{
        return (_O);
}

dstream& operator<<(dstream& _O, _Resetiosflags _M)
{
        return (_O);
}

dstream& operator<<(dstream& _O, _Setbase _M)
{
        return (_O);
}

dstream& operator<<(dstream& _O, _Setprecision _M)
{
        return (_O);
}

dstream& operator<<(dstream& _O, _Setw _M)
{
        return (_O);
}

#endif

dstream dout;

#endif

}

