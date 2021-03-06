/* Collection of various routines needed by the GCC when no OS is present 

  Works for the PowerPC 403 series currently

  Released under the GNU Public License 4/98 with no guarantee or statement
  for fitness of purpose for any application

Larry Battraw
(battraw@cicddal.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>


void outbyte(int byte);
int inbyte(void);


#define spbase 0x40000000
#define _spls   0
#define _sphs   2
#define _brdh   4
#define _brdl   5
#define _spctl  6
#define _sprc   7
#define _sptc   8
#define _sprb   9
#define _sptb   9


#define SPLS (*(volatile unsigned char*) (spbase+_spls))
#define SPHS (*(volatile unsigned char*) (spbase+_sphs))
#define SPBRDH (*(volatile unsigned char*) (spbase+_spbrdh))
#define SPBRDL (*(volatile unsigned char*) (spbase+_spbrdl))
#define SPCTL (*(volatile unsigned char*) (spbase+_spctl))
#define SPRC (*(volatile unsigned char*) (spbase+_sprc))
#define SPTC (*(volatile unsigned char*) (spbase+_sptc))
#define SPRB (*(volatile unsigned char*) (spbase+_sprb))
#define SPTB (*(volatile unsigned char*) (spbase+_sptb))
#define SPRB (*(volatile unsigned char*) (spbase+_sprb))


#define sgr	0x03b9
#define iccr	0x3fb
#define br0	0x80
#define br1	0x81
#define br2	0x82
#define iocr	0xa0
#define lr	0x008

 void _start(void)
{
	asm("
	addis 1,0,0x7fe8
	subi  1,1,0x1000	
	mr    2,1
	addi  3,0,0x1000
	mtmsr 3
        b main
	");
} 


int inbyte(void)
{
        while((SPLS&128)==0 && (SPLS&0x78)==0) 
	{
	  if((SPLS&0x78)!=0) SPLS=0x78;
	  if((SPHS&0xc0)!=0) SPHS=0xc0;
 	}
	  
        return(SPRB);
}

void outbyte(int byte)
{
        while((SPLS&4)!=4) ;
        SPTB=byte;
}

void byteout(unsigned char val, unsigned long addr)
{
	asm("stb  3,0(4)");
}
int bytein(unsigned long addr)
{
	asm("lbz 3,0(3)");
}


/* returns -1 if character waiting, 0 if not */
int getchq(void)
{
        if((SPLS&128)==0) return(0);
        else return(-1);
}


int __srget(FILE *stream)
{
return(inbyte());
}

int rget(FILE *stream)
{
return(inbyte());
}


int link(const char *oldpath, const char *newpath) { return(-1); }

int *__errno(void)
{
static int _errorno_;
return(&_errorno_);
}


void prs(char *str)
{
int i;
for(i=0; i<99999; i++)
 {
        if(str[i]==0) break;
        outbyte(str[i]);
	if(str[i]=='\n') outbyte('\r');
 	if(str[i]=='\r') outbyte('\n');
 }
}


void abort(void)
{
prs("<abort>-- exit to OS\n");
asm("   lis     3,0x7ff0
        mtlr    3
        blr");
	while(1) ;
}



void _exit(int _status ) 
{
prs("<normal exit>-- returning to OS\n");
asm("	lis	3,0x7ff0
	mtlr	3
	blr");
	while(1);
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
return(0);
}
clock_t times(struct tms *buf)
{
return(1);
}
int ftime(struct timeb *tp)
{
return(0);
}



