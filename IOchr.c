#include "IO.h"
#include <stdio.h>
char outbuffer=0;
int outcount=0;
char inbuffer=0;
char incount=0;
void ffrisc_out(char * f){
	outbuffer<<=1;
	outbuffer+=(*f>0);
	outcount++;
	if (outcount==8){
		putc(outbuffer,stdout);
		outcount=0;
	}
	return;
}
void ffrisc_in(char * f){
	if (incount==0||incount==8){
		incount=0;
		inbuffer=getc(stdin);
	}
	*f=(inbuffer&0x01);
	inbuffer>>1;
	incount++;
	return;

}
