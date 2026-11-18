
struct MKZYhdr
{
  char ident[4];
  unsigned short hdrsize;
  unsigned short hdrversion;
  unsigned short size;
  unsigned short checksum;
  char name[12];
  char instrumentname[16];
  unsigned short startc;
  unsigned short pixels;
  unsigned short viewangle;
  unsigned short scans;
  short exptime;
  unsigned char channel;
  unsigned char flag;
  unsigned long date;
  unsigned long starttime;
  unsigned long stoptime;
  double lat;
  double lon;
};

struct MKZYhdr MKZY;


#define headsiz 12

void SetBit(unsigned char *pek,long bit)
{
  unsigned short ut=0x80;
  pek[bit>>3]|=(ut>>(bit&7));
}

void ClearBit(unsigned char *pek,long bit)
{
  unsigned char ut=0x80;
  pek[bit>>3]&=~(ut>>(bit&7));
}

void WriteBits(short a,short curr,long *inpek,unsigned char *utpek,long bitnr)
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
    
  for(jj=0;jj<a;jj++)             /* spara undan alla */
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

short BitsPrec(long i)
{
short j=1;
if(!i) return(0);
if(i<0)
        {
        if(i == -1) return(1);
        while(i != -1 )
                {
                j++;
                i=(i>>1);
                }
        }
else
while( i )
        {
        j++;
        i=(i>>1);
        }
return(j);
}

long bitnr;
long *strt;

void PackSeg(unsigned char *utpek, long *kvar )
{
  short len[33];
  long j;
  long *incpy;
  short curr,i,a;
  
  for(j=0;j<33;j++) len[j]=0;
  incpy=strt;

  i=BitsPrec(*incpy++);
  curr=i; 
  a=0;
  do {
    a++;
    for(j=0;j<curr;j++)
      {
        if(i>j) len[j]=0;
        else {
          len[j]++;
          if( len[j]*(curr-j)>headsiz*2)
            {
              a-=len[j];
              goto Fixat;
            }
        }
      } 
    i=BitsPrec(*incpy++);    
    if(i>curr)
      {
        /* i har blivit för stort. Vi ska då titta bakåt så att
           vi tjänar in plats bakåt också på att öka bitantalet */      
        if( a*(i-curr)>headsiz ) goto Fixat;
        
        /* gå till fixat om det inte lönar sig att öka
           bitantalet på den föregående gruppen */
               
        while(curr!=i)          /* det lönade sig att byta */
          {
            len[curr]=a;
            curr++;                     /* öka bitantalet */
          }
      }
  } while( a<*kvar && a<127 );
 Fixat:
    
  WriteBits(a,curr,strt,utpek,bitnr);  
  *kvar -=a;
  strt += a;                  /* öka strt */
  bitnr += a*curr+headsiz;
}

unsigned short mk_compress(long *in,unsigned char *ut,unsigned short size)
{
  long kvar;
  unsigned short outsize;

  strt=in;
  kvar=size;
  bitnr=0;
  do {
    PackSeg(ut,&kvar);
  } while( kvar>0);

  outsize=(bitnr+7)>>3;
  return(outsize);
}

long UnPack(unsigned char *inpek,long kvar,long *ut )
{
  long *utpek;
  short len,curr;
  short j,jj;
  long a;
  unsigned short lentofile=0;
  long bit=0;
  FILE *f;
  
  /*  memcpy(ut,inpek,kvar*2);
  return(kvar);
  */
  f=fopen("chunks.txt","w");

  utpek=ut;
  lentofile=0;  
  while(kvar>0)
    {
      len=0;
      for(j=0;j<7;j++)
        {
          len+=len;
          len|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
          bit++;
        }      
      curr=0;
      for(j=0;j<5;j++)
        {
          curr+=curr;
          curr|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
          bit++;
        }
      fprintf(f,"%d %d\n",len,curr);    
      if(curr)
        {
          for(jj=0;jj<len;jj++)
            {
              a=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
              if(a) a=-1;
              bit++;
              for(j=1;j<curr;j++)
                {
                  a+=a;
                  a|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
                  bit++;
                }
              *utpek++=a;
            }
        }
      else for(jj=0;jj<len;jj++) *utpek++=0;      
      kvar-=len;      
      lentofile+=len;
    }
  for(jj=1;jj<lentofile;jj++)
    {
      ut[jj]+=ut[jj-1];
    }
  fclose(f);
  return(lentofile);
}
