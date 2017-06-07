
#ifndef _INC_FPMI_H
#define _INC_FPMI_H

#include"fpm.h"

#include"iff/iff.h"
#include"ipf/ipf.h"
using namespace ff;

void fpm_cvt_data(const void *pin, int width, int height, int istep, int istride, int itype, 
				   void *pout, int ostep, int ostride, int otype
				   );


inline bool fpm_is_valid_type(int type)
{
	return type>FPMT_BEG&&type<FPMT_END;
}


int fpm_ssd_select(const double *pssd, const unsigned char *pmask, int count, int *pmatch, int nmax, double ss);

void fpm_set_mask(int iw,int ih,int pw,int ph, const uchar *pMask, int mstep, FVTImage &dest,unsigned char mval=1);

void fpm_set_mask(int iw,int ih,const uchar *mask, int mstep, FVTImage &dest, uchar mval=1);

int fpm_type_size(int type);


template<int _NDIM,typename _ValT,typename _IntValT>
inline void IntegralImage(const _ValT *pSrc,int width,int height,int irowStep,int ipxStep,
						  _IntValT *pInt,int orowStep,int opxStep, bool bFillBorder
						  )
{
	if(bFillBorder)
	{
		_IntValT zero[_NDIM];
		
		memset(zero,0,sizeof(zero));

		fiuSetBoundaryPixel(ByteDiff(pInt,-orowStep-opxStep),width+1,height+1,orowStep,opxStep,zero,sizeof(zero),1,0,1,0);
	}

	_IntValT (*pRow)[_NDIM]=new _IntValT[width][_NDIM];
	memset(pRow,0,sizeof(_IntValT)*width*_NDIM);

	for(int yi=0;yi<height;++yi,pSrc=ByteDiff(pSrc,irowStep),pInt=ByteDiff(pInt,orowStep))
	{
		const _ValT *psx=pSrc;
		_IntValT *pix=pInt,*pix0=NULL;

		for(int xi=0;xi<width;++xi,psx=ByteDiff(psx,ipxStep),pix=ByteDiff(pix,opxStep))
		{
			if(xi==0)
			{
				for(int i=0;i<_NDIM;++i)
				{
					pRow[0][i]+=psx[i];
					pix[i]=pRow[0][i];
				}
			}
			else
			{
				for(int i=0;i<_NDIM;++i)
				{
					pRow[xi][i]+=psx[i];
					pix[i]=pix0[i]+pRow[xi][i];
				}
			}
			pix0=pix;
		}
	}
	delete[]pRow;
}

//NOTE:the integral image must contain the top and left border.
template<int _NDIM,typename _IntValT,typename _DestT>
inline void GetIntegralValue(const _IntValT *pInt,int width,int height,int irowStep,int ipxStep,
							 _DestT *pDest,int drowStep,int dpxStep,
							 const int wx,const int wy
							 )
{
	int np=0;

	for(int yi=0;yi<=height-wy;++yi,pInt=ByteDiff(pInt,irowStep),pDest=ByteDiff(pDest,drowStep))
	{
		const _IntValT *pix=pInt;
		_DestT *pdx=pDest;

		for(int xi=0;xi<=width-wx;++xi,pix=ByteDiff(pix,ipxStep),pdx=ByteDiff(pdx,dpxStep))
		{
#define _INT_VAL(dx,dy)  ByteDiff(pix,(dy)*irowStep+(dx)*ipxStep)

			const _IntValT *pLT=_INT_VAL(-1,-1),*pRB=_INT_VAL(wx-1,wy-1),*pLB=_INT_VAL(-1,wy-1),*pRT=_INT_VAL(wx-1,-1);

			for(int i=0;i<_NDIM;++i)
			{
					pdx[i]=(_DestT)(pLT[i]+pRB[i]-pLB[i]-pRT[i]);
			}			
#undef  _INT_VAL

		}
	}

}




#endif

