
#include"fpm.h"

#include<assert.h>
#include<float.h>

#include<algorithm>

IFPM::IFPM()
{
}
IFPM::~IFPM()
{
}


#include"fpmi.h"

template<typename _SrcT, typename _DestT>
void fpm_cd_imp(const char *pin, int width, int height, int istep, int istride,
				   char *pout, int ostep, int ostride
				   )
{
	if(istride<sizeof(_SrcT))
		istride=sizeof(_SrcT);

	if(ostride<sizeof(_DestT))
		ostride=sizeof(_DestT);

	for(int yi=0;yi<height;++yi,pin+=istep,pout+=ostep)
	{
		const char *pix=pin;
		char *pox=pout;

		for(int xi=0;xi<width;++xi,pix+=istride,pox+=ostride)
		{
			*(_DestT*)pox=_DestT(*(_SrcT*)pix);
		}
	}
}

void fpm_cvt_data(const void *pin, int width, int height, int istep, int istride, int itype,
				   void *pout, int ostep, int ostride, int otype
				   )
{
	assert(fpm_is_valid_type(itype)&&fpm_is_valid_type(otype));

	typedef void (*FuncT)(const char *pin, int width, int height, int istep, int istride,
				   char *pout, int ostep, int ostride
				   );

	typedef unsigned char uchar;

	static FuncT s_ftab[][4]=
	{
		{fpm_cd_imp<uchar,uchar>,fpm_cd_imp<uchar,int>,fpm_cd_imp<uchar,float>,fpm_cd_imp<uchar,double>},

		{fpm_cd_imp<int,uchar> , fpm_cd_imp<int,int>,  fpm_cd_imp<int, float>, fpm_cd_imp<int, double>},

		{fpm_cd_imp<float,uchar>,fpm_cd_imp<float,int>,fpm_cd_imp<float,float>, fpm_cd_imp<float,double>},

		{fpm_cd_imp<double,uchar>,fpm_cd_imp<double,int>,fpm_cd_imp<double,float>,fpm_cd_imp<double,double>}
	};

	if(pin&&pout)
	{
		(s_ftab[itype-1][otype-1])((char*)pin,width,height,istep,istride,(char*)pout,ostep,ostride);
	}
}


int fpm_ssd_select(const double *pssd, const unsigned char *pmask, int count, int *pmatch, int nmax, double ss)
{
	int ret=0;

	if(!pmask)
	{
		int mid=(int)(std::max_element(pssd,pssd+count)-pssd);

		if(ss<1e-6||nmax<=1)
		{
			pmatch[0]=mid; ret=1;
		}
		else
		{
			double merr=pssd[mid]+ss;

			for(int i=0;i<count;++i)
			{
				if(pssd[i]<merr)
				{
					pmatch[ret]=i;

					if(++ret>=nmax)
						break;
				}
			}
		}
	}
	else
	{
		int mid=-1;
		double merr=FLT_MAX;

		for(int i=0;i<count;++i)
		{
			if(pmask[i]&&pssd[i]<merr)
			{
				mid=i;
				merr=pssd[i];
			}
		}

		if(ss<1e-6||nmax<=1)
		{
			pmatch[0]=mid; ret=1;
		}
		else
		{
			merr+=ss;

			for(int i=0;i<count;++i)
			{
				if(pmask[i]&&pssd[i]<merr)
				{
					pmatch[ret]=i;

					if(++ret>=nmax)
						break;
				}
			}
		}
	}
	return ret;
}

void fpm_set_mask(int iw,int ih,int pw,int ph, const uchar *mask, int mstep, FVTImage &dest, uchar mval)
{
	dest.Reset(iw,ih,FI_8UC1,1);
	assert(dest.Step()==iw);

	if(mask)
		fiuCopy(mask,iw,ih,mstep,FI_AL_DS(dest));
	else
		fiSetMem(dest,mval);
}

void fpm_set_mask(int iw,int ih,const uchar *mask, int mstep, FVTImage &dest, uchar mval)
{
	dest.Reset(iw,ih,FI_8UC1,1);
	assert(dest.Step()==iw);

	if(mask)
		fiuCopy(mask,iw,ih,mstep,FI_AL_DS(dest));
	else
		fiSetMem(dest,mval);
}


int fpm_type_size(int type)
{
	static const int tsz[]={1,4,4,8};

	if(type>FPMT_BEG&&type<FPMT_END)
		return tsz[type-1];

	return 0;
}

//======================================================================================

template<int cn,typename _TValT,typename _OValT>
class IOPThreshold
{
	const _TValT m_threshold;
	const _OValT m_val0,m_val1;
public:
	IOPThreshold(const _TValT threshold,const _OValT val0,const _OValT val1)
		:m_threshold(threshold),m_val0(val0),m_val1(val1)
	{
	}
	template<typename _IPValT,typename _OPValT>
	void operator()(const _IPValT *pix,_OPValT *pox) const
	{
		for(int ci=0;ci<cn;++ci)
			pox[ci]=(_OPValT)(pix[ci]<m_threshold? m_val0:m_val1);
	}
};

void FPMMakePatMask(int iwidth, int iheight, int pwidth, int pheight, const unsigned char *pPixelMask, int xmstep,
					unsigned char *pPatMask, int pmstep,unsigned char mval
					)
{
	if(pPixelMask)
	{
		FVTImage pmask(iwidth,iheight,FI_8UC1);

		IOPThreshold<1,uchar,uchar> tmp(1,0,1);
		for_each_pixel_1_1(pPixelMask,iwidth,iheight,xmstep,1,FI_AL_DSP(pmask), tmp);

		FVTImage intbuf(iwidth+1,iheight+1,FI_32SC1),inti;
		inti.AttachROI(intbuf,Rect(1,1,iwidth,iheight));

		IntegralImage<1>(FI_AL_DWHSP(pmask),FI_AL_DSP_AS(inti,int),true);

		FVTImage np(iwidth,iheight,FI_32SC1);

		fiSetMem(np,0);
		GetIntegralValue<1>(FI_AL_DWHSP_AS(inti,int),FI_AL_DSP_AS(np,int),pwidth,pheight);

		IOPThreshold<1,int,uchar> tmp2(pwidth*pheight,0,mval);
		for_each_pixel_1_1(np.DataAs<int>(), iwidth, iheight, iwidth, 1, pPatMask, pmstep,1, tmp2);
	}
	else
	{
		FVTImage pmask(pPatMask,iwidth,iheight,FI_8UC1,pmstep);

		fpm_set_mask(iwidth,iheight,pwidth,pheight,NULL,0,pmask,mval);
	}
}
