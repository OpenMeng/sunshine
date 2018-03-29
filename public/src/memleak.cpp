#include "memleak.h"

#ifdef MEM_DEBUG

void* 
operator new( size_t nSize, char* pszFileName, int nLineNum )
{
	void *pResult;	
	pResult = ::operator new( nSize );
	
	if ( pResult )
	{
	  printf("%s:%d:new 0x%x:%d\n",pszFileName,nLineNum,pResult,nSize);
	}
	
	return pResult;
}

void* 
operator new[]( size_t nSize, char* pszFileName, int nLineNum )
{
	void *pResult;
	
	pResult = ::operator new[]( nSize );
	
	if ( pResult )
	{
	  printf("%s:%d:new[]  0x%x:%d\n",pszFileName,nLineNum,pResult,nSize);
	}

	return pResult;
}


#ifdef HPUX
void  operator delete( void *ptr ) __THROWSPEC_NULL
#else
void  operator delete( void *ptr ) throw ()
#endif
{
	printf("delete 0x%x\n",ptr);
	free( ptr );
	ptr = 0;
}

#ifdef HPUX
void  operator delete[]( void *ptr ) __THROWSPEC_NULL
#else
void  operator delete[]( void *ptr) throw ()
#endif
{
	printf("delete[] 0x%x\n",ptr);
	free( ptr );
	ptr = 0;
}

#endif

