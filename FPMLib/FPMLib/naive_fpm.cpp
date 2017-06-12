

#include"fpm.h"

#include"fpmi.h"


template<typename _DataT,typename _DestT>
void calc_ssd(const _DataT *pImg, int iw, int ih, int istep, const uchar *pMask, int mstep,
			  const _DataT *pPat, int pw, int ph,
			  const int dim,
			  _DestT *pSSD
			  )
{
	typedef typename ff::DiffType<_DataT>::RType _DiffT;

	int nex=pw*dim;

	for(int yi=0;yi<ih-ph;++yi, pImg=ByteDiff(pImg,istep),pMask+=mstep,pSSD+=iw)
	{
		_DestT *ssdx=pSSD;
		const _DataT *pix=pImg;

		for(int xi=0;xi<iw-pw;++xi,pix+=dim,++ssdx)
		{
			if(pMask[xi])
			{
				const _DataT *pixx=pix,*ppx=pPat;
				_DestT s=0;

				for(int i=0;i<ph;++i,pixx=ByteDiff(pixx,istep))
				{
					for(int j=0;j<nex;++j,++ppx)
					{
						_DiffT d=pixx[j]-*ppx;

						s+=(_DestT)(d*d);
					}
				}

				*ssdx=s;
			}
		}
	}
}

template<typename _DataT,typename _DestT,typename _WeiT>
void calc_wssd(const _DataT *pImg, int iw, int ih, int istep, const uchar *pMask, int mstep,
			  const _DataT *pPat, int pw, int ph, const _WeiT *pWei,
			  const int dim,
			  _DestT *pSSD
			  )
{
	typedef typename ff::DiffType<_DataT>::RType _DiffT;

	int nex=pw*dim;

	for(int yi=0;yi<ih-ph;++yi, pImg=ByteDiff(pImg,istep),pMask+=mstep,pSSD+=iw)
	{
		const _DataT *pix=pImg;
		_DestT *ssdx=pSSD;

		for(int xi=0;xi<iw-pw;++xi,pix+=dim,++ssdx)
		{
			if(pMask[xi])
			{
				const _DataT *pixx=pix,*ppx=pPat;
				const _WeiT  *pwx=pWei;
				_DestT s=0;

				for(int i=0;i<ph;++i,pixx=ByteDiff(pixx,istep))
				{
					for(int j=0;j<nex;++j,++ppx,++pwx)
					{
						_DiffT d=pixx[j]-*ppx;

						s+=(_DestT)(*pwx*(d*d));
						//s+=(_DestT)(*pwx * std::abs(d));
					}
				}

				*ssdx=s;
			}
		}
	}
}


template<typename _DataT>
void ssd(const void *pImg, int iw, int ih, int istep, const uchar *pMask, int mstep,
			  const void *pPat, int pw, int ph, const double *pWei,
			  const int dim,
			  double *pSSD
			  )
{
	if(pWei)
	{
		calc_wssd((_DataT*)pImg,iw,ih,istep,pMask,mstep,(_DataT*)pPat,pw,ph,pWei,dim,pSSD);
	}
	else
	{
		calc_ssd((_DataT*)pImg,iw,ih,istep,pMask,mstep,(_DataT*)pPat,pw,ph,dim,pSSD);
	}
}

typedef void (*FuncT)(const void *pImg, int iw, int ih, int istep, const uchar *pMask, int mstep,
			  const void *pPat, int pw, int ph, const double *pWei,
			  const int dim,
			  double *pSSD
			  );

const static FuncT g_ssdFunc[]=
{
	ssd<uchar>,ssd<int>,ssd<float>,ssd<double>
};

#if 1


class FPMNaive::_CImp
{
public:
	int		m_iw,m_ih,m_pw,m_ph,m_dim,m_flag;
	double  m_sel;

	int     m_itype,m_istep,m_tsz;

	FVTImage m_mask0, m_mask;
	std::vector<uchar>  m_img,m_pat;
	std::vector<double> m_ssd,m_vw;

public:
	virtual void Plan(int iwidth, int iheight, int dim,int flag, double sel)
	{
		m_iw=iwidth, m_ih=iheight, m_pw=0, m_ph=0, m_dim=dim, m_flag=flag, m_sel=sel;
	}

	virtual void SetImage(const void *img, int istep,  int itype, const unsigned char *mask, int mstep)
	{
		assert(fpm_is_valid_type(itype));

		m_img.resize(istep*m_ih);
		memcpy(&m_img[0],img,istep*m_ih);

		m_itype=itype;
		m_istep=istep;
		m_tsz=fpm_type_size(itype);

		m_ssd.resize(m_iw*m_ih);
		fpm_set_mask(m_iw,m_ih,mask,mstep,m_mask0);
	}

	virtual const double* GetSSD(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight,double *pdelta)
	{
		if(pwidth!=m_pw || pheight!=m_ph)
		{
			m_pw=pwidth; m_ph=pheight;
			m_pat.resize(m_pw*m_ph*m_dim*m_tsz);

			uchar mv=0;
			m_mask=m_mask0;
			iop_copy<1> tmp(0);
			auto tmp2 = bind_i0(&mv,tmp);
			for_boundary_pixel_0_1(FI_AL_DWHSP(m_mask),tmp2,0,pheight,0,pwidth);

			m_vw.resize(m_pw*m_ph*m_dim);
		}


		fpm_cvt_data(pat,m_pw*m_dim,m_ph,pstep,0,ptype,&m_pat[0],m_pw*m_dim*m_tsz,0,m_itype);

		if((m_flag&FPMF_WEIGHTED)&&weight)
		{
			double *pwx=&m_vw[0];
			const int np=m_pw*m_ph;

			for(int i=0;i<np;++i,pwx+=m_dim)
			{
				for(int j=0;j<m_dim;++j)
					pwx[j]=weight[i];
			}
		}

		(g_ssdFunc[m_tsz-1])(&m_img[0],m_iw,m_ih,m_istep,FI_AL_DS(m_mask),&m_pat[0],m_pw,m_ph,m_flag&FPMF_WEIGHTED? &m_vw[0]:NULL,m_dim,&m_ssd[0]);

		if(pdelta)
			*pdelta=0;

		return &m_ssd[0];
	}

	virtual int Match(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, int *pmatch,double *pssdx,int nmax,int flag)
	{
		const double *pssd=this->GetSSD(pat,pwidth,pheight,pstep,ptype,weight,NULL);

		int nm= fpm_ssd_select(pssd,m_mask.Data(),m_iw*m_ih,pmatch,nmax,m_sel);

		if(pssdx)
		{
			for(int i=0;i<nm;++i)
				pssdx[i]=pssd[pmatch[i]];
		}
		return nm;
	}
};


//=======================================================================================================================

FPMNaive::FPMNaive()
:m_pImp(new _CImp)
{
}

FPMNaive::~FPMNaive()
{
	delete m_pImp;
}

const double *FPMNaive::GetSSD(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, double *pdelta)
{
	if(!pat||!weight&&(m_pImp->m_flag&FPMF_WEIGHTED))
	{
		FF_EXCEPTION(ERR_NULL_POINTER,"");
	}
	return m_pImp->GetSSD(pat,pwidth, pheight,pstep,ptype,weight,pdelta);
}

void FPMNaive::Plan(int iwidth, int iheight, int dim,int flag, double sel)
{
	m_pImp->Plan(iwidth,iheight,dim,flag,sel);
}

void FPMNaive::SetImage(const void *img, int istep, int itype, const unsigned char *mask, int mstep)
{
	if(!img)
	{
		FF_EXCEPTION(ERR_NULL_POINTER,"");
	}

	m_pImp->SetImage(img,istep,itype,mask,mstep);
}

int FPMNaive::Match(const void *pat, int pwidth, int pheight, int pstep, int ptype, const double *weight, int *pmatch,double *pssd,int nmax,int flag)
{
	if(!pat||!pmatch)
	{
		FF_EXCEPTION(ERR_NULL_POINTER,"");
	}
	return m_pImp->Match(pat,pwidth, pheight, pstep,ptype,weight,pmatch,pssd,nmax,flag);
}

#endif
