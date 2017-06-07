
#include<memory.h>
#include<algorithm>

#include"fpm.h"

#include"fftw3.h"

#include"fpmi.h"

//#define USE_IPF_STATIC_LIB
//#define USE_IFF_LIB
//#define USE_FFS_LIB
//#define USE_BFC_LIB
//#include"lib1.h"


//=======================================================

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

fft_data* fft_create(int w, int W, int h, int H, double *pin, cdata_t *pout)
{
	fft_data *pfft=new fft_data;
	memset(pfft,0,sizeof(*pfft));

	pfft->prin=(double*)fftw_malloc(H*sizeof(double));
	memset(pfft->prin,0,sizeof(double)*H);

	pfft->prout=(cdata_t*)fftw_malloc(H*sizeof(cdata_t));

	int hH=H/2+1;

	pfft->parr=(cdata_t*)fftw_malloc(W*hH*sizeof(cdata_t));
	memset(pfft->parr,0,W*hH*sizeof(cdata_t));

	pfft->p1=fftw_plan_dft_r2c_1d(H,pfft->prin,pfft->prout,FFTW_MEASURE);
	
	int n2[]={W,H};

	pfft->p2=fftw_plan_many_dft(1,n2,hH,pfft->parr,NULL,1,W,pout,NULL,1,W,FFTW_FORWARD,FFTW_MEASURE|FFTW_PRESERVE_INPUT);

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
	const int w=pfft->w;
	double *pin=pfft->pinx;

	for(int i=0;i<w;++i,++pin)
	{
		scopy<sizeof(double)>(pfft->pinx+i,pfft->h,sizeof(double)*pfft->w,pfft->prin,sizeof(double));

		fftw_execute(pfft->p1);

		scopy<sizeof(cdata_t)>(pfft->prout,pfft->H/2+1,sizeof(cdata_t),pfft->parr+i,sizeof(cdata_t)*pfft->W);
	}

	fftw_execute(pfft->p2);
}

//============================================

class pat_fft
{
private:
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

		m_pat_out=(cdata_t*)fftw_malloc(sizeof(cdata_t)*W*(H/2+1));

		m_pat_fft=fft_create(w,W,h,H,m_pat_in,m_pat_out);

		m_w=w, m_h=h, m_W=W, m_H=H;
	}

	double* in_buf() const
	{
		return m_pat_in;
	}

	const cdata_t* exec()
	{
		fft_exec(m_pat_fft);

		return m_pat_out;
	}
};


class img_fft
{
private:
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

		m_plan=fftw_plan_dft_2d(height,width,m_in,m_out,FFTW_FORWARD,FFTW_MEASURE);

		m_width=width; m_height=height;
	}
	const cdata_t* exec(const void *pat, int step, int stride, int type)
	{
		fpm_cvt_data(pat,m_width,m_height,step,stride,type,m_in,sizeof(cdata_t)*m_width,sizeof(cdata_t),FPMT_64F);

		fftw_execute(m_plan);

		return m_out;
	}
};

class img_rfft
{
private:
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
		m_in=(cdata_t*)fftw_malloc((width/2+1)*height*sizeof(cdata_t));
		m_out=(double*)fftw_malloc(width*height*sizeof(double));

		m_plan=fftw_plan_dft_c2r_2d(height,width,m_in,m_out,FFTW_MEASURE);

		m_width=width; m_height=height;
	}
	cdata_t* in_buf() const
	{
		return m_in;
	}
	double * out_buf() const
	{
		return m_out;
	}
	const double* exec()
	{
		fftw_execute(m_plan);

		return m_out;
	}
};

static void _mul_conj(const double *pca, const double *pcb, int count, double *pdest)
{
	for(int i=0;i<count;++i,pca+=2,pcb+=2,pdest+=2)
	{
		pdest[0]=pca[0]*pcb[0]+pca[1]*pcb[1];
		pdest[1]=pca[0]*pcb[1]-pca[1]*pcb[0];
	}
}

static void _add_mul_conj(const double *pca, const double *pcb, int count, double *pdest)
{
	for(int i=0;i<count;++i,pca+=2,pcb+=2,pdest+=2)
	{
		pdest[0]+=pca[0]*pcb[0]+pca[1]*pcb[1];
		pdest[1]+=pca[0]*pcb[1]-pca[1]*pcb[0];
	}
}


static void _calc_spec_conv(const double *pws, const double *pi2s, const double *pps,double *pdest,double *pbuf, int width, int height)
{
	const int hcount=(height/2+1)*width;
	double *pbx=pbuf;

	for(int i=0;i<hcount;++i,pws+=2,pi2s+=2,pps+=2,pbx+=2)
	{
		pbx[0]=(pws[0]*pi2s[0]+pws[1]*pi2s[1]-pps[0]);

		pbx[1]=(pws[0]*pi2s[1]-pws[1]*pi2s[0]-pps[1]);
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

static void _calc_spec_conv(const double *pps,double *pdest, int width, int height)
{
	const int hcount=(height/2+1)*width;
	const double *pbx=pps;

	const int hw=width/2+1,hh=height/2+1;

	double *pdx=pdest;
	int     yi=0;
	pbx=pps;

	for(;yi<hh;++yi,pdx+=(hw<<1),pbx+=(width<<1))
	{
		memcpy(pdx,pbx,2*sizeof(double)*hw);
	}

	pbx=pps+((height-1)/2*width)*2;

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

class IOPVSquare
{
	const int m_dim;
public:
	IOPVSquare(int dim)
		:m_dim(dim)
	{
	}
	template<typename _SrcT,typename _DestT>
	void operator()(const _SrcT *pin, _DestT *pout)
	{
		_DestT s=0;

		for(int i=0;i<m_dim;++i)
		{
			s+=pin[i]*pin[i];
		}
		*pout=s;
	}

};


class FPM_FFT::_CImp
{
public:
	pat_fft  m_pfft;
	img_rfft m_rfft;

	int		 m_iw,m_ih,m_pw,m_ph, m_dim, m_flag;
	double    m_sel;

	std::vector<FVTImage> m_vispec;

	FVTImage  m_i2spec, m_i2int, m_i2roi,m_rfft_roi;

	FVTImage m_mask0; //user specified mask
	FVTImage m_mask;  //mask without border

	std::vector<double>  m_wpat;
	FVTImage m_rbuf, m_wpbuf;
	
public:
	void PlanPatch(int pwidth, int pheight)
	{
		if(pwidth!=m_pw || pheight!=m_ph)
		{
			m_pfft.reset(pwidth,m_iw,pheight,m_ih);			
			m_wpat.resize(pwidth*pheight*m_dim);

			uchar mv=0;
			m_mask=m_mask0;
			for_boundary_pixel_0_1(FI_AL_DWHSP(m_mask),bind_i0(&mv,iop_copy<1>()),0,pheight,0,pwidth);
			
			m_pw=pwidth; m_ph=pheight;
		}
	}
	void Plan(int iwidth, int iheight, int dim, int flag, double sel)
	{
	//	m_pfft.reset(pwidth,iwidth,pheight,iheight);
		m_rfft.reset(iwidth,iheight);

		m_iw=iwidth,m_ih=iheight,m_dim=dim, m_flag=flag;
		m_sel=sel;

	//	m_wpat.resize(m_pw*m_ph*m_dim);
		m_rbuf.Reset(m_iw,m_ih/2+1,FI_32FC4);
		m_wpbuf.Reset(m_iw,m_ih/2+1,FI_32FC4);

		m_vispec.resize(dim);

		for(int i=0;i<dim;++i)
		{
			m_vispec[i].Reset(m_iw,(m_ih/2+1),FI_32FC4,sizeof(double));
		}

		m_pw=0; m_ph=0;
	}

	void SetImage(const void *img, int istep, int itype)
	{
		int np=m_iw*m_ih;

		double *pix=new double[np*m_dim], *pix2=new double[np];

		fpm_cvt_data(img,m_iw*m_dim,m_ih,istep,0,itype,pix,sizeof(double)*m_dim*m_iw,0,FPMT_64F);

		img_fft  m_ifft;

		m_ifft.reset(m_iw,m_ih);

		double scale=1.0/(m_iw*m_ih);

		//fiuForPixels_1_1(pix,m_iw,m_ih,sizeof(double)*m_dim*m_iw,sizeof(double)*m_dim,pix2,sizeof(double)*m_iw,sizeof(double),IOPVSquare(m_dim));
		ff::for_each_pixel_1_1(pix,m_iw,m_ih,m_dim*m_iw,m_dim,pix2,m_iw,1,IOPVSquare(m_dim));

		if(m_flag&FPMF_WEIGHTED)
		{
			for_each_pixel_0_1(pix2,m_iw,m_ih,m_iw,1,inplace_i0(iop_scale<1,double>(scale)));

			const cdata_t *pres=m_ifft.exec(pix2,sizeof(double)*m_iw,sizeof(double),FPMT_64F);

			m_i2spec.Reset(m_iw,m_ih/2+1,FI_32FC4,sizeof(double));
			fiuCopy(pres,sizeof(cdata_t)*m_iw,m_ih/2+1,sizeof(cdata_t)*m_iw,FI_AL_DS(m_i2spec));
		}
#if 0 //not supported for dynamic path size
		else
		{
			FVTImage buf(m_iw+1,m_ih+1,FI_32FC2),roi;
			roi.AttachROI(buf,ff::Rect(1,1,m_iw,m_ih));

			IntegralImage<1>(pix2,m_iw,m_ih,sizeof(double)*m_iw,sizeof(double),FI_AL_DSP_AS(roi,double),true);

			m_i2int.Reset(m_iw,m_ih,FI_32FC2,sizeof(double));
			fiSetMem(m_i2int,0);

			GetIntegralValue<1>(FI_AL_DWHSP_AS(roi,double),FI_AL_DSP_AS(m_i2int,double),m_pw,m_ph);

			m_i2roi.AttachROI(m_i2int,ff::Rect(0,0,m_iw-m_pw,m_ih-m_ph));
			m_rfft_roi.Attach(m_rfft.out_buf(),m_iw-m_pw,m_ih-m_ph,FI_32FC2,sizeof(double)*m_iw);
		}
#endif

		for_each_pixel_0_1(pix,m_iw*m_dim,m_ih,m_dim*m_iw,1,inplace_i0(iop_scale<1,double>(2.0f*scale)));

		for(int i=0;i<m_dim;++i)
		{
			const cdata_t *pres=m_ifft.exec(pix+i,sizeof(double)*m_dim*m_iw,sizeof(double)*m_dim,FPMT_64F);

			fiuCopy(pres,sizeof(cdata_t)*m_iw,m_ih/2+1,sizeof(cdata_t)*m_iw,FI_AL_DS(m_vispec[i]));
		}

		delete[]pix;
		delete[]pix2;
	}

	void SetMask(const unsigned char *mask, int mstep)
	{
		fpm_set_mask(m_iw,m_ih,mask,mstep,m_mask0);
	}

	double _calc_delta(const double *pat,const double *weight)
	{
		const int np=m_pw*m_ph;
		double delta=0;

		for(int i=0;i<np;++i,pat+=m_dim,weight++)
		{
			for(int j=0;j<m_dim;++j)
				delta+=*weight*pat[j]*pat[j];
		}
		return delta;
	}

	const double *GetSSD(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, double *pDelta)
	{
		this->PlanPatch(pwidth,pheight);

		fpm_cvt_data(pat,m_pw*m_dim,m_ph,pstep,0,ptype,&m_wpat[0],sizeof(double)*m_pw*m_dim,0,FPMT_64F);

		if(pDelta)
			*pDelta=this->_calc_delta(&m_wpat[0],weight);

		int np=m_pw*m_ph,nip=m_iw*(m_ih/2+1);

		double *pibuf=m_pfft.in_buf(),*ppspec=m_wpbuf.DataAs<double>();

		for(int k=0;k<m_dim;++k)
		{
			int j=k;
			if(m_flag&FPMF_WEIGHTED)
			{
				for(int i=0;i<np;j+=m_dim,++i)
					pibuf[i]=m_wpat[j]*weight[i];
			}
			else
			{
				for(int i=0;i<np;j+=m_dim,++i)
					pibuf[i]=m_wpat[j];
			}

			const cdata_t *pwspec=m_pfft.exec();

			if(k==0)
				_mul_conj((double*)pwspec,m_vispec[k].DataAs<double>(),nip,ppspec);
			else
				_add_mul_conj((double*)pwspec,m_vispec[k].DataAs<double>(),nip,ppspec);
		}

		const double   *pwssd=NULL;

		if(m_flag&FPMF_WEIGHTED)
		{
			assert(weight);

			memcpy(pibuf,weight,sizeof(double)*np);

			const cdata_t *pwspec=m_pfft.exec();

			_calc_spec_conv((double*)pwspec,m_i2spec.DataAs<double>(),(double*)ppspec,(double*)m_rfft.in_buf(),m_rbuf.DataAs<double>(),m_iw,m_ih);

			pwssd=(double*)m_rfft.exec();
		}
		else
		{
			_calc_spec_conv((double*)ppspec,(double*)m_rfft.in_buf(),m_iw,m_ih);

			pwssd=(double*)m_rfft.exec();

			assert(pwssd==m_rfft_roi.DataAs<double>());

		//	for_each_pixel_1_1(FI_AL_DWHSP_AS(m_i2roi,double),FI_AL_DSP_AS(m_rfft_roi,double),inplace_i1(iop_sub<1>()));
			for_each_pixel_1_1(m_i2roi.DataAs<double>(), m_i2roi.Width(), m_i2roi.Height(), m_i2roi.Width()*m_i2roi.NChannels(), m_i2roi.NChannels(), m_rfft_roi.DataAs<double>(), m_rfft_roi.Width()*m_rfft_roi.NChannels(),m_rfft_roi.NChannels(),inplace_i1(iop_sub<1>()));
		}

		return (double*)pwssd;
	}

	int Match(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, int *pmatch,double *pssdx,int nmax,int flag)
	{
		double delta=0;

		const double *pssd=this->GetSSD(pat,pwidth, pheight, pstep,ptype,weight,flag&FPMF_TRUE_SSD? &delta:NULL);

		int nm= fpm_ssd_select(pssd,m_mask.Data(),m_iw*m_ih,pmatch,nmax,m_sel);

		if(pssdx)
		{
			for(int i=0;i<nm;++i)
				pssdx[i]=pssd[pmatch[i]]+delta;
		}
		return nm;
	}
};


//====================================================================================================================

FPM_FFT::FPM_FFT()
:m_pImp(new _CImp)
{
}

FPM_FFT::~FPM_FFT()
{
	delete m_pImp;
}

const double *FPM_FFT::GetSSD(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, double *pdelta)
{
	if(!pat||!weight&&(m_pImp->m_flag&FPMF_WEIGHTED))
	{
		FF_EXCEPTION(ERR_NULL_POINTER,"");
	}
	return m_pImp->GetSSD(pat, pwidth, pheight, pstep,ptype,weight,pdelta);
}

void FPM_FFT::Plan(int iwidth, int iheight, int dim,int flag, double sel)
{
	m_pImp->Plan(iwidth,iheight,dim,flag,sel);
}

void FPM_FFT::SetImage(const void *img, int istep, int itype, const unsigned char *mask, int mstep)
{
	if(!img)
	{
		FF_EXCEPTION(ERR_NULL_POINTER,"");
	}

	m_pImp->SetImage(img,istep,itype);
	m_pImp->SetMask(mask,mstep);
}

int FPM_FFT::Match(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, int *pmatch,double *pssd,int nmax,int flag)
{
	if(!pat||!pmatch)
	{
		FF_EXCEPTION(ERR_NULL_POINTER,"");
	}
	return m_pImp->Match(pat, pwidth, pheight, pstep,ptype,weight,pmatch,pssd,nmax,flag);
}







