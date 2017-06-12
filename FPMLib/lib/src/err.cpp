

#include<cassert>
#include<string>

#include "bfc/err.h"
#include "bfc/mem.h"

#include "bfc/cfg.h"
#include <cstring>

_FF_BEG


CSException::CSException()
:m_err(NULL), m_msg(NULL)
{
}

static char* _str_clone(const char *str)
{
	char *ds=NULL;

	if(str)
	{
		size_t len=strlen(str);
		ds=new char[len+1];
		memcpy(ds,str,len+1);
	}

	return ds;
}

CSException::CSException(const char *err, const char *msg)
:m_err(NULL), m_msg(NULL)
{
	m_err=_str_clone(err);

	m_msg=_str_clone(msg);
}

CSException::~CSException()
{
	delete[]m_err;
	delete[]m_msg;
}

const char* CSException::GetError() const
{
	return m_err;
}

const char* CSException::GetMessage() const
{
	return m_msg;
}

const char* CSException::what() const noexcept(true)
{
	return m_err;
}


_FFS_API void  DefaultErrorHandler(int type, const char *err,const char *msg,const char* file,int line)
{
	switch(type)
	{
	case ERROR_CLASS_EXCEPTION:
		throw CSException(err,msg);
		break;
	case ERROR_CLASS_PROGRAM_ERROR:
		{
#ifdef _DEBUG
			_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_WNDW );

			if(_CrtDbgReport(_CRT_ERROR,file,line,NULL,msg)==1)
				__debugbreak(); //retry.
#else
			DefaultErrorHandler(ERROR_CLASS_EXCEPTION,err,msg,file,line);
#endif
		}
		break;
	case ERROR_CLASS_WARNING:
		//OutputDebugString("\n\nWARNING:%s\n\n",err);
		break;
	}
}

static ErrorHandler g_errorHandler=DefaultErrorHandler;

_FFS_API void SetErrorHandler(ErrorHandler handler)
{
	if(handler)
		g_errorHandler=handler;
	else
		g_errorHandler=DefaultErrorHandler;
}

_FFS_API ErrorHandler GetErrorHandler()
{
	return g_errorHandler;
}


_FF_END

using namespace ff;

#include <iostream>

using std::cerr;
using std::endl;

_FFS_API void  _FF_HandleError(int type, const char *err,const char *msg,const char* file,int line)
{
	cerr << "Error: " << err << endl;
	cerr << "Message: " << msg << endl;
	cerr << "File: " << file << endl;
	cerr << "Line: " << line << endl;
	/*
	ff_mem mem;

	if(err==ERR_WINAPI_FAILED && (!msg||msg[0]=='\0') )
	{ //get winapi error message
		const int BUF_SIZE=1024;

		char *buf=(char*)ff_alloc(BUF_SIZE);
		size_t nsz=FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,::GetLastError(),0,buf,BUF_SIZE,NULL);
		buf[nsz]=0;

		mem.set_mem(buf);
		msg=buf;
	}

	//output error message in IDE output window
//	OutputDebugStringA(StrFormat("\n***************\nFVT %s:\n",type==FVT_ERROR_WARNING? "Warning":type==FVT_ERROR_EXCEPTION? "Exception":"Error").c_str());
//	OutputDebugStringA(StrFormat("%s(%d) : %s",file,line,msg.c_str()).c_str());
//	OutputDebugStringA("\n***************\n");

	_FF_NS(g_errorHandler)(type,err,msg,file,line);
	*/
}

_FFS_API void  _FF_HandleError(int type, const char *err,const wchar_t *msg,const char* file,int line)
{
	cerr << "Error: " << err << endl;
	cerr << "Message: " << msg << endl;
	cerr << "File: " << file << endl;
	cerr << "Line: " << line << endl;
	/*
	ff_mem mem;
	char* amsg=ff_w2a(msg, mem);

	_FF_HandleError(type,err,amsg,file,line);
	*/
}


extern _FFS_API  const char *ERR_UNKNOWN				="ERR_UNKNOWN";
extern _FFS_API  const char *ERR_BAD_ALLOC				="ERR_BAD_ALLOC";
extern _FFS_API  const char *ERR_ASSERT_FAILED			="ERR_ASSERT_FAILED";

extern _FFS_API  const char *ERR_INVALID_ARGUMENT		="ERR_INVALID_ARGUMENT";
extern _FFS_API  const char *ERR_INVALID_OP				="ERR_INVALID_OP";
extern _FFS_API  const char *ERR_INVALID_FORMAT			="ERR_INVALID_FORMAT";

extern _FFS_API  const char *ERR_NULL_POINTER			="ERR_NULL_POINTER";
extern _FFS_API  const char *ERR_BUFFER_OVERFLOW		="ERR_BUFFER_OVERFLOW";

extern _FFS_API  const char *ERR_FILE_OPEN_FAILED		="ERR_FILE_OPEN_FAILED";
extern _FFS_API  const char *ERR_FILE_READ_FAILED		="ERR_FILE_READ_FAILED";
extern _FFS_API  const char *ERR_FILE_WRITE_FAILED		="ERR_FILE_WRITE_FAILED";
extern _FFS_API  const char *ERR_FILE_OP_FAILED			="ERR_FILE_OP_FAILED";

extern _FFS_API  const char *ERR_WINAPI_FAILED			="ERR_WINAPI_FAILED";
