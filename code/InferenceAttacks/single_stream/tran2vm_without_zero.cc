#include<fstream>
#include<iostream>
#include<cstdlib>
#include<cstdio>
#include<stdio.h>
#include<iomanip>
#include <openssl/md5.h>
using namespace std;
bool Check0(char s[100])
{
	if((unsigned char)s[0]==0x1c)
	if((unsigned char)s[1]==0xea)
	if((unsigned char)s[2]==0xf7)
	if((unsigned char)s[3]==0x3d)
	if((unsigned char)s[4]==0xf4)
	if((unsigned char)s[5]==0x0e)
	if((unsigned char)s[6]==0x53)
	if((unsigned char)s[7]==0x1d)
	if((unsigned char)s[8]==0xf3)
	if((unsigned char)s[9]==0xbf)
	if((unsigned char)s[10]==0xb2)
	if((unsigned char)s[11]==0x6b)
	if((unsigned char)s[12]==0x4f)
	if((unsigned char)s[13]==0xb7)
	if((unsigned char)s[14]==0xcd)
	if((unsigned char)s[15]==0x95)
	if((unsigned char)s[16]==0xfb)
	if((unsigned char)s[17]==0x7b)
	if((unsigned char)s[18]==0xff)
	if((unsigned char)s[19]==0x1d)return 1;
	return 0;
}

int main(int arc,char* arcv[])
{
	/*fstream hashin;
	hashin.open("csci4430-1b.fp.sha1", ios::binary|ios::in, 0);
	if (hashin.fail())cout << "CNM!!" << endl;
	char s[30];
	memset(s, 0, 30);
	char *tmp=NULL;
	hashin.read(tmp, 12);
	hashin.read(tmp, 20);
	for (int i = 0; i<20; i++)
		cout <<setbase(16)<< (short int)tmp[i]<<" ";*/
	if(arc<2)printf("Invalid Input\n");
	const char *tp=arcv[1];
	ifstream fin(tp);//,ifstream::binary);
	char tmp [100];
	char tmp2[100];
	fin.read(tmp, 12);
	printf("Chunk Hash\t\tChunk Size (bytes)\tCompression Ratio (tenth)\n");
	while (fin.read(tmp, 20))
	{
		if(Check0(tmp)){fin.read(tmp,12);continue;}
		MD5((unsigned char*)tmp,20,(unsigned char*)tmp2);
		for (int i = 0; i<6; i++)
		{
			printf("%02x", (unsigned char)tmp2[i]); if (i != 5)printf(":");
		}
	
		printf("\t%d\t0\n",4*1024);
		fin.read(tmp, 12);
	}
	return 0;
}
