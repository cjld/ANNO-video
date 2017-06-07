
#include "iff/iff.h"

#include "bfc/mem.h"
#include "iff/util.h"


#include "bfc/cfg.h"

_IFF_BEG

void *  fiAlloc(uint size)
{
	return ff_alloc(size);	
}
uchar*  fiAllocImageData(const int step,const int height)
{
	return (uchar*)fiAlloc(step*height);
}
uchar*  fiAllocImageData(const int width,const int height,const int type,const int align)
{
	assert(fiIsValidType(type)&&fiIsValidAlign(align));

	int step=fiStep(width,type,align);
	return fiAllocImageData(step,height);
}
void    fiFree(void* ptr)
{
	ff_free(ptr);
}

//---------------------------------------------------------------------------------

void  fiCopy(const FVTImage& src,FVTImage& dest,const int mode)
{
	assert(fiSizeEq(src,dest) && src.Type()==dest.Type());
	
	if(mode&FI_COPY_FLIP)
		fiuFlipX(src.Data(),src.LineSize(),src.Height(),src.Step(),dest.Data(),dest.Step());
	else
		fiuCopy(src.Data(),src.LineSize(),src.Height(),src.Step(),dest.Data(),dest.Step());

	if(mode&FI_COPY_SWAP_RB)
	{
		if(src.NChannels()>=3)
			fiuSwapChannels(FI_AL_DWHS(dest),dest.Type(),0,2);
	}
}

//---------------------------------------------------------------------------------
void    fiSetMem(FVTImage& img,const char val)
{
	fiuSetMem(img.Data(),img.LineSize(),img.Height(),img.Step(),val);
}
void    fiSetBoundary(FVTImage &img,const int bw,const char val)
{
	fiuSetBoundary(img.Data(),img.LineSize(),img.Height(),img.Step(),bw,bw,bw*img.PixelSize(),bw*img.PixelSize(),val);
}

void    fiSetPixel(FVTImage& img,const void* pPixelVal)
{
	fiuSetPixel(FI_AL_DWHS(img),img.PixelSize(),pPixelVal,img.PixelSize());
}
void    fiSetBoundaryPixel(FVTImage &img,const int bw,const void *pPixelVal)
{
	fiuSetBoundaryPixel(FI_AL_DWHS(img),img.PixelSize(),pPixelVal,img.PixelSize(),bw,bw,bw,bw);
}
//---------------------------------------------------------------------------------
void  fiFlip(const FVTImage& src,FVTImage& dest,const int axis)
{
	dest.ResetIf(src.Width(),src.Height(),src.Type());
	switch(axis)
	{
	case FI_AXIS_HORZ:
		fiuFlipX(src.Data(),src.LineSize(),src.Height(),src.Step(),dest.Data(),dest.Step());
		break;
	case FI_AXIS_VERT:
		fiuFlipY(src.Data(),src.Width(),src.Height(),src.Step(),src.PixelSize(),dest.Data(),dest.Step());
		break;
	default:
		assert(false);
	}
}

void  fiFlip(FVTImage& srcDest,const int axis)
{
	switch(axis)
	{
	case FI_AXIS_HORZ:
		fiuFlipX(srcDest.Data(),srcDest.LineSize(),srcDest.Height(),srcDest.Step());
		break;
	case FI_AXIS_VERT:
		fiuFlipY(srcDest.Data(),srcDest.Width(),srcDest.Height(),srcDest.Step(),srcDest.PixelSize());
		break;
	default:
		assert(false);
	}
}
//---------------------------------------------------------------------------------
void  fiTile(const FVTImage& src,FVTImage& dest,const int destWidth,const int destHeight)
{
	dest.ResetIf(destWidth,destHeight,src.Type());
	fiuTile(src.Data(),src.Width(),src.Height(),src.Step(),dest.Data(),dest.Width(),dest.Height(),dest.Step(),src.PixelSize());
}

//---------------------------------------------------------------------------------

void  fiTransformPixel(const FVTImage& src,FVTImage& dest,const int destDepth,const double scale,const double shift)
{
	FI_ASSERT_DEPTH(destDepth);

	dest.ResetIf(src.Width(),src.Height(),FI_MAKE_TYPE(destDepth,src.NChannels()));
	fiuTransformPixel(src.Data(),src.Width(),src.Height(),src.Step(),src.Type(),dest.Data(),dest.Step(),destDepth,scale,shift);
}

//---------------------------------------------------------------------------------

void  fiCastDepth(const FVTImage& src,FVTImage& dest,const int destDepth)
{
	fiTransformPixel(src,dest,destDepth,1,0);
}

//---------------------------------------------------------------------------------
void  fiSwapChannels(FVTImage& img,const int ic0,const int ic1)
{
	fiuSwapChannels(img.Data(),img.Width(),img.Height(),img.Step(),img.Type(),ic0,ic1);
}
void  fiCopyChannels(const FVTImage& src, const int icBeg,const int icEnd,
							FVTImage &dest,const int ocBeg)
{
	//dest.ResetIf(src.Width(),src.Height(),FI_MAKE_TYPE(src.Depth(),ocn));
	assert(fiSizeEq(src,dest)&&uint(dest.NChannels()-ocBeg)<=uint(icEnd-icBeg));
	
	fiuCopyChannels(src.Data(),src.Width(),src.Height(),src.Step(),src.Type(),icBeg,icEnd,
		dest.Data(),dest.Step(),dest.NChannels(),ocBeg);
}
void  fiGetChannel(const FVTImage& src,FVTImage& dest,const int ic)
{
	dest.ResetIf(src.Width(),src.Height(),FI_MAKE_TYPE(src.Depth(),1));
	fiCopyChannels(src,ic,ic+1,dest,0);
}

//---------------------------------------------------------------------------------

void  fiColorToGray(const FVTImage& src,FVTImage& dest,double w0,double w1,double w2)
{
	const int cn=src.NChannels();
	if(cn==1)
		dest=src;
	else
		if(cn==2)
			fiGetChannel(src,dest,0);
		else
		{
			double sum=w0+w1+w2;
			if(sum!=1&&sum!=0)
				w0/=sum,w1/=sum,w2/=sum;
			dest.ResetIf(src.Width(),src.Height(),FI_MAKE_TYPE(src.Depth(),1));
			fiuColorToGray(src.Data(),src.Width(),src.Height(),src.Step(),src.Type(),dest.Data(),dest.Step(),w0,w1,w2);
		}
}

//---------------------------------------------------------------------------------

void  fiConvertRGBChannels(const FVTImage &src,FVTImage &dest, const int ocn, double alpha)
{
	dest.ResetIf(src.Width(),src.Height(),FI_MAKE_TYPE(src.Depth(),ocn));

	return fiuConvertRGBChannels(src.Data(),src.Width(),src.Height(),src.Step(),src.Type(),dest.Data(),dest.Step(),ocn,alpha);
}

void _IFF_API fiResize(const FVTImage &src, FVTImage &dest, int dwidth, int dheight, int resampleMethod)
{
	if(!src.IsNull())
	{
		dest.ResetIf(dwidth,dheight,src.Type());

		fiuResize(FI_AL_DWHS(src),src.Type(), FI_AL_DWHS(dest), resampleMethod);
	}
}

void _IFF_API fiScale(const FVTImage &src, FVTImage &dest, double xscale, double yscale, int resampleMethod)
{
	fiResize(src,dest,int(src.Width()*xscale+0.5), int(src.Height()*yscale+0.5), resampleMethod);
}

_IFF_END

#if 0

#include "iff/ioimpl.h"

_IFF_BEG

void fiLoadImage(const char_t* file,FVTImage& dest)
{
	fiLoadImageImpl(file,dest);
}

void fiSaveImage(const char_t* file,const FVTImage& image)
{
	fiSaveImageImpl(file,image);
}

_IFF_END

#include "bfc/winx.h"

_IFF_BEG

class FIBitmap::_CImp
{
public:
	HDC      m_hdc;
	void    *m_pbits;
	int      m_width, m_height;

public:
	_CImp()
		:m_hdc(NULL), m_pbits(NULL), m_width(0), m_height(0)
	{
	}

	void Clear()
	{
		if(m_hdc)
		{
			wxDeleteBitmapDevice(m_hdc);
			m_hdc=NULL;
		}

		m_pbits=NULL;
		m_width=m_height=NULL;
	}

	void SetImage(const FVTImage &img, bool flip)
	{
		if(!img.IsNull())
		{
			void  *pbits=NULL;
			HDC hdc=wxCreateDeviceInDIB(img.Width(), img.Height(), &pbits,24);

			FVTImage dest(pbits,img.Width(),img.Height(),FI_8UC3,fiStep(img.Width(),FI_8UC3,4) );

			fiConvertRGBChannels(img,dest,3);

			if(flip)
				fiFlip(dest);

			this->Clear();

			m_hdc=hdc;
			m_pbits=pbits;
			m_width=img.Width();
			m_height=img.Height();
		}
		else
			this->Clear();

	}

	void Present(void *hdc, int dx, int dy, int ix, int iy, int cx, int cy)
	{
		if(m_hdc)
		{
			if(cx<0)
				cx=m_width;

			if(cy<0)
				cy=m_height;

			::BitBlt((HDC)hdc,dx,dy,cx,cy,m_hdc,ix,iy,SRCCOPY);
		}
	}

	~_CImp()
	{
		this->Clear();
	}
};

FIBitmap::FIBitmap()
:m_pimp(new _CImp)
{
}

FIBitmap::~FIBitmap()
{
	delete m_pimp;
}

void FIBitmap::SetImage(const ff::FVTImage &img, bool flip)
{
	m_pimp->SetImage(img,flip);
}

void FIBitmap::Present(void *hdc, int dx, int dy, int ix, int iy , int cx, int cy )
{
	m_pimp->Present(hdc,dx,dy,ix,iy,cx,cy);
}

int FIBitmap::GetWidth() const
{
	return m_pimp->m_width;
}

int FIBitmap::GetHeight() const
{
	return m_pimp->m_height;
}

void* FIBitmap::GetBitmapHDC() const
{
	return m_pimp->m_hdc;
}

void* FIBitmap::GetBitmapBits() const
{
	return m_pimp->m_pbits;
}

_IFF_END


#endif

