
#include "iff/util.h"

#include<memory>
#include<limits>
#include<vector>
#include<stack>

#undef max
#undef min

#define _MACRO_ARG(arg) arg

#include "bfc/cfg.h"
#include"bfc/vector.h"

#undef min
#undef max
#include <cstring>

_IFF_BEG

//-------------------------------------------------------------------------------

template<typename _ValT>
static void fiiFillBits(void *pPixel,const int count,const double value)
{
	_ValT* ppv=(_ValT*)pPixel;
	const _ValT MIN=std::numeric_limits<_ValT>::min();
	const _ValT MAX=std::numeric_limits<_ValT>::max();
	_ValT val=_ValT(value<MIN? MIN:value>MAX? MAX:value);
	for(int i=0;i<count;++i)
		ppv[i]=val;
}

void   fiuFillBits(void *pDest,int depth,int count,double value)
{
	typedef void (*_FuncT)(void *pPixel,const int cn,const double value);

	static const _FuncT _pFuncTab[]=
	{
		FI_DEPTH_FUNC_LIST(fiiFillBits)
	};
	if(pDest)
		_pFuncTab[FI_DEPTH_ID(depth)](pDest,count,value);
}

//-------------------------------------------------------------------------------

void    fiuCopy(const void* pIn,const int rowSize,const int height,const int istep,
			  void* pOut,const int ostep)
{
	assert(rowSize<=istep);
	if(pIn&&pOut)
	{
		//if both source and dest is continuous.
		if(istep==ostep&&istep==rowSize)
			memcpy(pOut,pIn,height*istep);
		else
		{ //else copy line by line.
			const uchar* pi=(uchar*)pIn;
			uchar* po=(uchar*)pOut;
			for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
				memcpy(po,pi,rowSize);
		}
	}
}

//-------------------------------------------------------------------------------

template<int cps>
static void _fiuiCopyPixel(const void *pIn,const int width,const int height,const int istep,const int ips,
						 void *pOut,const int ostep,const int ops)
{
	if(pIn&&pOut)
	{
		uchar *pi=(uchar*)pIn,*po=(uchar*)pOut;
		for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
		{
			uchar *pix=pi,*pox=po;
			for(int xi=0;xi<width;++xi,pix+=ips,pox+=ops)
				memcpy(pox,pix,cps);
		}
	}
}
static void _fiuiCopyPixel(const void *pIn,const int width,const int height,const int istep,const int ips,
						 void *pOut,const int ostep,const int ops,const int cps)
{
	if(pIn&&pOut)
	{
		uchar *pi=(uchar*)pIn,*po=(uchar*)pOut;
		for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
		{
			uchar *pix=pi,*pox=po;
			for(int xi=0;xi<width;++xi,pix+=ips,pox+=ops)
				memcpy(pox,pix,cps);
		}
	}
}

void  fiuCopyPixel(const void *pIn,const int width,const int height,const int istep,const int ips,
						 void *pOut,const int ostep,const int ops,const int cps)
{
	typedef void (*_FuncT)(const void *pIn,const int width,const int height,const int istep,const int ips,
						 void *pOut,const int ostep,const int ops);

	static const _FuncT _pFuncTab[]=
	{
		_fiuiCopyPixel<1>,_fiuiCopyPixel<2>,_fiuiCopyPixel<3>,_fiuiCopyPixel<4>
	};

	if(uint(cps)<=4)
		(_pFuncTab[cps-1])(pIn,width,height,istep,ips,pOut,ostep,ops);
	else
		_fiuiCopyPixel(pIn,width,height,istep,ips,pOut,ostep,ops,cps);
}

void _IFF_API fiuRotate90(const void *pIn, const int width, const int height, const int istep, void *pOut, const int ostep, const int ps, const int n90)
{
	int k=n90%4;

	if(k==1)
	{
		fiuCopyPixel( (char*)pIn+ps*(width-1), height, width, -ps, istep, pOut, ostep, ps, ps );
	}
	else if(k==2)
	{
		fiuCopyPixel( (char*)pIn+istep*(height-1)+(width-1)*ps, width, height, -istep, -ps, pOut, ostep, ps, ps);
	}
	else if(k==3)
	{
		fiuCopyPixel( (char*)pIn+istep*(height-1), height, width, ps, -istep, pOut, ostep, ps, ps);
	}
	else if(k==0)
	{
		fiuCopy(pIn,width*ps,height,istep,pOut,ostep);
	}
}

//-------------------------------------------------------------------------------

void   fiuSetMem(void* pOut,const int rowSize,const int height,const int step,const char val)
{
	assert(rowSize<=step);
	if(pOut)
	{
		if(rowSize==step)
			memset(pOut,val,rowSize*height);
		else
		{
			uchar* pox=(uchar*)pOut;
			for(int i=0;i<height;++i,pox+=step)
				memset(pox,val,rowSize);
		}
	}
}

//-------------------------------------------------------------------------------

void   fiuSetBoundary(void *pOut,const int rowSize,const int height,const int step,
							  const int nTopLines,const int nBottomLines,
							  const int nLeftBytes,const int nRightBytes,
							  const char val
							  )
{
	assert(rowSize<=step);
	if(pOut)
	{
		//top
		uchar *pox=(uchar*)pOut;
		int yi=0;
		for(;yi<nTopLines;++yi,pox+=step)
			memset(pox,val,rowSize);

		for(;yi<height-nBottomLines;++yi,pox+=step)
		{
			//left
			memset(pox,val,nLeftBytes);
			//right
			memset(pox+rowSize-nRightBytes,val,nRightBytes);
		}
		//bottom
		for(;yi<height;++yi,pox+=step)
			memset(pox,val,rowSize);
	}
}

//-------------------------------------------------------------------------------
template<int cps>
static void _fiuiSetPixel(uchar* po,const int width,const int height,const int step,
				  const int pw,const void* pval)
{
	for(int yi=0;yi<height;++yi,po+=step)
	{
		uchar *pox=po;
		for(int xi=0;xi<width;++xi,pox+=pw)
			memcpy(pox,pval,cps);
	}
}
static void _fiuiSetPixel(uchar* po,const int width,const int height,const int step,
				  const int pw,const void* pval,int cps)
{
	for(int yi=0;yi<height;++yi,po+=step)
	{
		uchar *pox=po;
		for(int xi=0;xi<width;++xi,pox+=pw)
			memcpy(pox,pval,cps);
	}
}
static void _fiuiSetContPixel(uchar* po,const int width,const int height,const int step,
				  const int pw,const void* pval)
{
	uchar *pox=po;
	//fill the first line pixel by pixel.
	for(int i=0;i<width;++i,pox+=pw)
		memcpy(pox,pval,pw);
	//copy first line to fill other lines.
	const int rowSize=width*pw;
	pox=(uchar*)po+step;
	for(int i=1;i<height;++i,pox+=step)
		memcpy(pox,po,rowSize);
}

void   fiuSetPixel(void* pOut,const int width,const int height,const int step,
				  const int pw,const void* pval,const int cps)
{
	typedef void (*_FuncT)(uchar* po,const int width,const int height,const int step,
				  const int pw,const void* pval);

	static const _FuncT _pFuncTab[]=
	{
		_fiuiSetPixel<1>,_fiuiSetPixel<2>,_fiuiSetPixel<3>,_fiuiSetPixel<4>
	};

	if(pOut&&pval)
	{
		assert(width*pw<=step);

		if(pw==cps)
			_fiuiSetContPixel((uchar*)pOut,width,height,step,pw,pval);
		else
		{
			if(uint(cps)<=4)
				(_pFuncTab[cps-1])((uchar*)pOut,width,height,step,pw,pval);
			else
				_fiuiSetPixel((uchar*)pOut,width,height,step,pw,pval,cps);
		}
	}
}

//-------------------------------------------------------------------------------

void    fiuSetBoundaryPixel(void *pOut,const int width,const int height,const int step,
									const int ps,const void *pval,const int cps,
									const int nTopLines,const int nBottomLines,
									const int nLeftPixels,const int nRightPixels
									)
{
	if(pOut)
	{
		uchar *pox=(uchar*)pOut;

		if(nTopLines>0)
			fiuSetPixel(pox,width,nTopLines,step,ps,pval,cps);
		if(nBottomLines>0)
			fiuSetPixel(pox+(height-nBottomLines)*step,width,nBottomLines,step,ps,pval,cps);
		if(nLeftPixels>0)
			fiuSetPixel(pox+nTopLines*step,nLeftPixels,height-nTopLines-nBottomLines,step,ps,pval,cps);
		if(nRightPixels>0)
			fiuSetPixel(pox+nTopLines*step+ps*(width-nRightPixels),nRightPixels,height-nTopLines-nBottomLines,step,ps,pval,cps);
	}
}

//-------------------------------------------------------------------------------


void  fiuFlipX(const void* pIn,const int rowSize,const int height,const int istep,
			  void* pOut,const int ostep)
{
	assert(rowSize<=istep);
	if(pIn&&pOut)
	{
		const uchar* pi=(uchar*)pIn+istep*(height-1);
		uchar* po=(uchar*)pOut;
		for(int yi=0;yi<height;++yi,pi-=istep,po+=ostep)
			memcpy(po,pi,rowSize);
	}
}
void    fiuFlipX(void* pInOut,const int rowSize,const int height,const int step)
{
	assert(rowSize<=step);
	if(pInOut)
	{
		char* po=(char*)pInOut,*pi=(char*)pInOut+(height-1)*step;
		char *pBuf=new char[rowSize];
		for(;po<pi;po+=step,pi-=step)
		{
			memcpy(pBuf,po,rowSize);
			memcpy(po,pi,rowSize);
			memcpy(pi,pBuf,rowSize);
		}
		delete[]pBuf;
	}
}

//-----------------------------------------------------------------------------------

struct _cfiuFlipY
{
	template<int pixelSize>
	static void Flip(const void* pIn,int iwidth,int iheight,int istep,
		void* pOut,int ostep)
	{//optimized version, which is dependent on the compiler optimization to @memcpy when
	 //the copied bytes is a compile time determined constant.
		char* pi=(char*)pIn+iwidth*pixelSize,*po=(char*)pOut;
		for(int yi=0;yi<iheight;++yi,pi+=istep,po+=ostep)
		{
			char* pix=pi,*pox=po;
			for(int xi=0;xi<iwidth;++xi,pix-=pixelSize,pox+=pixelSize)
				memcpy(pox,pix,pixelSize);
		}
	}
	static void Flip(const void* pIn,int iwidth,int iheight,int istep,int pixelSize,
		void* pOut,int ostep)
	{
		char* pi=(char*)pIn+iwidth*pixelSize,*po=(char*)pOut;
		for(int yi=0;yi<iheight;++yi,pi+=istep,po+=ostep)
		{
			char* pix=pi,*pox=po;
			for(int xi=0;xi<iwidth;++xi,pix-=pixelSize,pox+=pixelSize)
				memcpy(pox,pix,pixelSize);
		}
	}
	template<int pixelSize>
	static void Flip(void* pio,int width,int height,int step)
	{
		char pBuf[pixelSize];
		for(int yi=0;yi<height;++yi,pio=(char*)pio+step)
		{
			char* pl=(char*)pio,*pr=pl+(width-1)*pixelSize;
			for(;pl<pr;pl+=pixelSize,pr-=pixelSize)
			{
				memcpy(pBuf,pl,pixelSize);
				memcpy(pl,pr,pixelSize);
				memcpy(pr,pBuf,pixelSize);
			}
		}
	}
	static void Flip(void* pio,int width,int height,int step,int pixelSize)
	{
		char* pBuf=new char[pixelSize];
		for(int yi=0;yi<height;++yi,pio=(char*)pio+step)
		{
			char* pl=(char*)pio,*pr=pl+(width-1)*pixelSize;
			for(;pl<pr;pl+=pixelSize,pr-=pixelSize)
			{
				memcpy(pBuf,pl,pixelSize);
				memcpy(pl,pr,pixelSize);
				memcpy(pr,pBuf,pixelSize);
			}
		}
		delete[]pBuf;
	}
};


void   fiuFlipY(const void* pIn,const int width,const int height,const int istep,const int pixelSize,
		  void* pOut,int ostep)
{
	if(!pIn||!pOut)
		return;

	switch(pixelSize)
	{//optimize for the cases when pixel-size is 1,2,3,4.
	case 1:
		_cfiuFlipY::Flip<1>(pIn,width,height,istep,pOut,ostep);
		break;
	case 2:
		_cfiuFlipY::Flip<2>(pIn,width,height,istep,pOut,ostep);
		break;
	case 3:
		_cfiuFlipY::Flip<3>(pIn,width,height,istep,pOut,ostep);
		break;
	case 4:
		_cfiuFlipY::Flip<4>(pIn,width,height,istep,pOut,ostep);
		break;
	default:
		_cfiuFlipY::Flip(pIn,width,height,istep,pixelSize,pOut,ostep);
	}
}

void   fiuFlipY(void* pInOut,const int width,const int height,const int istep,const int pixelSize)
{
	if(!pInOut)
		return;

	switch(pixelSize)
	{//optimize for pixelSize 1,2,3,4.
	case 1:
		_cfiuFlipY::Flip<1>(pInOut,width,height,istep);
		break;
	case 2:
		_cfiuFlipY::Flip<2>(pInOut,width,height,istep);
		break;
	case 3:
		_cfiuFlipY::Flip<3>(pInOut,width,height,istep);
		break;
	case 4:
		_cfiuFlipY::Flip<4>(pInOut,width,height,istep);
		break;
	default:
		_cfiuFlipY::Flip(pInOut,width,height,istep,pixelSize);
	}
}

//-----------------------------------------------------------------------------------

void   fiuTile(const void* pIn,const int iwidth,const int iheight,const int istep,
					  void* pOut,const int owidth,const int oheight,const int ostep,
					  const int pixelSize)
{
	if(pIn&&iwidth&&iheight&&pOut)
	{
		const int nx=owidth/iwidth,sz_xlast=owidth%iwidth*pixelSize;
		//bytes to copy per-line.
		const int szx=iwidth*pixelSize;

		uchar* po=(uchar*)pOut;
		for(int yi=0;yi<oheight;++yi,po+=ostep)
		{
			const uchar* pi=(uchar*)pIn+yi%iheight*istep;
			uchar* pox=po;
			//copy total lines.
			for(int i=0;i<nx;++i,pox+=szx)
				memcpy(pox,pi,szx);
			//copy left pixels.
			if(sz_xlast!=0)
				memcpy(pox,pi,sz_xlast);
		}
	}
}

//-----------------------------------------------------------------------------------

template<typename _SrcT,typename _DestT>
static void _fiuiScaleDepth(const uchar* pi,const int width,const int height,const int istep,const int cn,
				   uchar* po,const int ostep,double scale,double shift)
{
	_DestT _min=std::numeric_limits<_DestT>::min(),_max=std::numeric_limits<_DestT>::max();

	const int  ncw=cn*width;
	for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
	{
		_SrcT *pix=(_SrcT*)pi;
		_DestT* pox=(_DestT*)po;
		for(int xi=0;xi<ncw;++xi,++pix,++pox)
		{
			double val=(*pix)*scale+shift;
			*pox=_DestT(val<_min? _min:val>_max? _max:val);
		}
	}
}
template<typename _SrcT>
static void _fiuiCallScaleDepth(const uchar* pi,const int width,const int height,const int istep,const int cn,
					   uchar* po,const int ostep,int depth,double scale,double shift)
{
	typedef void (*_FuncT)(const uchar* pi,const int width,const int height,const int istep,const int cn,
		uchar* po,const int ostep,double scale,double shift);

#define _PRE_FIX &_fiuiScaleDepth<_SrcT,

	static _FuncT _pFuncTab[]=
	{
		FI_MAKE_DEPTH_FUNC_LIST(_MACRO_ARG(_PRE_FIX),>)
	};

#undef _PRE_FIX

	_pFuncTab[FI_DEPTH_ID(depth)](pi,width,height,istep,cn,po,ostep,scale,shift);
}

void   fiuTransformPixel(const void* pIn,const int width,const int height,const int istep,const int itype,
	   void* pOut,const int ostep,const int odepth,const double scale,const double shift)
{
	typedef void (*_FuncT)(const uchar* pi,const int width,const int height,const int istep,const int cn,
		uchar* po,const int ostep,int depth,double scale,double shift);

	static const _FuncT _pFuncTab[]=
	{
		FI_DEPTH_FUNC_LIST(_fiuiCallScaleDepth)
	};

	FI_ASSERT_TYPE(itype);
	FI_ASSERT_DEPTH(odepth);

	if(pIn&&pOut)
	{
		_pFuncTab[FI_DEPTH_ID(FI_DEPTH(itype))]((uchar*)pIn,width,height,istep,FI_CN(itype),(uchar*)pOut,ostep,odepth,scale,shift);
	}
}

//--------------------------------------------------------------------------------------

template<int cw>
static void _fiuiSwapChannels(void *pImg,const int width,const int height,const int step,
							  const int cn,const int ic0,const int ic1)
{
	assert(uint(ic0)<uint(cn)&&uint(ic1)<uint(cn));

	const int ib=std::min(ic0,ic1),id=(std::max(ic0,ic1)-ib)*cw;
	if(id>0)
	{
		const int pw=cw*cn;
		uchar* pi=(uchar*)pImg+ib*cw;
		uchar buf[cw];
		for(int yi=0;yi<height;++yi,pi+=step)
		{
			uchar* pix=pi;
			for(int xi=0;xi<width;++xi,pix+=pw)
			{
				memcpy(buf,pix,cw);
				memcpy(pix,pix+id,cw);
				memcpy(pix+id,buf,cw);
			}
		}
	}
}

void   fiuSwapChannels(void *pImg,const int width,const int height,const int step,
							  const int type,const int ic0,const int ic1)
{
	typedef void (*_FuncT)(void *pImg,const int width,const int height,const int step,
							  const int cn,const int ic0,const int ic1);

	static _FuncT _pFuncTab[]={_fiuiSwapChannels<1>,_fiuiSwapChannels<2>,0,_fiuiSwapChannels<4>,
								0,0,0,_fiuiSwapChannels<8>};

	FI_ASSERT_TYPE(type);
	const int idx=FI_DEPTH_SIZE(type)-1;
	if(uint(idx)>=sizeof(_pFuncTab)/sizeof(_pFuncTab[0])||
		_pFuncTab[idx]==NULL)
	{
		FF_ERROR(ERR_INVALID_ARGUMENT,"");
	}
	else
	{
		if(pImg)
			(_pFuncTab[idx])(pImg,width,height,step,FI_CN(type),ic0,ic1);
	}
}

//--------------------------------------------------------------------------------------

template<typename _Ty>
static void _fiuiColorToGrayAverage(const uchar* pi,const int width,const int height,const int istep,const int cn,
							   uchar*  po,const int ostep)
{
	assert(cn==3||cn==4);
	for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
	{
		const _Ty* pix=(_Ty*)pi;
		_Ty* pox=(_Ty*)po;
		typedef typename FI_ACCUM_TYPE<_Ty>::type _AccumT;
		for(int xi=0;xi<width;++xi,pix+=cn,++pox)
			*pox=_Ty((_AccumT(pix[0])+pix[1]+pix[2])/3);
	}
}

static void fiuiColorToGrayAverage(const void* pIn,const int width,const int height,const int istep,const int type,
							void* pOut,const int ostep)
{
	typedef void (*_FuncT)(const uchar* pi,const int width,const int height,const int istep,const int cn,
							   uchar*  po,const int ostep);

	static const _FuncT pFuncTab[]=
	{
		FI_DEPTH_FUNC_LIST(_fiuiColorToGrayAverage)
	};
	FI_ASSERT_TYPE(type);
	if(pIn&&pOut)
		(pFuncTab[FI_DEPTH_ID(type)])((uchar*)pIn,width,height,istep,FI_CN(type),(uchar*)pOut,ostep);
}

template<typename _Ty>
static void _fiuiColorToGrayWeight(const uchar* pi,const int width,const int height,const int istep,const int cn,
							   uchar*  po,const int ostep,const double w0,const double w1,const double w2)
{
	assert(cn==3||cn==4);
	for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
	{
		const _Ty* pix=(_Ty*)pi;
		_Ty* pox=(_Ty*)po;
		typedef typename FI_ACCUM_TYPE<_Ty>::type _AccumT;
		for(int xi=0;xi<width;++xi,pix+=cn,++pox)
			*pox=_Ty(pix[0]*w0+pix[1]*w1+pix[2]*w2);
	}
}
void   fiuColorToGray(const void* pIn,const int width,const int height,const int istep,const int type,
							void* pOut,const int ostep,
							const double w0,const double w1,const double w2)
{
	typedef void (*_FuncT)(const uchar* pi,const int width,const int height,const int istep,const int cn,
							   uchar*  po,const int ostep,const double,const double,const double);

	static const _FuncT pFuncTab[]=
	{
		FI_DEPTH_FUNC_LIST(_fiuiColorToGrayWeight)
	};
	FI_ASSERT_TYPE(type);
	if(pIn&&pOut)
	{
		//convert by averge the R,G,B value.
		if(w0==w1&&w1==w2&&fabs((w0+w1+w2)-1.0)<1e-6)
			fiuiColorToGrayAverage(pIn,width,height,istep,type,pOut,ostep);
		else
		{//convert with weight sum.
			(pFuncTab[FI_DEPTH_ID(type)])((uchar*)pIn,width,height,istep,FI_CN(type),(uchar*)pOut,ostep,w0,w1,w2);
		}
	}
}

//--------------------------------------------------------------------------------------

template<int cw>
void _fiuiGrayToColor(const void* pGray,const int width,const int height,const int istep,const int icn,
					void* pColor,const int ostep,const int ocn,const void* pAlpha)
{
	assert((icn==1||icn==2)&&ocn>=2&&ocn<=4);

	uchar *pi=(uchar*)pGray,*po=(uchar*)pColor;
	const int ipw=cw*icn;

	for(int yi=0;yi<height;++yi,pi+=istep,po+=ostep)
	{
		uchar* pix=pi,*pox=po;

		if(pAlpha)
		{
			for(int xi=0;xi<width;++xi,pix+=ipw,pox+=cw)
			{
				for(int ci=0;ci<ocn-1;++ci,pox+=cw)
					memcpy(pox,pix,cw);
				memcpy(pox,pAlpha,cw);
			}
		}
		else
		{
			for(int xi=0;xi<width;++xi,pix+=ipw)
			{
				for(int ci=0;ci<ocn;++ci,pox+=cw)
					memcpy(pox,pix,cw);
			}
		}
	}
}

void  fiuGrayToColor(const void* pGray,const int width,const int height,const int istep,const int type,
					void* pColor,const int ostep,const int ocn,double alpha)
{
	typedef void (*_FuncT)(const void* pGray,const int width,const int height,const int istep,const int icn,
					void* pColor,const int ostep,const int ocn,const void* pAlpha);

	static const _FuncT _pTab[]=
	{
		_fiuiGrayToColor<1>,_fiuiGrayToColor<2>,NULL,_fiuiGrayToColor<4>
	};
	const int depth=FI_DEPTH(type);
	const int cw=FI_DEPTH_SIZE(depth);

	if(uint(cw)<=4)
	{
		if(_FuncT pFunc=_pTab[cw-1])
		{
			char pAlpha[sizeof(double)];
			//ipMakeTypeValue(pAlpha,alpha,depth);
			fiuFillBits(pAlpha,FI_DEPTH(type),1,alpha);

			pFunc(pGray,width,height,istep,FI_CN(type),pColor,ostep,ocn,ocn==4||ocn==2? pAlpha:NULL);
			return ;
		}
	}

	assert(false);
}


//--------------------------------------------------------------------------------------

void  fiuCopyChannels(const void* pSrc,const int width,const int height,const int istep,const int type,
					 const int icBeg,const int icEnd,
					 void* pDest,const int ostep,const int ocn,const int ocBeg)
{
#ifdef _DEBUG
	const uint icn=FI_CN(type);

	assert(uint(icBeg)<icn&&uint(icEnd)<=icn&&icBeg<icEnd);
	assert(uint(ocBeg)<uint(ocn)&&uint(ocBeg+icEnd-icBeg)<=uint(ocn));
#endif

	if(pSrc&&pDest)
	{
		const int ipw=FI_PIXEL_SIZE(type),cw=FI_DEPTH_SIZE(FI_DEPTH(type)),opw=ocn*cw;

		fiuCopyPixel((uchar*)pSrc+cw*icBeg,width,height,istep,ipw,(uchar*)pDest+ocBeg*cw,
			ostep,opw,(icEnd-icBeg)*cw);
	}
}

//--------------------------------------------------------------------------------------

////convert image format.
////@unknown : the value to fill the unknown channel, e.g. the alpha channel in function
////			 @fiuGrayToColor and @fiuCopyChannels.
//
//void  fiuConvertFormat(const void *pIn,const int width,const int height,const int istep,const int ifmt,
//							  void *pDest,const int ostep, const int ofmt, double unknown=0);


void  fiuConvertRGBChannels(const void *pIn,const int width,const int height,const int istep,const int type,
							 void *pDest, const int ostep, const int dcn,double alpha)
{
	bool bFail=false;
	const int scn=FI_CN(type);
	if(scn==dcn)
		fiuCopy(pIn,width*FI_PIXEL_SIZE(type),height,istep,pDest,ostep);
	else
		if(scn>dcn)
		{
			if((scn==3||scn==4)&&dcn==1)
				fiuColorToGray(pIn,width,height,istep,type,pDest,ostep,1.0/3,1.0/3,1.0/3);
			else
				if(scn==4&&dcn==3||scn==2&&dcn==1)
					fiuCopyChannels(pIn,width,height,istep,type,0,dcn,pDest,ostep,dcn,0);
				else
					bFail=true;
		}
		else
		{
			if(scn==1&&(dcn==3||dcn==4))
				fiuGrayToColor(pIn,width,height,istep,type,pDest,ostep,dcn,alpha);
			else
				if(scn==3&&dcn==4||scn==1&&dcn==2)
				{
					char pv[sizeof(double)];
					fiuFillBits(pv,FI_DEPTH(type),1,alpha);

					const int cw=FI_DEPTH_SIZE(FI_DEPTH(type));
					fiuSetPixel((uchar*)pDest+cw*scn,width,height,ostep,cw*dcn,pv,cw);

					fiuCopyChannels(pIn,width,height,istep,type,0,scn,pDest,ostep,dcn,0);
				}
				else
					bFail=true;
		}

	if(bFail)
		FF_EXCEPTION(ERR_INVALID_ARGUMENT,"");
}

_FF_END

#include "iff/resample.h"

_FF_BEG
//================================================================================

static void _calc_resize_tab(int width, int dwidth, int xt[], int scale)
{
	double s=dwidth<=1? 0: double(width-1)*scale/(dwidth-1);

	for(int i=0; i<dwidth; ++i)
	{
		xt[i]= int(i*s);
	}
}


static void _calc_resize_tab(int width, int dwidth, double xt[], int)
{
	double s=dwidth<=1? 0: double(width-1)/(dwidth-1);

	for(int i=0; i<dwidth; ++i)
	{
		xt[i]= i*s;
	}
}

template<int _Shift, int _CN, typename _ValT, typename _RST, typename _TValT>
static void _resize_3(uchar *dest, int dwidth, int dheight, int dstep, const _TValT xt[], const _TValT yt[], _RST &rs)
{
	for(int yi=0; yi<dheight; ++yi, dest+=dstep)
	{
		_ValT *pdx=(_ValT*)dest;
		const _TValT y=yt[yi];

		for(int xi=0; xi<dwidth; ++xi, pdx+=_CN)
		{
			rs.template resample<_Shift,_CN,_ValT>(xt[xi], y, pdx);
		}
	}
}

template<int _Shift, typename _ValT, typename _RST, typename _TValT>
static void _resize_2(uchar *dest, int dwidth, int dheight, int dstep, int cn, const _TValT xt[], const _TValT yt[], _RST &rs)
{
	typedef void (*_FUNCT)(uchar*,int,int,int,const _TValT [], const _TValT [], _RST&);

	static _FUNCT fp[]=
	{
		&_resize_3<_Shift,1,_ValT,_RST,_TValT>, &_resize_3<_Shift,2,_ValT,_RST,_TValT>, &_resize_3<_Shift,3,_ValT,_RST,_TValT>, &_resize_3<_Shift,4,_ValT,_RST,_TValT>
	};

	(fp[cn-1])(dest,dwidth,dheight,dstep,xt,yt,rs);
}

template<int _Shift, typename _RST, typename _TValT>
static void _resize_1(uchar *dest, int dwidth, int dheight, int dstep, int type, const _TValT xt[], const _TValT yt[], _RST &rs, FixNNResampler *)
{
	typedef void (*_FUNCT)(uchar*,int,int,int,int, const _TValT [], const _TValT [], _RST&);

	static _FUNCT fp[]=
	{
		&_resize_2<_Shift,char,_RST,_TValT>,
		&_resize_2<_Shift,uchar,_RST,_TValT>,
		&_resize_2<_Shift,short,_RST,_TValT>,
		&_resize_2<_Shift,ushort,_RST,_TValT>,
		&_resize_2<_Shift,int,_RST,_TValT>,
		NULL
	};

	assert(fp[FI_DEPTH_ID(type)]);

	(fp[FI_DEPTH_ID(type)])(dest,dwidth,dheight,dstep,FI_CN(type),xt,yt,rs);
}

template<int _Shift, typename _RST, typename _TValT>
static void _resize_1(uchar *dest, int dwidth, int dheight, int dstep, int type, const _TValT xt[], const _TValT yt[], _RST &rs, NNResampler *)
{
	typedef void (*_FUNCT)(uchar*,int,int,int,int, const _TValT [], const _TValT [], _RST&);

	static _FUNCT fp[]=
	{
		NULL,NULL,NULL,NULL,NULL, &_resize_2<_Shift,float,_RST,_TValT>
	};

	assert(fp[FI_DEPTH_ID(type)]);

	(fp[FI_DEPTH_ID(type)])(dest,dwidth,dheight,dstep,FI_CN(type),xt,yt,rs);
}

template<int _Shift, typename _RST, typename _NNR, typename _TValT>
static void _resize_0(const void *src, int width, int height, int istep, int type, uchar *dest, int dwidth, int dheight, int dstep, const _TValT xt[], const _TValT yt[])
{
	_RST rs;

	rs.Init(src,width,height,istep,_Shift);
	_resize_1<_Shift>(dest,dwidth,dheight,dstep,type,xt,yt,rs,(_NNR*)NULL);
}

const int _RESIZE_SHIFT=10;

template<int _Shift, typename _NNR,typename _BLR, typename _CUBR, typename _TValT>
static void _resize(const void *src, int width, int height, int istep, int type, uchar *dest, int dwidth, int dheight, int dstep, int resampleMethod)
{
	_TValT *xt=new _TValT[dwidth], *yt=new _TValT[dheight];

	_calc_resize_tab(width,dwidth,xt,1<<_RESIZE_SHIFT);
	_calc_resize_tab(height,dheight,yt,1<<_RESIZE_SHIFT);

	switch(resampleMethod)
	{
	case RESAMPLE_NN:
		_resize_0<_Shift,_NNR,_NNR>(src,width,height,istep,type,dest,dwidth,dheight,dstep,xt,yt);
		break;
	case RESAMPLE_LINEAR:
		_resize_0<_Shift,_BLR,_NNR>(src,width,height,istep,type,dest,dwidth,dheight,dstep,xt,yt);
		break;
	case RESAMPLE_CUBIC:
		_resize_0<_Shift,_CUBR,_NNR>(src,width,height,istep,type,dest,dwidth,dheight,dstep,xt,yt);
		break;
	}

	delete[]xt;
	delete[]yt;
}

void _IFF_API fiuResize(const void *src, int width, int height, int istep, int type, void *dest, int dwidth, int dheight, int dstep, int resampleMethod)
{
	if(src&&dest)
	{
		if(dwidth<width&&dheight<height)
			resampleMethod=RESAMPLE_NN;

		if(FI_DEPTH(type)!=FI_32F)
		{
			_resize<_RESIZE_SHIFT,FixNNResampler,FixBL4Resampler,FixCubicResampler, int>(src,width,height,istep,type, (uchar*)dest,dwidth,dheight,dstep,resampleMethod);
		}
		else
		{
			_resize<_RESIZE_SHIFT,NNResampler,BL4Resampler,CubicResampler, double>(src,width,height,istep,type, (uchar*)dest,dwidth,dheight,dstep,resampleMethod);
		}

	}
}


//return number of merged classes.
static int fiuiMergeEqClass(const std::vector<Vector<int,2> > &veq,int nc,std::vector<int> &remap)
{
	std::vector<std::vector<int> > vmap(nc);
	for(size_t i=0;i<veq.size();++i)
	{
		const Vector<int,2> &e(veq[i]);

		vmap[e[0]].push_back(e[1]);
		vmap[e[1]].push_back(e[0]);
	}
	int tv=0,mv=-1;
	std::vector<int> vtag(nc,-1);
	remap.resize(nc);

	for(int i=0;i<nc;++i)
	{
		if(vtag[i]<0)
		{
			++mv;
			if(!vmap[i].empty())
			{
				std::stack<int> stk;
				stk.push(i);
				++tv;
				while(!stk.empty())
				{
					int top=stk.top();
					stk.pop();
					vtag[top]=tv;

					const std::vector<int> &vmi(vmap[top]);
					for(size_t j=0;j<vmi.size();++j)
						if(vtag[vmi[j]]!=tv)
						{
							assert(vtag[vmi[j]]<0);
							stk.push(vmi[j]);
						}
				}
				for(int i=0;i<nc;++i)
					if(vtag[i]==tv)
						remap[i]=mv;
			}
			else
			{
				remap[i]=mv;
				vtag[i]=0;
			}
		}
	}
	return mv+1;
}

template<typename _PtrValT>
inline _PtrValT * ByteDiff(_PtrValT *ptr,int off)
{
	return (_PtrValT*)((char*)ptr+off);
}

#define _CHK_POS(dx,dy) \
	{if(*(psx+dy*istep+dx)==*psx) \
	{ \
		int idx=*ByteDiff(pcx,dy*ostep+dx*(int)sizeof(_DepthValT)); \
		assert(idx>0); \
		if(id0>0) \
		{ \
			if(id0!=idx&&veq.back()!=Vector<int,2>(id0,idx)) \
				veq.push_back(Vector<int,2>(id0,idx)); \
		} \
		else \
			*pcx=id0=idx; \
	}}

template<typename _DepthValT>
static int fiuiConnectedComponent(const uchar *pSrc,const int width,const int height,const int istep,
									uchar *pCC,const int ostep,
									const int bkVal,bool bC8
									)
{
	std::vector<Vector<int,2> > veq;
	veq.reserve(1024*4);

	veq.push_back(Vector<int,2>(0,0));

	int id=0;

	for(int yi=0;yi<height;++yi,pSrc+=istep,pCC+=ostep)
	{
		_DepthValT *pcx=(_DepthValT*)pCC;
		const uchar *psx=pSrc;
		for(int xi=0;xi<width;++xi,++pcx,++psx)
		{
			if(*psx!=bkVal)
			{
				int id0=-1;

				if(xi>0)
				{
					_CHK_POS(-1,0) ;

					if(bC8&&yi>0)
						_CHK_POS(-1,-1);
				}
				if(yi>0)
				{
					_CHK_POS(0,-1);

					if(bC8&&xi<width-1)
						_CHK_POS(1,-1);
				}

				if(id0<0)
					*pcx=++id;
			}
#undef _CHK_POS
		}
	}

	std::vector<int> remap;
	id=fiuiMergeEqClass(veq,id+1,remap);
	pCC-=height*ostep;

	for(int yi=0;yi<height;++yi,pCC+=ostep)
	{
		_DepthValT *pcx=(_DepthValT*)pCC;
		for(int xi=0;xi<width;++xi,++pcx)
			*pcx=remap[*pcx];
	}
	return id;
}

int  fiuConnectedComponent(const uchar *pSrc,const int width,const int height,const int istep,
									void *pCC,const int ostep,const int odepth,
									const int bkVal, bool bC8
									)
{
	int nc=0;

	if(pSrc&&pCC)
	{
		fiuSetMem(pCC,width*FI_DEPTH_SIZE(odepth),height,ostep,0);

#define _CALL_IMP(dvt) nc=fiuiConnectedComponent<dvt>(pSrc,width,height,istep,(uchar*)pCC,ostep,bkVal,bC8)

		switch(odepth)
		{
		case FI_8U:
			_CALL_IMP(uchar);
			break;
		case FI_16S:
			_CALL_IMP(short);
			break;
		case FI_16U:
			_CALL_IMP(ushort);
			break;
		case FI_32S:
			_CALL_IMP(int);
			break;
		default:
			FF_EXCEPTION(ERR_INVALID_ARGUMENT,"");
		}

#undef _CALL_IMP
	}
	return nc;
}


_IFF_END
