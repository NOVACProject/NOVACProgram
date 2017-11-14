#include "StdAfx.h"
#include "mkpack.h"

#include "../Spectra/Spectrum.h"

using namespace SpectrumIO;

MKPack::MKPack(void)
{
}

MKPack::~MKPack(void)
{
}

void MKPack::SetBit(unsigned char *pek,long bit)
{
	unsigned short ut=0x80;
	pek[bit>>3]|=(ut>>(bit&7));
}

void MKPack::ClearBit(unsigned char *pek,long bit)
{
	unsigned char ut=0x80;
	pek[bit>>3]&=~(ut>>(bit&7));
}

void MKPack::WriteBits(short a,short curr,long *inpek,unsigned char *utpek,long bitnr)
{
	short jj,j;
	long utwrd;
	long kk;
	unsigned short utlng=0x80;

	utwrd=(unsigned long)( (a<<5) | (curr & 0x1f) );
	kk=1L<<(headsiz-1);
	for(j=0;j<headsiz;j++)
		{
			if( utwrd & kk ) utpek[(bitnr>>3)]|=(utlng>>(bitnr&7));
			bitnr++;
			kk=kk>>1;
		}
		
	for(jj=0;jj<a;jj++)						 /* spara undan alla */
		{
			kk=(1L<<(curr-1));
			utwrd=*inpek++;
			for(j=0;j<curr;j++)
	{
		if( utwrd & kk ) utpek[(bitnr>>3)]|=(utlng>>(bitnr&7));
		bitnr++;
		kk=kk>>1;
	}
		}
}

short MKPack::BitsPrec(long i)
{
	short j = 1;
	
	// If 'i' is 0 then we need 0 bits to store it.
	if(!i)
		return(0);

	if(i < 0){
		if(i == -1)
			return(1);
		while(i != -1 ){
			j++;
			i=(i>>1);
		}
	}else{
		// as long as there are bits left in 'i', increase
		//	the counter of number of bits and shift all bits 
		//	in 'i' one step (divide by 2).
		while( i ){
			j++;
			i=(i>>1);
		}
	}
	return(j);
}

void MKPack::PackSeg(unsigned char *utpek, long *kvar)
{
	short len[64]; // was '33', increased buffer to manage 'strange' spectra
	long j;
	short curr,i,a;
	
	// set all elements of 'len' to zero
	for(j = 0; j < 64; j++)
		len[j] = 0;

	// incpy is a local copy of 'm_strt'
	long *incpy = m_strt;

	// the number of bits that we need to store the 
	//	current pixel-value
	i = BitsPrec(*incpy++);
	curr = i; 
	a = 0;
	do {
		a++;
		for(j = 0; j < curr; ++j){
			if(i > j){
				len[j] = 0;
			}else{
				len[j]++;
				if( len[j]*(curr-j)>headsiz*2){
					a -= len[j];
					goto Fixat;
				}
			}
		}
		i = BitsPrec(*incpy++);
		if(i > curr)
		{
			/* i har blivit för stort. Vi ska då titta bakåt så att
				 vi tjänar in plats bakåt också på att öka bitantalet */
			if(a * (i-curr) > headsiz )
				goto Fixat;

			/* gå till fixat om det inte lönar sig att öka
				 bitantalet på den föregående gruppen */

				while(curr != i) /* det lönade sig att byta */
				{
					len[curr] = a;
					curr++; /* öka bitantalet */
				}
		}
	}while(a < *kvar && a < 127);

 Fixat:	
	WriteBits(a, curr, m_strt, utpek, m_bitnr);
	*kvar -= a;
	m_strt += a; // increase m_strt, i.e. go to the next value in the buffer to compress
	m_bitnr += a * curr + headsiz;
}

unsigned short MKPack::mk_compress(long *in,unsigned char *ut,unsigned short size)
{
	unsigned short outsize;

	// 'kvar' is the length of the spectrum
	long kvar = size;

	// m_strt is a pointer to the next value we want to compress
	m_strt = in;
	m_bitnr = 0;
	do {
		PackSeg(ut, &kvar);
	} while(kvar > 0);

	outsize = (unsigned short)(m_bitnr+7)>>3;
	return(outsize);
}

/** Decompresses the spectrum. 
	@param inpek - a pointer to the compressed spectrum
	@param kvar - the length of the uncompressed spectrum
	@param ut - a pointer to a buffer to which the data will be uncompressed */
long MKPack::UnPack(unsigned char *inpek, long kvar, long *ut )
{
	long *utpek = NULL;
	short len,curr;
	short j,jj;
	long a;
	unsigned short lentofile=0;
	long bit = 0;

	// validate the input data - Added 2006.02.13 by MJ
	if(kvar > MAX_SPECTRUM_LENGTH)
		return -1;
	if(ut == NULL || inpek == NULL)
		return -1;


	utpek = ut;
	lentofile = 0;
	while(kvar > 0)
	{
		len = 0;
		for(j = 0; j < 7; j++)
		{
			len += len;
			len |= inpek[(bit>>3)]>>(7-(bit&0x7))&1;
			bit++;
		}
		curr = 0;
		for(j = 0; j < 5; j++)
		{
			curr += curr;
			curr |= inpek[(bit>>3)]>>(7-(bit&0x7))&1;
			bit++;
		}
		if(curr)
		{
			for(jj = 0; jj < len; jj++)
			{
				a = inpek[(bit>>3)]>>(7-(bit&0x7))&1;
				if(a)
					a = -1;
				bit++;
				for(j = 1; j < curr; j++)
				{
					a += a;
					a |= inpek[(bit>>3)]>>(7-(bit&0x7))&1;
					bit++;
				}
				*utpek++ = a;
			}
		 }
		 else 
			 for(jj = 0;jj < len; jj++)
				 *utpek++=0;

		kvar -= len;			
		lentofile += len;
	}
	for(jj=1;jj<lentofile;jj++)
	{
		ut[jj] += ut[jj-1];
	}
//	fclose(f);

	return(lentofile);
}
