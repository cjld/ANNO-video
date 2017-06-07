
#include<memory.h>
#include<algorithm>

#include"fpm.h"

#include"fftw3.h"

#include"fpmi.h"

#include"fvtif.h"

#include FVT_FITYPE_H
#include FVT_FIUTIL_H
#include FVT_FICORE_H

using namespace fvt;

typedef fftw_complex  cdata_t;

struct fft_data
{
	fftw_plan p1,p2;

	double    *prin;
	cdata_t   *prout,*parr;

	double    *pinx;
	cdata_t  *poutx;
	
	int		 w,W,h,H;
};

#if 1

fft_data* fft_create(int w, int W, int h, int H, double *pin, cdata_t *pout)
{
	fft_data *pfft=new fft_data;
	memset(pfft,0,sizeof(*pfft));

//	pfft->prin=(double*)fftw_malloc(H*sizeof(double));
//	memset(pfft->prin,0,sizeof(double)*H);

	pfft->prin=(double*)fftw_malloc(H*sizeof(double));
	memset(pfft->prin,0,sizeof(double)*H);

	pfft->prout=(cdata_t*)fftw_malloc(H*sizeof(cdata_t));

	pfft->parr=(cdata_t*)fftw_malloc(W*H*sizeof(cdata_t));
	memset(pfft->parr,0,W*H*sizeof(cdata_t));

//	int n1[]={H,w};

//	pfft->p1=fftw_plan_many_dft_r2c(1,n1,w,pin,NULL,1,H,pfft->parr,NULL,W,1,FFTW_MEASURE|FFTW_PRESERVE_INPUT);

	pfft->p1=fftw_plan_dft_r2c_1d(H,pfft->prin,pfft->prout,FFTW_MEASURE);
	
	//pfft->p1=fftw_plan_dft_1d(H,pfft->prin,pfft->prout,FFTW_FORWARD,FFTW_MEASURE|FFTW_PRESERVE_INPUT);

	int n2[]={W,H};

	pfft->p2=fftw_plan_many_dft(1,n2,H/2+1,pfft->parr,NULL,1,W,pout,NULL,1,W,FFTW_FORWARD,FFTW_MEASURE|FFTW_PRESERVE_INPUT);

	pfft->pinx=pin;
	pfft->poutx=pout;

	pfft->w=w,pfft->W=W,pfft->h=h,pfft->H=H;

	return pfft;
}

void fft_free(fft_data *pfft)
{
	if(pfft)
	{
		fftw_destroy_plan(pfft->p1);
		fftw_destroy_plan(pfft->p2);

		fftw_free(pfft->parr);
		fftw_free(pfft->prin);
		fftw_free(pfft->prout);

		delete pfft;
	}
}

template<int csz>
void scopy(const void *pin, int count, int istride, void *pout, int ostride)
{
	const char *pinx=(char*)pin;
	char *poutx=(char*)pout;

	for(int i=0;i<count;++i,pinx+=istride,poutx+=ostride)
		memcpy(poutx,pinx,csz);
}

void fft_exec(fft_data *pfft)
{
//	fftw_execute(pfft->p1);
	const int w=pfft->w;
	double *pin=pfft->pinx;

	for(int i=0;i<w;++i,++pin)
	{
		scopy<sizeof(double)>(pfft->pinx+i,pfft->h,sizeof(double)*pfft->w,pfft->prin,sizeof(double));

		fftw_execute(pfft->p1);

		scopy<sizeof(cdata_t)>(pfft->prout,pfft->H,sizeof(cdata_t),pfft->parr+i,sizeof(cdata_t)*pfft->W);
	}

	fftw_execute(pfft->p2);
}
#else


fft_data* fft_create(int w, int W, int h, int H, double *pin, cdata_t *pout)
{
	fft_data *pfft=new fft_data;
	memset(pfft,0,sizeof(*pfft));

	pfft->prin=(double*)fftw_malloc(H*sizeof(double));
	memset(pfft->prin,0,sizeof(double)*H);

	pfft->prout=(cdata_t*)fftw_malloc(H*sizeof(cdata_t));

	pfft->parr=(cdata_t*)fftw_malloc(W*H*sizeof(cdata_t));
	memset(pfft->parr,0,W*H*sizeof(cdata_t));

	int n1[]={H,w};

	pfft->p1=fftw_plan_many_dft_r2c(1,n1,w,pin,NULL,1,H,pfft->parr,NULL,W,1,FFTW_MEASURE|FFTW_PRESERVE_INPUT);

	int n2[]={W,H};

	pfft->p2=fftw_plan_many_dft(1,n2,H/2+1,pfft->parr,NULL,1,W,pout,NULL,1,W,FFTW_FORWARD,FFTW_MEASURE|FFTW_PRESERVE_INPUT);

	pfft->pinx=pin;
	pfft->poutx=pout;

	pfft->w=w,pfft->W=W,pfft->h=h,pfft->H=H;

	return pfft;
}

void fft_free(fft_data *pfft)
{
	if(pfft)
	{
		fftw_destroy_plan(pfft->p1);
		fftw_destroy_plan(pfft->p2);

		fftw_free(pfft->parr);
		fftw_free(pfft->prin);
		fftw_free(pfft->prout);

		delete pfft;
	}
}

template<int csz>
void scopy(const void *pin, int count, int istride, void *pout, int ostride)
{
	const char *pinx=(char*)pin;
	char *poutx=(char*)pout;

	for(int i=0;i<count;++i,pinx+=istride,poutx+=ostride)
		memcpy(poutx,pinx,csz);
}

void fft_exec(fft_data *pfft)
{
	fftw_execute(pfft->p1);

	fftw_execute(pfft->p2);
}

#endif

#if 1

class pat_fft
{
public:
	fft_data *m_pat_fft;
	double    *m_pat_in;
	cdata_t  *m_pat_out;

	int m_w,m_W,m_h,m_H;
public:

	pat_fft()
	{
		memset(this,0,sizeof(*this));
	}

	~pat_fft()
	{
		fft_free(m_pat_fft);
		fftw_free(m_pat_in);
		fftw_free(m_pat_out);
	}

	void reset(int w,int W,int h,int H)
	{
		m_pat_in=(double*)fftw_malloc(sizeof(double)*w*h);
		memset(m_pat_in,0,sizeof(double)*w*h);

		m_pat_out=(cdata_t*)fftw_malloc(sizeof(cdata_t)*W*H);

		m_pat_fft=fft_create(w,W,h,H,m_pat_in,m_pat_out);

		m_w=w, m_h=h, m_W=W, m_H=H;
	}

	const cdata_t* exec(const void *pat, int step, int stride, int type)
	{
		fpm_cvt_data(pat, m_w, m_h, step, stride, type, m_pat_in, sizeof(double)*m_w, sizeof(double), FPMT_64F);

		fft_exec(m_pat_fft);

		return m_pat_out;
	}
};

#else

class pat_fft
{
public:
	fft_data *m_pat_fft;
	double    *m_pat_in;
	cdata_t  *m_pat_out;

	int m_w,m_W,m_h,m_H;
public:

	pat_fft()
	{
		memset(this,0,sizeof(*this));
	}

	~pat_fft()
	{
		fft_free(m_pat_fft);
		fftw_free(m_pat_in);
		fftw_free(m_pat_out);
	}

	void reset(int w,int W,int h,int H)
	{
		m_pat_in=(double*)fftw_malloc(sizeof(double)*w*H);
		memset(m_pat_in,0,sizeof(double)*w*H);

		m_pat_out=(cdata_t*)fftw_malloc(sizeof(cdata_t)*W*H);

		m_pat_fft=fft_create(w,W,h,H,m_pat_in,m_pat_out);

		m_w=w, m_h=h, m_W=W, m_H=H;
	}

	const cdata_t* exec(const void *pat, int step, int stride, int type)
	{
		fpm_cvt_data(pat, m_h, m_w, step, stride, type, m_pat_in, sizeof(double)*m_H, sizeof(double), FPMT_64F);

		fft_exec(m_pat_fft);

		return m_pat_out;
	}
};

#endif

class img_fft
{
public:
	fftw_plan m_plan;

	cdata_t     *m_in;
	cdata_t     *m_out;

	int        m_width, m_height;

public:
	img_fft()
	{
		memset(this,0,sizeof(*this));
	}
	~img_fft()
	{
		fftw_destroy_plan(m_plan);

		fftw_free(m_in);
		fftw_free(m_out);
	}
	void reset(int width,int height)
	{
		m_in=(cdata_t*)fftw_malloc(width*height*sizeof(cdata_t));
		m_out=(cdata_t*)fftw_malloc(width*height*sizeof(cdata_t));

		m_plan=fftw_plan_dft_2d(width,height,m_in,m_out,FFTW_FORWARD,FFTW_MEASURE);

		m_width=width; m_height=height;
	}
	const cdata_t* exec(const void *pat, int step, int stride, int type)
	{
		fpm_cvt_data(pat,m_width,m_height,step,stride,type,m_in,sizeof(cdata_t)*m_width,sizeof(cdata_t),FPMT_64F);

		fftw_execute(m_plan);

		return m_out;
	}
};

#if 0
class img_rfft
{
public:
	fftw_plan m_plan;

	cdata_t     *m_in;
	cdata_t     *m_out;

	int        m_width, m_height;

public:
	img_rfft()
	{
		memset(this,0,sizeof(*this));
	}
	~img_rfft()
	{
		fftw_destroy_plan(m_plan);

		fftw_free(m_in);
		fftw_free(m_out);
	}
	void reset(int width,int height)
	{
		m_in=(cdata_t*)fftw_malloc(width*height*sizeof(cdata_t));
		m_out=(cdata_t*)fftw_malloc(width*height*sizeof(cdata_t));

		m_plan=fftw_plan_dft_2d(width,height,m_in,m_out,FFTW_BACKWARD,FFTW_MEASURE);

		m_width=width; m_height=height;
	}
	cdata_t* in_buf() const
	{
		return m_in;
	}
	const cdata_t* exec()
	{
		fftw_execute(m_plan);

		return m_out;
	}
};
#else

class img_rfft
{
public:
	fftw_plan m_plan;

	cdata_t     *m_in;
	double     *m_out;

	int        m_width, m_height;

public:
	img_rfft()
	{
		memset(this,0,sizeof(*this));
	}
	~img_rfft()
	{
		fftw_destroy_plan(m_plan);

		fftw_free(m_in);
		fftw_free(m_out);
	}
	void reset(int width,int height)
	{
		m_in=(cdata_t*)fftw_malloc(width*height*sizeof(cdata_t));
		m_out=(double*)fftw_malloc(width*height*sizeof(double));

		m_plan=fftw_plan_dft_c2r_2d(width,height,m_in,m_out,FFTW_MEASURE);

		m_width=width; m_height=height;
	}
	cdata_t* in_buf() const
	{
		return m_in;
	}
	const double* exec()
	{
		fftw_execute(m_plan);

		return m_out;
	}
};

#endif

# if 1

static void _calc_spec_conv(const double *pws, const double *pi2s, const double *pps, const double *pis, double *pdest, int count)
{
//	double scale=1.0/count;
//	for(int i=0;i<count;++i,pws+=2,pi2s+=2,pps+=2,pis+=2,pdest+=2)

	for(int yi=0;yi<512;++yi)
	{
		for(int xi=0;xi<=256;++xi,pws+=2,pi2s+=2,pps+=2,pis+=2,pdest+=2)
		{
			pdest[0]=(pws[0]*pi2s[0]+pws[1]*pi2s[1]-pps[0]*pis[0]-pps[1]*pis[1]);

			pdest[1]=(pws[0]*pi2s[1]-pws[1]*pi2s[0]-pps[0]*pis[1]+pps[1]*pis[0]);
		}
		pws+=510,pi2s+=510,pps+=510,pis+=510;
	}
}
//#else

static void _calc_spec_conv(const double *pws, const double *pi2s, const double *pps, const double *pis,double *pbuf, double *pdest, int width, int height)
{
	const int hcount=(height/2+1)*width;
	double *pbx=pbuf;

	for(int i=0;i<hcount;++i,pws+=2,pi2s+=2,pps+=2,pis+=2,pbx+=2)
	{
		pbx[0]=(pws[0]*pi2s[0]+pws[1]*pi2s[1]-pps[0]*pis[0]-pps[1]*pis[1]);

		pbx[1]=(pws[0]*pi2s[1]-pws[1]*pi2s[0]-pps[0]*pis[1]+pps[1]*pis[0]);
	}

	const int hw=width/2+1,hh=height/2+1;

	double *pdx=pdest;
	int     yi=0;
	pbx=pbuf;

	for(;yi<hh;++yi,pdx+=(hw<<1),pbx+=(width<<1))
	{
		memcpy(pdx,pbx,2*sizeof(double)*hw);
	}

	pbx=pbuf+((height-1)/2*width)*2;

	for(;yi<height;++yi,pbx-=(width<<1))
	{
		pdx[0]=pbx[0], pdx[1]=-pbx[1];
		pdx+=2;

		const double *pbxx=pbx+((width-1)<<1);

		for(int xi=1;xi<hw;++xi,pbxx-=2,pdx+=2)
		{
			pdx[0]=pbxx[0], pdx[1]=-pbxx[1];
		}
	}

}

#endif

template<typename _ValT>
inline bool fpm_is_sym(const _ValT *pcd, int width, int height, int stride=0,_ValT err=1)
{
	if(stride==0)
		stride=2;

	for(int xi=1;xi<width;++xi)
	{
		const _ValT *p0=pcd+xi*stride;
		const _ValT *p1=pcd+(width-xi)*stride;

		if(fabs(p0[0]-p1[0])>err||fabs(p0[1]+p1[1])>err)
			return false;
	}

	for(int xi=1;xi<height;++xi)
	{
		const _ValT *p0=pcd+xi*width*stride;
		const _ValT *p1=pcd+(height-xi)*width*stride;

		if(fabs(p0[0]-p1[0])>err||fabs(p0[1]+p1[1])>err)
			return false;
	}

	for(int yi=1;yi<height;++yi)
	{
		for(int xi=1;xi<width;++xi)
		{
			const _ValT *p0=pcd+(yi*width+xi)*stride;
			const _ValT *p1=pcd+((height-yi)*width+(width-xi))*stride;

			if(fabs(p0[0]-p1[0])>err||fabs(p0[1]+p1[1])>err)
				return false;
		}
	}
	return true;
}

class FFT_FPM::_CImp
{
	pat_fft  m_pfft, m_wfft;
	img_fft  m_ifft;
	img_rfft m_rfft;

	int		 m_iw,m_ih,m_pw,m_ph, m_flag;
	double    m_sel;

	FVTImage m_ispec, m_i2spec, m_rbuf;

	FVTImage m_i2int;

	FVTImage m_mask;

	std::vector<double>  m_wpat;

public:
	void Plan(int iwidth, int iheight, int pwidth, int pheight, int flag, double sel)
	{
		m_pfft.reset(pwidth,iwidth,pheight,iheight);

		m_ifft.reset(iwidth,iheight);

		m_rfft.reset(iwidth,iheight);

		if(flag&FPMF_WEIGHTED)
			m_wfft.reset(pwidth,iwidth,pheight,iheight);

		m_iw=iwidth,m_ih=iheight,m_pw=pwidth,m_ph=pheight, m_flag=flag;
		m_sel=sel<1.0f? 1.0f:sel;

		m_wpat.resize(m_pw*m_ph);
	}

	void SetImage(const void *img, int istep,int istride, int itype, const unsigned char *mask, int mstep)
	{
		FVTImage ix(m_iw,m_ih,FI_32FC2),ix2(m_iw,m_ih,FI_32FC2);

		fpm_cvt_data(img,m_iw,m_ih,istep,istride,itype,FI_AL_DS(ix),sizeof(double),FPMT_64F);

		double scale=1.0/(m_iw*m_ih);

		{
			fiuForPixels_1_1(FI_AL_DWHSP_AS(ix,double),FI_AL_DSP_AS(ix2,double),IOPScale<1,double>(2.0f*scale));
			
			const cdata_t *pres=m_ifft.exec(ix2.Data(),ix2.Step(),sizeof(double),FPMT_64F);

			m_ispec.Reset(m_iw,m_ih,FI_32FC4,sizeof(double));
			fiuCopy(pres,sizeof(cdata_t)*m_iw,m_ih,sizeof(cdata_t)*m_iw,FI_AL_DS(m_ispec));
		}

		if(m_flag&FPMF_WEIGHTED)
		{
			fiuForPixels_2_1(FI_AL_DWHSP_AS(ix,double),FI_AL_DSP_AS(ix,double),FI_AL_DSP_AS(ix2,double),IOPMul<1>()); //calc square

			fiuForPixels_0_1(FI_AL_DWHSP_AS(ix2,double),InplaceI0(IOPScale<1,double>(scale)));

			const cdata_t *pres=m_ifft.exec(ix2.Data(),ix2.Step(),ix2.PixelSize(),FPMT_64F);

			m_i2spec.Reset(m_iw,m_ih,FI_32FC4,sizeof(double));
			fiuCopy(pres,sizeof(cdata_t)*m_iw,m_ih,sizeof(cdata_t)*m_iw,FI_AL_DS(m_i2spec));
		}
		else
		{
			//else integrate i^2, not implemented yet!
		}

		m_mask.Reset(m_iw,m_ih,FI_8UC1,1);
		assert(m_mask.Step()==m_iw);

		if(mask)
			fiuCopy(mask,m_iw,m_ih,mstep,FI_AL_DS(m_mask));
		else
			fiSetMem(m_mask,1);

		uchar mv=0;
		fiuForBoundaryPixels_0_1(FI_AL_DWHSP(m_mask),BindI0(&mv,IOPCopy<1>()),0,m_ph,0,m_pw);

		m_rbuf.Reset(m_iw,m_ih,FI_32FC4);
	}

	const double *GetSSD(const void *pat, int pstep, int pstride, int ptype, const double *weight, int flag)
	{
		double dssd=0;

		fpm_cvt_data(pat,m_pw,m_ph,pstep,pstride,ptype,&m_wpat[0],sizeof(double)*m_pw,sizeof(double),FPMT_64F);

		int np=m_pw*m_ph;

		if(flag&FPMF_TRUE_SSD)
		{
			for(int i=0;i<np;++i)
				dssd+=m_wpat[i]*m_wpat[i]*weight[i];
		}

		for(int i=0;i<np;++i)
			m_wpat[i]*=weight[i];

		const cdata_t *ppspec=m_pfft.exec(&m_wpat[0],sizeof(double)*m_pw,sizeof(double),FPMT_64F);
		const double   *pwssd=NULL;

		//{
		//	pat_fft_x  pfft;
		//	pfft.reset(m_pw,m_iw,m_ph,m_ih);

		//	const cdata_t *psx=pfft.exec(&m_wpat[0],sizeof(double)*m_pw,sizeof(double),FPMT_64F);

		//	for(int i=0;i<m_iw*m_ih;++i)
		//	{
		//		if(fabs(ppspec[i][0]-psx[i][0])>1||fabs(ppspec[i][1]-psx[i][1])>1)
		//		{
		//			const cdata_t *xx=ppspec+i,*yy=psx+i;

		//			int j=0;
		//		}
		//	}
		//}

		if(m_flag&FPMF_WEIGHTED)
		{
			assert(weight);

			const cdata_t *pwspec=m_wfft.exec(weight,sizeof(double)*m_pw,sizeof(double),FPMT_64F);

		//	bool bsym=fpm_is_sym((double*)pwspec,m_iw,m_ih);

		/*	bsym=fpm_is_sym((double*)ppspec,m_iw,m_ih);

			bsym=fpm_is_sym(m_i2spec.DataAs<double>(),m_iw,m_ih);

			bsym=fpm_is_sym(m_ispec.DataAs<double>(),m_iw,m_ih);*/

		//	_calc_spec_conv((double*)pwspec,m_i2spec.DataAs<double>(),(double*)ppspec,m_ispec.DataAs<double>(),(double*)m_rfft.in_buf(),m_iw*m_ih);

			_calc_spec_conv((double*)pwspec,m_i2spec.DataAs<double>(),(double*)ppspec,m_ispec.DataAs<double>(),m_rbuf.DataAs<double>(),(double*)m_rfft.in_buf(),m_iw,m_ih);

	//		bsym=fpm_is_sym((double*)m_rfft.in_buf(),m_iw,m_ih);

			pwssd=(double*)m_rfft.exec();
		}
		else
		{
			//...
		}

	//	if(dssd!=0)
		{
			np=m_iw*m_ih;
			double *pssdx=(double*)pwssd;

			for(int i=0;i<np;++i)
			{
			//	pssdx[i]=(pssdx[2*i]+dssd);
				pssdx[i]+=dssd;
				pssdx[i]=sqrt(pssdx[i]);
			}
		}

		return (double*)pwssd;
	}

	int Match(const void *pat, int pstep, int pstride, int ptype, const double *weight, int *pmatch,int nmax)
	{
		const double *pssd=this->GetSSD(pat,pstep,pstride,ptype,weight,FPMF_TRUE_SSD);

		return fpm_ssd_select(pssd,m_mask.Data(),m_iw*m_ih,pmatch,nmax,m_sel);
	}
};


//====================================================================================================================

FFT_FPM::FFT_FPM()
:m_pImp(new _CImp)
{
}

FFT_FPM::~FFT_FPM()
{
	delete m_pImp;
}

const double *FFT_FPM::GetSSD(const void *pat, int pstep, int pstride, int ptype, const double *weight, int flag)
{
	return m_pImp->GetSSD(pat,pstep,pstride,ptype,weight,flag);
}

void FFT_FPM::Plan(int iwidth, int iheight, int pwidth, int pheight, int flag, double sel)
{
	m_pImp->Plan(iwidth,iheight,pwidth,pheight,flag,sel);
}

void FFT_FPM::SetImage(const void *img, int istep, int istride, int itype, const unsigned char *mask, int mstep)
{
	m_pImp->SetImage(img,istep,istride,itype,mask,mstep);
}

int FFT_FPM::Match(const void *pat, int pstep, int pstride, int ptype, const double *weight, int *pmatch,int nmax)
{
	return m_pImp->Match(pat,pstep,pstride,ptype,weight,pmatch,nmax);
}







