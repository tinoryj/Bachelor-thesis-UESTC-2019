#include <bits/stdc++.h>
#include <string>
#include <openssl/bn.h>
#include <openssl/md5.h>
#include <sys/time.h>
int FPSIZE = 6;
 
using namespace std;

//===================================================================

void outPutChunks(FILE *fp,FILE *mk) {

	char read_buffer[256];
	char *item;
	fgets(read_buffer, 256, fp);// skip title line
    
    fprintf(mk,"Chunk Hash   Chunk Size (bytes) 	Compression Ratio (tenth)\n");
	while (fgets(read_buffer, 256, fp)) {
		
		// a new chunk
		// store chunk hash and size
		char hash[FPSIZE];
		memset(hash, 0, FPSIZE);

		item = strtok(read_buffer, ":\t\n ");
		int index = 0;
		while (item != NULL && index < FPSIZE){
			hash[index++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}
		uint64_t size = atoi((const char*)item);   //string-->int
		char hash2[FPSIZE];
   		unsigned char md5full[64];
    	MD5((unsigned char*)hash, FPSIZE, md5full);
    	memcpy(hash2,md5full,FPSIZE);
		//fprintf("%02x\t10\n"hash2[0],);
		string mKey(hash2, FPSIZE); //get key
        fprintf(mk,"%02x:%02x:%02x:%02x:%02x:%02x\t%d\t10\n",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF,size);
	}
}


int main (int argc, char *argv[]){
    
	assert(argc >= 2); 
    FILE *fp = NULL;
	fp = fopen(argv[1], "r");
	assert(fp != NULL);
    FILE *mk = NULL;
    char name[256];
    sprintf(name,"%s.enc",argv[1]);
    mk = fopen(name,"w");
	outPutChunks(fp,mk);
	return 0;
}
