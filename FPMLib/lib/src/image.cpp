
//#include<limits>

#undef max
#undef min

#include "iff\image.h"
#include "iff\iff.h"

const int _FIF_AUTO_RELEASE=0x01;

#include"bfc\cfg.h"


_IFF_BEG

FVTImage::FVTImage(int width, int height, int type, int align )
:_pData(NULL),_flag(0)
{
	this->Reset(width,height,type,align);
}
FVTImage::FVTImage(const FVTImage& right)
:_pData(NULL),_flag(0)
{
	this->Reset(right.Width(),right.Height(),right.Type());
	fiCopy(right,*this);
}
FVTImage::~FVTImage() throw()
{
	if(!this->IsExternData())
		fiFree(_pData);
}
FVTImage& FVTImage::operator =(const FVTImage& right)
{
	if(this!=&right)
	{
		FVTImage temp(right);
		this->Swap(temp);
	}
	return *this;
}

#define _CHECK_DATA_LOCK(pImg) {\
	if((pImg)->DataLocked())\
		FF_ERROR(ERR_INVALID_OP,"Attempt to release locked image data!");}

#define _RELEASE_DATA_IF(pImg) {\
	if(!(pImg)->IsExternData()) fiFree((pImg)->_pData);}

void FVTImage::Lock(int mask)
{
	_flag|=(mask&0xFF00);
}
void FVTImage::Unlock(int mask)
{
	_flag&=~(mask&0xFF00);
}
void FVTImage::Clear()
{
	_CHECK_DATA_LOCK(this);
	_RELEASE_DATA_IF(this);
	_pData=NULL;
	_flag=_width=_height=_step=0;
}
void FVTImage::Swap(FVTImage &right)
{
	_CHECK_DATA_LOCK(this);
	_CHECK_DATA_LOCK(&right);

	char buf[sizeof(*this)];
	memcpy(buf,this,sizeof(*this));
	memcpy(this,&right,sizeof(*this));
	memcpy(&right,buf,sizeof(*this));
}
void FVTImage::Reshape(int newType, int newWidth)
{
	FI_ASSERT_TYPE(newType);
	
	int lsz=this->LineSize(),nlsz=0,ntsz=FI_TYPE_SIZE(newType);

	if(newWidth<=0)
	{
		newWidth=lsz/FI_TYPE_SIZE(newType);
		nlsz=lsz;
	}
	else
		nlsz=ntsz*newWidth;

	if(lsz==nlsz||lsz*_height%nlsz==0&&lsz==_step)
	{
		_type=newType,_width=newWidth;
		if(lsz!=nlsz)
			_height=lsz*_height/nlsz;
	}
	else
	{
		FF_EXCEPTION(ERR_INVALID_ARGUMENT,"");
	}
}
void FVTImage::Reset(int width, int height, int type, int align)
{
	FI_ASSERT_TYPE(type);
	FI_ASSERT_ALIGN(align);

	this->StepReset(width,height,type,fiStep(width,type,align));
}
void FVTImage::StepReset(int width, int height, int type, int step)
{
	FI_ASSERT_TYPE(type);
	FI_ASSERT_STEP(width,type,step);

	_CHECK_DATA_LOCK(this);

	uchar* pNewData=fiAllocImageData(step,height);
	_RELEASE_DATA_IF(this);
	_pData=pNewData;
	_width=width,_height=height,_step=step,_type=type;
	_flag=_FIF_AUTO_RELEASE;
}
void FVTImage::ResetIf(int width, int height, int type)
{
	if(width!=_width||height!=_height||type!=_type)
		this->Reset(width,height,type);
}
void FVTImage::Attach(void *pData, int width, int height,int type, int step)
{
	FI_ASSERT_TYPE(type);
	FI_ASSERT_STEP(width,type,step);

	_CHECK_DATA_LOCK(this);
	_RELEASE_DATA_IF(this);
	_pData=(uchar*)pData;
	_width=width,_height=height,_type=type,_step=step;
	_flag=0;
}
bool FVTImage::IsExternData() const
{
	return (_flag&_FIF_AUTO_RELEASE)==0;
}

void FVTImage::AttachROI(const FVTImage &img, const Rect &roi)
{
	if(this!=&img)
	{
		Rect roix(OverlappedRect(roi,Rect(0,0,img.Width(),img.Height())));

		if(roix.IsEmpty())
			this->Clear(); //clear if ROI is empty.           
		else
			this->Attach((void*)img.DataAs<uchar>(roix.X(),roix.Y()),roix.Width(),roix.Height(),img.Type(),img.Step());
	}
	else
	{
		//attach part of the data to the image itself, can't finished.
		if(roi!=Rect(0,0,img.Width(),img.Height()))
		{
			FF_ERROR(ERR_INVALID_OP,"Attach image to itself!");
		}
	}
}




_FF_END

