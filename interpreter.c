#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "IO.h"
void interpret(int,unsigned char *);
void print_debug_info(unsigned char A, unsigned char B,unsigned  char X, unsigned char Y, unsigned short PC,unsigned  short DP,unsigned short Backup1,unsigned short Backup2, unsigned short Backup3, unsigned char f,unsigned char halfPC, unsigned char * memory);
char turbo=0;
char debug=0;
char slow=0;
int i;

struct timespec normal_speed;
struct timespec slow_speed;
int main(int argc, char * argv[]){
	if (argc<=1) {
		return 1;
	}
	{
	//1MHz
	normal_speed.tv_sec=0;
	normal_speed.tv_nsec=1000l;
	//10Hz
	slow_speed.tv_sec=0;
	slow_speed.tv_nsec=10000000l;
	int found;
	while (argc>2){
		found=0;
		if (strcmp(argv[1],"-t\0")==0){
			turbo=1;
			found=1;
		}
		else if (strcmp(argv[1],"-s\0")==0){
			slow=1;
			found=1;
		}
		else if (strcmp(argv[1],"-d\0")==0){
			printf("debug mode on\n");
			debug=1;
			found=1;
		}
		if (found>0){
			for (i=1; i<argc; i++)
				argv[i]=argv[i+1];
			argc--;
		}
		else return 1;
	}
	if (turbo+slow==2) return 1;
	}
	char * filename=argv[1];
	FILE * code_binary=fopen(filename,"rb");
	unsigned char * initial_memory=malloc(65536);
	int sizeOfProgram=fread(initial_memory,sizeof(unsigned char), 65536, code_binary);
	interpret(sizeOfProgram, initial_memory);
	return 0;
}
void print_debug_info(unsigned char A, unsigned char B,unsigned  char X, unsigned char Y, unsigned short PC,unsigned  short DP,unsigned short Backup1,unsigned short Backup2,unsigned short Backup3, unsigned char f,unsigned char halfPC,unsigned char * memory){
	printf("f=%d\n",(f>0));
	printf("A=%3hhu(0x%02x)\n",A,A);
	printf("B=%3hhu(0x%02x)\n",B,B);
	printf("X=%3hhu(0x%02x)\n",X,X);
	printf("Y=%3hhu(0x%02x)\n",Y,Y);
	printf("\nDP=%6hu(0x%04x)\n",DP,DP);
	printf("\nPC=%6hu%s(0x%04x%s)\n",PC,halfPC?".5":"",PC,halfPC?".8":"");
	printf("command=%02x\n",memory[PC]);
	printf("\nBackup1=%04x | Backup2=%04x | Backup3=%04x\n",Backup1,Backup2,Backup3);
	printf("\n");
}			
void interpret(int sizeOfProgram, unsigned char * initial_memory){
	unsigned char A,B,X,Y;
	unsigned short PC, DP, Backup1, Backup2, Backup3;
	unsigned char f,c,halfPC;
	unsigned char branch,memory_inst;
	unsigned char instruction;
	unsigned int temp;
	unsigned char * memory;
	halfPC=c=f=Y=X=B=A=(unsigned char)0;
	PC=(unsigned short)0;
	//fp=(unsigned short)0x100;
	Backup1=0xffff;
	Backup2=0xffff;
	memory=initial_memory;
	char Compressed_Array[16]={
							0x02, //0: NOP
							0x6f, //1: SKIPF
							0x7f, //2: FLIPF
							0x07, //3: MEMORY A,[PC+X]
							0x0d, //4: ADDF A,A
							0x82, //5: MOVE X,A
							0xcf, //6: SET
							0xdf, //7: RESET
							0xaf, //8: OUT
							0xbf, //9: IN
							0x8b, //A: JUMP X
							0x3a, //B: INCF A
							0x22, //C: MOVE A,X
							0xba, //D: INCF X
							0xbb, //E: DECF X
							0x0f, //F: EXIT COMPRESSED MODE
							};
	DP=(((unsigned short)sizeOfProgram) & 0xff00)+0x100;
	Backup3=DP+0x1000;
	
	
	while (1) {
		
		branch=0;
		memory_inst=0;
		if (debug){
			print_debug_info(A,B,X,Y,PC,DP,Backup1,Backup2, Backup3,f,halfPC, memory);
		}
		if (c){
			if (halfPC){
				instruction=Compressed_Array[(memory[PC]>>4)];
			}
			else{
				instruction=Compressed_Array[(memory[PC]&15)];
			}
			
		}
		else{
			instruction=memory[PC];
		}
		switch(instruction){
			case 0x00: //TEST A,0
				f=A&0x01;
				break;
			case 0x01: //PUTF A,0
				if (f) {
					A|=0x01;
				}
				else{
					A&=0xfe;
				}
				break;
			case 0x02: //NOP
				break;
			case 0x03: //MEMORY A,[PC+A]
				if (f)
					memory[PC+A]=A;
				else
					A=memory[PC+A];
				break;
			case 0x04: //XOR A,A
				A=0;
				f=1;
				break;
			case 0x05: //OR A,A
				f=A==0xff;
				break;
			
			case 0x06: //AND A,A
				f=A==0;
				break;
			case 0x07: //MEMORY A,[PC+X]
				if (f)
					memory[PC+X]=A;
				else
					A=memory[PC+X];
				
				memory_inst=1;
				break;
			case 0x08: //TEST A,1
				f=(A&0x02)>0;
				break;
			case 0x09: //PUTF A,1
				if (f)
					A|=0x02;
				else
					A&=0xfd;
				break;
			case 0x0a: //NOT A
				A=~A;
				f=!f;
				break;
			case 0x0b: //JUMP A
				branch=1;
				PC+=((signed char)A);
				break;
			case 0x0c: //ADD A,A
				temp=A+A;
				f=temp>0xff;
				A=(unsigned char)temp;
				break;
			case 0x0d: //ADDF A,A
				temp=A+A+(f>0);				
				f=temp>0xff;
				A=(unsigned char)temp;
				break;
			case 0x0e: //SUBF A,A
				if (f) {
					A=0xff;
				}
				else{
					A=0;
				}
				break;
			case 0x0f: //COMPRESSED
				c=!c;
				halfPC=0;
				break;
			case 0x10: //TEST A,2
				f=(A&0x04)>0;
				break;
			case 0x11: //PUTF A,2
				if (f) {
					A|=0x04;
				}
				else{
					A&=0xfb;
				}
				break;
			case 0x12: //MOVE A,B
				A=B;
				break;
				
			case 0x13: //MEMORY A, [PC+B]
				if (f)
					memory[PC+B]=A;
				else
					A=memory[PC+B];
				memory_inst=1;
				break;
			case 0x14: //XOR A,B
				A=A^B;
				f=A==0xff || A==0;
				break;
			case 0x15: //OR A,B
				A=A|B;
				f=A==0xff;
				break;
			case 0x16: //AND A,B
				A=A&B;
				f=(A==0x00);
				break;
			case 0x17: //MEMORY A,[PC+Y]
				if (f)
					memory[PC+Y]=A;
				else
					A=memory[PC+Y];
				break;
			case 0x18: //TEST A,3
				f=(A&0x08)>0;
				break;
			case 0x19: //PUTF A,3
				if (f) {
					A|=0x08;
				}
				else{
					A&=0xf7;
				}
				break;
			case 0x1a: //YXDP
				if (f){
					Y=(unsigned short)(DP>>8);
					X=(unsigned short)DP;
				}
				else
					DP=X+Y<<8;
				break;
				
			case 0x1b: //JUMPLINK A
				branch=1;
				temp=((unsigned int)PC)+1;
				PC=PC+((signed char)A);
				X=((unsigned char)temp);
				Y=((unsigned char)(temp>>8));
				break;
			case 0x1c: //ADD A,B
				temp=A+B;
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x1d: //ADDF A,B
				temp=A+B+(f>0);
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x1e: //SUBF A,B
				temp=A-B-(f>0);
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x1f: //UNDEFD
				branch=1;
				PC+=((signed char)temp);
				break;
			case 0x20: //TEST A,4
				f=(A&0x10)>0;
				break;
			case 0x21: //PUTF A,4
				if (f) {
					A|=0x10;
				}
				else{
					A&=0xef;
				}
				break;
			case 0x22: //MOVE A,X
				A=X;
				break;
				
			case 0x23:  //MEMORY B, [PC+A]
				if (f)
					memory[PC+A]=B;
				else
					B=memory[PC+A];
				memory_inst=1;
				break;
			case 0x24: //XOR A,X
				A=A^X;
				f=A==0xff || A==0;
				break;
			case 0x25: //OR A,X
				A=A|X;
				f=A==0xff;
				break;
			case 0x26: //AND A,X
				A=A&X;
				f=A==0x00;
				break;
				
			case 0x27: //MEMORY B,[PC+X]
				if (f)
					memory[PC+X]=B;
				else
					B=memory[PC+X];
				memory_inst=1;
				break;	
			case 0x28: //TEST A,5
				f=(A&0x20)>0;
				break;
			case 0x29: //PUTF A,5
				if (f) {
					A|=0x20;
				}
				else{
					A&=0xdf;
				}
				break;
			case 0x2a: //RR A
				temp=A&1;
				A=(A>>1)+((f>0)<<7);
				f=temp;
				break;
				
			case 0x2b: //MEMORY A, [YX]
				temp=(((unsigned short)Y)<<8)+((unsigned short)(X));
				if (f)
					memory[temp]=A;
				else
					A=memory[temp];
				memory_inst=1;
				break;
				
			case 0x2c: //ADD A,X
				temp=A+X;
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x2d: //ADDF A,X
				temp=A+X+(f>0);
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x2e: //SUBF A,X
				temp=A-X-(f>0);
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x2f: //INC DP
				DP++;
				break;
			case 0x30: //TEST A,6
				f=(A&0x40)>0;
				break;
			case 0x31: //PUTF A,6
				if (f) {
					A|=0x40;
				}
				else{
					A&=0xbf;
				}
				break;
			case 0x32: //MOVE A,Y
				A=Y;
				break;
				
			case 0x33://MEMORY B, [PC+B]
				if (f)
					memory[PC+B]=B;
				else
					B=memory[PC+B];
				memory_inst=1;
				break;
			case 0x34: //XOR A,Y
				A=A^Y;
				f=(A==0xff || A==0);
				break;
			case 0x35: //OR A,Y
				A=A|Y;
				f=(A==0xff);
				break;
			case 0x36: //AND A,Y
				A=A&Y;
				f=(A==0x00);
				break;
				
			case 0x37: //MEMORY B,[PC+Y]
				if (f)
					memory[PC+Y]=B;
				else
					B=memory[PC+Y];
				memory_inst=1;
				break;	
			case 0x38: //TEST A,7
				f=(A&0x80)>0;
				break;
			case 0x39: //PUTF A,7
				if (f) {
					A|=0x80;
				}
				else{
					A&=0x7f;
				}
				break;
			case 0x3a: //INCF A
				temp=((unsigned int)A)+(f>0);
				f=temp>0xff;
				A=(unsigned char)temp;
				break;
				
			case 0x3b: //DECF A
				temp=((unsigned int)A)-(f>0);
				f=temp>0xff;
				A=(unsigned char)temp;
				break;
				
			case 0x3c: //ADD A,Y
				temp=A+Y;
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x3d: //ADDF A,Y
				temp=A+Y+(f>0);
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x3e: //SUBF A,Y
				temp=A-Y-(f>0);
				f=(temp>0xff);
				A=(unsigned char)temp;
				break;
			case 0x3f: //DEC DP
				DP--;
				break;
			case 0x40: //TEST B,0
				f=(B&0x01)>0;
				break;
			case 0x41: //PUTF B,0
				if (f) {
					B|=0x01;
				}
				else{
					B&=0xfe;
				}
				break;
			case 0x42: //MOVE B,A
				B=A;
				break;
				
			case 0x43:  //MEMORY X, [PC+A]
				if (f)
					memory[PC+A]=X;
				else
					X=memory[PC+A];
				memory_inst=1;
				break;
			case 0x44: //XOR B,A
				B=B^A;
				f=(B==0xff || B==0);
				break;
			case 0x45: //OR B,A
				B=B|A;
				f=(B==0xff);
				break;
			case 0x46: //AND B,A
				B=B&A;
				f=(B==0x00);
				break;
				
			case 0x47: //MEMORY X,[PC+X]
				if (f)
					memory[PC+X]=X;
				else
					X=memory[PC+X];
				memory_inst=1;
				break;	
			case 0x48: //TEST B,1
				f=(B&0x02)>0;
				break;
			case 0x49: //PUTF B,1
				if (f) {
					B|=0x02;
				}
				else{
					B&=0xfd;
				}
				break;
			case 0x4a: //NOT B
				B=~B;
				f=!f;
				break;
				
			case 0x4b: //JUMP B
				branch=1;
				PC+=((signed char)B);
				break;
				
			case 0x4c: //ADD B,A
				temp=B+A;
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x4d: //ADDF B,A
				temp=B+A+(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x4e: //SUBF B,A
				temp=B-A-(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x4f: //SWAP AB
				temp=(((unsigned int)A)<<8)+((unsigned int)B);
				B=(unsigned char)Backup1;
				A=((unsigned char)(Backup1>>8));
				Backup1=(unsigned short)temp;
				break;
			case 0x50: //TEST B,2
				f=(B&0x04)>0;
				break;
			case 0x51: //PUTF B,2
				if (f) {
					B|=0x04;
				}
				else{
					B&=0xfb;
				}
				break;
			case 0x52: //NOP
				
				break;
				
			case 0x53:  //MEMORY X, [PC+B]
				if (f)
					memory[PC+B]=X;
				else
					X=memory[PC+B];
				memory_inst=1;
				break;
			case 0x54: //XOR B,B
				B=0;
				f=1;
				break;
			case 0x55: //OR B,B
				f=(B==0xff);
				break;
			case 0x56: //AND B,B
				f=(B==0x00);
				break;
				
			case 0x57: //MEMORY X,[PC+Y]
				if (f)
					memory[PC+Y]=X;
				else
					X=memory[PC+Y];
				memory_inst=1;
				break;	
			case 0x58: //TEST B,3
				f=(B&0x08)>0;
				break;
			case 0x59: //PUTF B,3
				if (f) {
					B|=0x08;
				}
				else{
					B&=0xf7;
				}
				break;
			case 0x5a: //UNDEFD
				branch=1;
				temp=((unsigned int)temp)+1;
				PC=PC+((signed char)X);
				X=((unsigned char)temp);
				Y=((unsigned char)(temp>>8));
				break;
				
			case 0x5b: //JUMPLINK B
				branch=1;
				temp=((unsigned int)PC)+1;
				PC=PC+((signed char)B);
				X=((unsigned char)temp);
				Y=((unsigned char)(temp>>8));
				break;
				
			case 0x5c: //ADD B,B
				temp=B+B;
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x5d: //ADDF B,B
				temp=B+B+(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x5e: //SUBF B,B
				temp=-(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x5f: //SWAP YX
				temp=(((unsigned int)Y)<<8)+((unsigned int)X);
				Y=(unsigned char)Backup2;
				X=((unsigned char)(Backup2>>8));
				Backup2=(unsigned short)temp;
				break;
			case 0x60: //TEST B,4
				f=(B&0x10)>0;
				break;
			case 0x61: //PUTF B,4
				if (f) {
					B|=0x10;
				}
				else{
					B&=0xef;
				}
				break;
			case 0x62: //MOVE B,X
				B=X;
				break;
				
			case 0x63:  //MEMORY Y, [PC+A]
				if (f)
					memory[PC+A]=Y;
				else
					Y=memory[PC+A];
				memory_inst=1;
				break;
			case 0x64: //XOR B,X
				B=B^X;
				if (B==0 || B==0xff) 
					f=1;
				else
					f=0;
				break;
			case 0x65: //OR B,X
				B=B|X;
				f=(B==0xff);
				break;
			case 0x66: //AND B,X
				B=B&X;
				f=(B==0x00);
				break;
				
			case 0x67: //MEMORY Y,[PC+X]
				if (f)
					memory[PC+X]=Y;
				else
					Y=memory[PC+X];
				memory_inst=1;
				break;	
			case 0x68: //TEST B,5
				f=(B&0x20)>0;
				break;
			case 0x69: //PUTF B,5
				if (f) {
					B|=0x20;
				}
				else{
					B&=0xdf;
				}
				break;
			case 0x6a: //RR B
				temp=B&1;
				B=(B>>1)+((f>0)<<7);
				f=temp;
				
				break;
				
			case 0x6b: //MEMORY B, [YX]
				temp=(((unsigned short)Y)<<8)+((unsigned short)(X));
				if (f)
					memory[temp]=B;
				else
					B=memory[temp];
				memory_inst=1;
				break;
				
			case 0x6c: //ADD B,X
				temp=B+X;
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x6d: //ADDF B,X
				temp=B+X+(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x6e: //SUBF B,X
				temp=B-X-(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x6f: //SKIPF
				PC+=(f>0);
				break;
			case 0x70: //TEST B,6
				f=(B&0x40)>0;
				break;
			case 0x71: //PUTF B,6
				if (f) {
					B|=0x40;
				}
				else{
					B&=0xbf;
				}
				break;
			case 0x72: //MOVE B,Y
				B=Y;
				break;
				
			case 0x73:  //MEMORY Y, [PC+B]
				if (f)
					memory[PC+B]=Y;
				else
					Y=memory[PC+B];
				memory_inst=1;
				break;
			case 0x74: //XOR B,Y
				B=B^Y;
				if (B==0 || B==0xff) 
					f=1;
				else
					f=0;
				break;
			case 0x75: //OR B,Y
				B=B|Y;
				f=(B==0xff);
				break;
			case 0x76: //AND B,Y
				B=B&Y;
				f=(B==0x00);
				break;
				
			case 0x77: //MEMORY Y,[PC+Y]
				if (f)
					memory[PC+Y]=Y;
				else
					Y=memory[PC+Y];
				memory_inst=1;
				break;	
			case 0x78: //TEST B,7
				f=(B&0x80)>0;
				break;
			case 0x79: //PUTF B,7
				if (f) {
					B|=0x20;
				}
				else{
					B&=0xdf;
				}
				break;
			case 0x7a: //INCF B
				temp=((unsigned int)B)+(f>0);
				f=temp>0xff;
				B=(unsigned char)temp;
				
				break;
				
			case 0x7b: //DECF B
				temp=((unsigned int)B)-(f>0);
				f=temp>0xff;
				B=(unsigned char)temp;
				break;
				
			case 0x7c: //ADD B,Y
				temp=B+Y;
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x7d: //ADDF B,Y
				temp=B+Y+(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x7e: //SUBF B,Y
				temp=B-Y-(f>0);
				f=(temp>0xff);
				B=(unsigned char)temp;
				break;
			case 0x7f: //FLIPF
				f=!f;
				break;
			case 0x80: //TEST X,0
				f=X&0x01;
				break;
			case 0x81: //PUTF X,0
				if (f) {
					X|=0x01;
				}
				else{
					X&=0xfe;
				}
				break;
			case 0x82: //NOP
				break;
			case 0x83: //MEMORY A, [DP+A]
				if (f)
					memory[DP+A]=A;
				else
					A=memory[DP+A];
				memory_inst=1;
				break;
			case 0x84: //XOR X,A
				X=X^A;
				f=X==0xff||X==0x00;
				break;
			case 0x85: //OR X,A
				X=X|A;
				f=X==0xff;
				break;
			
			case 0x86: //AND X,A
				X=X&A;
				f=X==0;
				break;
			case 0x87: //MEMORY A,[DP+X]
				if (f)
					memory[DP+X]=A;
				else
					A=memory[DP+X];
				memory_inst=1;
				break;
			case 0x88: //TEST X,1
				f=(X&0x02)>0;
				break;
			case 0x89: //PUTF X,1
				if (f)
					X|=0x02;
				else
					X&=0xfd;
				break;
			case 0x8a: //NOT X
				X=~X;
				f=!f;
				break;
			case 0x8b: //JUMP X
				branch=1;
				PC+=((signed char)X);
				break;
			case 0x8c: //ADD X,A
				temp=X+A;
				f=temp>0xff;
				X=(unsigned char)temp;
				break;
			case 0x8d: //ADDF X,A
				temp=X+A+(f>0);				
				f=temp>0xff;
				X=(unsigned char)temp;
				break;
			case 0x8e: //SUBF X,A
				temp=X-A-(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0x8f: //MULT
				if (f){
					temp=(unsigned int)((signed int)((signed short)
						((signed char)B))*((signed short)((signed char)A)));
					B=(unsigned char)temp;
					A=(unsigned char)(temp>>8);
				}
				else{
					temp=(unsigned int)((unsigned short)A)*((unsigned short)B);
					B=(unsigned char)temp;
					A=(unsigned char)(temp>>8);
					
				}
					
					
				break;
			case 0x90: //TEST X,2
				f=(X&0x04)>0;
				break;
			case 0x91: //PUTF X,2
				if (f) {
					X|=0x04;
				}
				else{
					X&=0xfb;
				}
				break;
			case 0x92: //MOVE X,B
				X=B;
				break;
				
			case 0x93:  //MEMORY A, [DP+B]
				if (f)
					memory[DP+B]=A;
				else
					A=memory[DP+B];
				memory_inst=1;
			case 0x94: //XOR X,B
				X=X^B;
				f=X==0xff || X==0;
				break;
			case 0x95: //OR X,B
				X=X|B;
				f=X==0xff;
				break;
			case 0x96: //AND X,B
				X=X&B;
				f=(X==0x00);
				break;
			case 0x97: //MEMORY A,[DP+Y]
				if (f)
					memory[DP+Y]=A;
				else
					A=memory[DP+Y];
				memory_inst=1;
				break;
			case 0x98: //TEST X,3
				f=(X&0x08)>0;
				break;
			case 0x99: //PUTF X,3
				if (f) {
					X|=0x08;
				}
				else{
					X&=0xf7;
				}
				break;
			case 0x9a: //UNDEFD
				branch=1;
				temp=((unsigned int)temp)+1;
				PC=PC+((signed char)Y);
				X=((unsigned char)temp);
				Y=((unsigned char)(temp>>8));
				break;
				
			case 0x9b: //JUMPLINK X
				branch=1;
				temp=((unsigned int)PC)+1;
				PC=PC+((signed char)X);
				X=((unsigned char)temp);
				Y=((unsigned char)(temp>>8));
				break;
			case 0x9c: //ADD X,B
				temp=X+B;
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0x9d: //ADDF X,B
				temp=X+B+(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0x9e: //SUBF X,B
				temp=X-B-(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0x9f: //SWAP DP
				temp=(unsigned int)Backup3;
				Backup3=DP;
				DP=(unsigned short)temp;
				break;
			case 0xa0: //TEST X,4
				f=(X&0x10)>0;
				break;
			case 0xa1: //PUTF X,4
				if (f) {
					X|=0x10;
				}
				else{
					X&=0xef;
				}
				break;
			case 0xa2: //MOVE X,X
				break;
				
			case 0xa3:  //MEMORY B, [DP+A]
				if (f)
					memory[DP+A]=B;
				else
					B=memory[DP+A];
				memory_inst=1;
			case 0xa4: //XOR X,X
				X=0;
				f=1;
				break;
			case 0xa5: //OR X,X
				f=X==0xff;
				break;
			case 0xa6: //AND X,X
				f=X==0x00;
				break;
				
			case 0xa7: //MEMORY B,[DP+X]
				if (f)
					memory[DP+X]=B;
				else
					B=memory[DP+X];
				memory_inst=1;
				break;	
			case 0xa8: //TEST X,5
				f=(X&0x20)>0;
				break;
			case 0xa9: //PUTF X,5
				if (f) {
					X|=0x20;
				}
				else{
					X&=0xdf;
				}
				break;
			case 0xaa: //RR X
				temp=X&1;
				X=(X>>1)+((f>0)<<7);
				f=temp;
				break;
				
			case 0xab: //MEMORY X, [YX]
				temp=(((unsigned short)Y)<<8)+((unsigned short)(X));
				if (f)
					memory[temp]=X;
				else
					X=memory[temp];
				memory_inst=1;
				break;
				
			case 0xac: //ADD X,X
				temp=X+X;
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0xad: //ADDF X,X
				temp=X+X+(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0xae: //SUBF X,X
				temp=X-X-(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0xaf: //OUT
				if (debug) printf("\n--Output Bit %d--\n", f>0);
				ffrisc_out(&f);
				break;
			case 0xb0: //TEST X,6
				f=(X&0x40)>0;
				break;
			case 0xb1: //PUTF X,6
				if (f) {
					X|=0x40;
				}
				else{
					X&=0xbf;
				}
				break;
			case 0xb2: //MOVE X,Y
				X=Y;
				break;
				
			case 0xb3:  //MEMORY B, [DP+B]
				if (f)
					memory[DP+B]=B;
				else
					B=memory[DP+B];
				memory_inst=1;
				break;
			case 0xb4: //XOR X,Y
				X=X^Y;
				f=(X==0xff || X==0);
				break;
			case 0xb5: //OR X,Y
				X=X|Y;
				f=(X==0xff);
				break;
			case 0xb6: //AND X,Y
				X=X&Y;
				f=(X==0x00);
				break;
				
			case 0xb7: //MEMORY B,[DP+Y]
				if (f)
					memory[DP+Y]=B;
				else
					B=memory[DP+Y];
				memory_inst=1;
				break;	
			case 0xb8: //TEST X,7
				f=(X&0x80)>0;
				break;
			case 0xb9: //PUTF X,7
				if (f) {
					X|=0x80;
				}
				else{
					X&=0x7f;
				}
				break;
			case 0xba: //INCF X
				temp=((unsigned int)X)+(f>0);
				f=temp>0xff;
				X=(unsigned char)temp;
				break;
				
			case 0xbb: //DECF X
				temp=((unsigned int)X)+(f>0);
				f=temp>0xff;
				X=(unsigned char)temp;
				break;
				
			case 0xbc: //ADD X,Y
				temp=X+Y;
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0xbd: //ADDF X,Y
				temp=X+Y+(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0xbe: //SUBF X,Y
				temp=X-Y-(f>0);
				f=(temp>0xff);
				X=(unsigned char)temp;
				break;
			case 0xbf: //IN
				ffrisc_in(&f);
				if (debug) printf("\n--INPUT Bit %d--\n", f>0);
				break;
			case 0xc0: //TEST Y,0
				f=(Y&0x01)>0;
				break;
			case 0xc1: //PUTF Y,0
				if (f) {
					Y|=0x01;
				}
				else{
					Y&=0xfe;
				}
				break;
			case 0xc2: //MOVE Y,A
				Y=A;
				break;
				
			case 0xc3:  //MEMORY X, [DP+A]
				if (f)
					memory[DP+A]=X;
				else
					X=memory[DP+A];
				memory_inst=1;
				break;
			case 0xc4: //XOR Y,A
				Y=Y^A;
				f=(Y==0xff || Y==0);
				break;
			case 0xc5: //OR Y,A
				Y=Y|A;
				f=(Y==0xff);
				break;
			case 0xc6: //AND Y,A
				Y=Y&A;
				f=(Y==0x00);
				break;
				
			case 0xc7: //MEMORY X,[DP+X]
				if (f)
					memory[DP+X]=X;
				else
					X=memory[DP+X];
				memory_inst=1;
				break;	
			case 0xc8: //TEST Y,1
				f=(Y&0x02)>0;
				break;
			case 0xc9: //PUTF Y,1
				if (f) {
					Y|=0x02;
				}
				else{
					Y&=0xfd;
				}
				break;
			case 0xca: //NOT Y
				Y=~Y;
				f=!f;
				break;
				
			case 0xcb: //JUMP Y
				branch=1;
				PC+=((signed char)Y);
				break;
				
			case 0xcc: //ADD Y,A
				temp=Y+A;
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xcd: //ADDF Y,A
				temp=Y+A+(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xce: //SUBF Y,A
				temp=Y-A-(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xcf: //SET
				f=1;
				break;
			case 0xd0: //TEST Y,2
				f=(Y&0x04)>0;
				break;
			case 0xd1: //PUTF Y,2
				if (f) {
					Y|=0x04;
				}
				else{
					Y&=0xfb;
				}
				break;
			case 0xd2: //MOVE Y,B
				Y=B;
				break;
				
			case 0xd3:  //MEMORY X, [DP+B]
				if (f)
					memory[DP+B]=X;
				else
					X=memory[DP+B];
				memory_inst=1;
				break;
			case 0xd4: //XOR Y,B
				Y=Y^B;
				f=(Y==0 || Y==0xff);
				break;
			case 0xd5: //OR Y,B
				Y=Y|B;
				f=(Y==0xff);
				break;
			case 0xd6: //AND Y,B
				Y=Y&B;
				f=(Y==0x00);
				break;
				
			case 0xd7: //MEMORY X,[DP+Y]
				if (f)
					memory[DP+Y]=X;
				else
					X=memory[DP+Y];
				memory_inst=1;
				break;	
			case 0xd8: //TEST Y,3
				f=(Y&0x08)>0;
				break;
			case 0xd9: //PUTF Y,3
				if (f) {
					Y|=0x08;
				}
				else{
					Y&=0xf7;
				}
				break;
			case 0xda: //UNDEFD
				X=(unsigned char)temp;
				B|=X;
				break;
				
			case 0xdb: //JUMPLINK Y
				branch=1;
				temp=((unsigned int)PC)+1;
				PC=PC+((signed char)Y);
				X=((unsigned char)temp);
				Y=((unsigned char)(temp>>8));
				break;	
				
			case 0xdc: //ADD Y,B
				temp=Y+B;
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xdd: //ADDF Y,B
				temp=Y+B+(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xde: //SUBF Y,B
				temp=Y-B-(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xdf: //RESET
				f=0;
				break;
			case 0xe0: //TEST Y,4
				f=(Y&0x10)>0;
				break;
			case 0xe1: //PUTF Y,4
				if (f) {
					Y|=0x10;
				}
				else{
					Y&=0xef;
				}
				break;
			case 0xe2: //MOVE Y,X
				Y=X;
				break;
				
			case 0xe3:  //MEMORY Y, [DP+A]
				if (f)
					memory[DP+A]=Y;
				else
					Y=memory[DP+A];
				memory_inst=1;
				break;
			case 0xe4: //XOR Y,X
				Y=Y^X;
				if (Y==0 || Y==0xff) 
					f=1;
				else
					f=0;
				break;
			case 0xe5: //OR Y,X
				Y=Y|X;
				f=(Y==0xff);
				break;
			case 0xe6: //AND Y,X
				Y=Y&X;
				f=(Y==0x00);
				break;
				
			case 0xe7: //MEMORY Y,[DP+X]
				if (f)
					memory[DP+X]=Y;
				else
					Y=memory[DP+X];
				memory_inst=1;
				break;	
			case 0xe8: //TEST Y,5
				f=(Y&0x20)>0;
				break;
			case 0xe9: //PUTF Y,5
				if (f) {
					Y|=0x20;
				}
				else{
					Y&=0xdf;
				}
				break;
			case 0xea: //RR Y
				temp=Y&1;
				Y=(Y>>1)+((f>0)<<7);
				f=temp;
				
				break;
				
			case 0xeb: //MEMORY Y, [YX]
				temp=(((unsigned short)Y)<<8)+((unsigned short)(X));
				if (f)
					memory[temp]=Y;
				else
					Y=memory[temp];
				memory_inst=1;
				break;
				
			case 0xec: //ADD Y,X
				temp=Y+X;
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xed: //ADDF Y,X
				temp=Y+X+(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xee: //SUBF Y,X
				temp=Y-X-(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xef: //LONGJUMP
				branch=1;
				PC=((unsigned short)Y)<<8+((unsigned short)X);
				break;
			case 0xf0: //TEST Y,6
				f=(Y&0x40)>0;
				break;
			case 0xf1: //PUTF Y,6
				if (f) {
					Y|=0x40;
				}
				else{
					Y&=0xbf;
				}
				break;
			case 0xf2: //NOP
				break;
			case 0xf3:  //MEMORY Y, [DP+B]
				if (f)
					memory[DP+B]=Y;
				else
					Y=memory[DP+B];
				memory_inst=1;
				break;
			case 0xf4: //XOR Y,Y
				Y=0;
				f=1;
				break;
			case 0xf5: //OR Y,Y
				f=(Y==0xff);
				break;
			case 0xf6: //AND Y,Y
				f=(Y==0x00);
				break;
				
			case 0xf7: //MEMORY Y,[DP+Y]
				if (f)
					memory[DP+Y]=Y;
				else
					Y=memory[DP+Y];
				memory_inst=1;
				break;	
			case 0xf8: //TEST Y,7
				f=(Y&0x80)>0;
				break;
			case 0xf9: //PUTF Y,7
				if (f) {
					Y|=0x80;
				}
				else{
					Y&=0x7f;
				}
				break;
			case 0xfa: //INCF Y
				temp=((unsigned int)Y)+(f>0);
				f=temp>0xff;
				Y=(unsigned char)temp;
				
				break;
				
			case 0xfb: //DECF Y
				temp=((unsigned int)Y)+(f>0);
				f=temp>0xff;
				Y=(unsigned char)temp;
				break;
				
			case 0xfc: //ADD Y,Y
				temp=Y+Y;
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xfd: //ADDF Y,Y
				temp=Y+Y+(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xfe: //SUBF Y,Y
				temp=-(f>0);
				f=(temp>0xff);
				Y=(unsigned char)temp;
				break;
			case 0xff: //HALT
				return;
				break;
			
			
		}
		f=f>0;
		if (!branch){
			if (c && !halfPC){
				halfPC=1;
			}
			else{
				PC++;
				halfPC=0;
			}
		}
		if (!turbo){
			for (int i=0; i<1+memory_inst; i++){
				if (slow)
					nanosleep(&slow_speed, NULL);
				else
					nanosleep(&normal_speed, NULL);
			}
		}
	}
	return;
}
