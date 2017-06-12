
#include "bfc/mem.h"

#include <stdlib.h>
//#include <crtdbg.h>
#include <malloc.h>
#include <exception>
#include <new>
//#include <windows.h>
#include <assert.h>
#include <cstring>
#include <wchar.h>
/*
_FFS_API void* m__cdecl operator new(size_t size, const char* file, int line,_ff_debug_new *)
{
	void *pbuf=_malloc_dbg(size,_NORMAL_BLOCK,file,line);
	if(!pbuf)
		throw std::bad_alloc();

	return pbuf;
}

_FFS_API void* m__cdecl operator new[](size_t size, const char* file, int line,_ff_debug_new *)
{
	void *pbuf=operator new(size,file,line,NULL);
	if(!pbuf)
		throw std::bad_alloc();

	return pbuf;
}

_FFS_API void m__cdecl operator delete(void* p, const char *file, int line,_ff_debug_new *)
{
	_free_dbg(p,_NORMAL_BLOCK);
}

_FFS_API void m__cdecl operator delete[](void* p, const char *file, int line,_ff_debug_new *)
{
	operator delete(p,file,line,NULL);
}
*/

#include "bfc/cfg.h"


_FF_BEG

_FFS_API void* ff_alloc(size_t size)
{
	return new char[size];
}

_FFS_API void  ff_free(void *ptr)
{
	delete[](char*)ptr;
}

static void **g_gm_list=NULL;
static int    g_gm_size=0, g_gm_count=0, g_gm_free=0;

static int _find_gm(void *pm)
{
	for(int i=0; i<g_gm_count; ++i)
	{
		if(g_gm_list[i]==pm)
			return i;
	}
	return -1;
}

_FFS_API void* ff_galloc(size_t size)
{
	void *gm=ff_alloc(size);

	if(g_gm_free==0)
	{
		if(g_gm_count>=g_gm_size)
		{
			void **gm_list=new void*[g_gm_size+100];

			memcpy(gm_list,g_gm_list,sizeof(void*)*g_gm_count);
			g_gm_size+=100;

			delete[]g_gm_list;
			g_gm_list=gm_list;
		}

		g_gm_list[g_gm_count]=gm;
		++g_gm_count;
	}
	else
	{
		int im=_find_gm(NULL);

		assert(im>=0);
		g_gm_list[im]=gm;
		--g_gm_free;
	}

	return gm;
}

_FFS_API void  ff_gfree(void *ptr)
{
	if(ptr)
	{
		int im=_find_gm(ptr);

		if(im>=0)
		{
			g_gm_list[im]=NULL;
			++g_gm_free;
		}

		ff_free(ptr);
	}
}

static void  _gfree_memory()
{
	for(int i=0; i<g_gm_count; ++i)
	{
		ff_free(g_gm_list[i]);
	}

	delete[]g_gm_list;
	g_gm_list=NULL;

	g_gm_count=g_gm_size=g_gm_free=0;
}

static ff_gfree_func_t *g_gf_list=NULL;
static int g_gf_size=0, g_gf_count=0;

_FFS_API void  ff_add_gfree_func(ff_gfree_func_t fp)
{
	if(fp)
	{
		if(g_gf_count>=g_gf_size)
		{
			ff_gfree_func_t *gf_list=new ff_gfree_func_t[g_gf_size+100];

			if(g_gf_list)
			{
				memcpy(gf_list,g_gf_list,sizeof(ff_gfree_func_t)*g_gf_count);
				delete[]g_gf_list;
			}

			g_gf_list=gf_list;
			g_gf_size+=100;
		}

		g_gf_list[g_gf_count]=fp;
		++g_gf_count;
	}
}

static void _gfree_func()
{
	for(int i=0; i<g_gf_count; ++i)
	{
		if(g_gf_list[i])
		{
			try
			{
				(g_gf_list[i])();
			}
			catch(...)
			{
			}
		}
	}

	delete[]g_gf_list;
	g_gf_list=NULL;

	g_gf_count=g_gm_size=0;
}

_FFS_API void ff_gfree_all()
{
	_gfree_memory();
	_gfree_func();
}


_FFS_API void enable_memory_leak_report(bool enable)
{
#ifdef _DEBUG

	int flag=_CrtSetDbgFlag(0);

	if(enable)
		_CrtSetDbgFlag(flag|_CRTDBG_LEAK_CHECK_DF);
	else
		_CrtSetDbgFlag(flag&~_CRTDBG_LEAK_CHECK_DF);

#endif
}


ff_mem::ff_mem()
:m_mem(NULL)
{
}

void ff_mem::set_mem(void *mem)
{
	if(m_mem)
		ff_free(m_mem);

	m_mem=mem;
}

ff_mem::~ff_mem()
{
	ff_free(m_mem);
}

_FFS_API char* ff_w2a(const wchar_t *wcs, ff_mem &mem)
{
	char *ds=NULL;
/*
	if(wcs)
	{
		int len=(int)wcslen(wcs)+1;

		ds=(char*)ff_alloc((len+1)*2);
		int dsz=WideCharToMultiByte(CP_THREAD_ACP,WC_NO_BEST_FIT_CHARS,wcs,len,ds,(len+1)*2,NULL,NULL);

		if(dsz==0)
		{
			ff_free(ds);
			ds=NULL;
		}
	}
*/
	mem.set_mem(ds);

	return ds;
}

_FFS_API wchar_t* ff_a2w(const char *acs, ff_mem &mem)
{
	wchar_t *ds=NULL;
/*
	if(acs)
	{
		int len=(int)strlen(acs)+1;

		ds=(wchar_t*)ff_alloc((len+1)*2);
		int dsz=MultiByteToWideChar(CP_THREAD_ACP,0,acs,len,ds,(len+1)*2);

		if(dsz==0)
		{
			ff_free(ds);
			ds=NULL;
		}
	}
*/
	mem.set_mem(ds);

	return ds;
}

_FF_END
