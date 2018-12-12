#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>
#include <fstream>
#include "leveldb/db.h"

int FPSIZE = 6;
 
using namespace std;

int insertFCDB(string metaStr, string chunkHash);


// data counter--------------------

uint64_t totalLogicChunkNumber = 0;

// store DBs-----------------------
leveldb::DB *FCdb;  // file chunk db

//init leveldb======================================================
int initDB(char *db1) {

    leveldb::Options options;
    options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(options, db1, &FCdb);
    assert(status.ok());
    assert(FCdb != NULL);

}

int readInChunks(FILE *fp, FILE *typeFp) {

	char read_buffer[256];
    char typeBuffer[50];
    char typeNumBuffer[256];
    
	fgets(read_buffer, 256, fp);// skip title line
    uint64_t fileIndex = 0;

    //int cnt = 2000;
    while (fgets(typeBuffer,256,typeFp)) {

		//cnt++;
        //cout<<cnt<<endl;
		char *itemN = strtok(typeBuffer,":\t\n");
		string typeName(itemN); 

        fgets(typeNumBuffer, 256, typeFp);
        char *tmp;
        tmp = strtok(typeNumBuffer, ":\t\n");
        uint64_t chunkNumber = atoi((const char*)tmp);
        
        char typeHash[FPSIZE];
   		unsigned char md5full[64];
    	MD5((unsigned char*)itemN, strlen(itemN), md5full);
    	memcpy(typeHash, md5full, FPSIZE);
        string typeHashStr(typeHash,FPSIZE);

		char mk1[256];
        string mKey = typeHashStr;
        sprintf(mk1,"%02x:%02x:%02x:%02x:%02x:%02x\t",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF);
        //cout<<itemN<<"\t"<<chunkNumber<<"\t"<<mk1<<endl;


        if (typeHashStr.length() != 6) 
            cout<<"error"<<endl;

        //index + chunknum + filetypehash
        string indexStr((char*)(&fileIndex), sizeof(uint64_t));
        string chunkCountStr((char*)(&chunkNumber), sizeof(uint64_t));
        string metaStr = indexStr + chunkCountStr + typeHashStr;

        fileIndex++;
        for (uint64_t i = 0; i < chunkNumber; i++) {
            
			fgets(read_buffer, 256, fp);
            totalLogicChunkNumber++;
		    //cout<<totalLogicChunkNumber<<endl;
		    char hash[FPSIZE];
		    memset(hash, 0, FPSIZE);
			char *item;
		    item = strtok(read_buffer, ":\t\n ");
		    int index = 0;
	    	while (item != NULL && index < FPSIZE){
			    hash[index++] = strtol(item, NULL, 16);
			    item = strtok(NULL, ":\t\n");
		    }

		    string chunkHash(hash,FPSIZE);

		    insertFCDB(metaStr, chunkHash);
        }
    }
}


int insertFCDB(string metaStr, string chunkHash) {

	string exs="";
    leveldb::Status status = FCdb->Get(leveldb::ReadOptions(), metaStr, &exs);
    if (status.ok() == 0) {

        uint64_t count = 1;
        string chunkCount((char*)(&count), sizeof(uint64_t));
        string insertStr = chunkHash + chunkCount;
        if (insertStr.size() % 14 != 0) {
			cout<<"insert key error"<<endl;
			exit(0);
		}
        status = FCdb->Put(leveldb::WriteOptions(), metaStr ,insertStr); 
		return 1;
    }
	else {

        uint64_t uniqueChunkNum = exs.size();
        bool flag = true;
		for (uint64_t i = 0; i < uniqueChunkNum; i += (FPSIZE+sizeof(uint64_t))) {
			
			string key = exs.substr(i,(FPSIZE+sizeof(uint64_t)));
            string hashTmp = key.substr(0,FPSIZE);
            string countTmp = key.substr(6,sizeof(uint64_t));
            if (memcmp(hashTmp.c_str(), chunkHash.c_str(), FPSIZE) == 0) {
            
                uint64_t count = *(uint64_t*)countTmp.c_str();
                count++;
                string chunkCount((char*)(&count), sizeof(uint64_t));
                exs.replace(i+FPSIZE, 8, chunkCount);
                flag = false;
                break;
            }
            else {
                continue;
            }
		}
        string insertKey;
        if (flag) {
            uint64_t count = 1;
            string chunkCount((char*)(&count), sizeof(uint64_t));
            insertKey = exs + chunkHash + chunkCount;
        }
        else {
            insertKey = exs;
        }

		if (insertKey.size() % 14 != 0) {
			cout<<"insert key error"<<endl;
			exit(0);
		}
		status = FCdb->Put(leveldb::WriteOptions(), metaStr, insertKey);
		return -1; 
	}
}


int main (int argc, char *argv[]){

	//assert(argc >= 5); 
	//filename CFCdb CSFdb MFCdb MSFdb

	initDB(argv[3]);
	FILE *fp = NULL;
    FILE *typeFp = NULL;
	fp = fopen(argv[1], "r");
    typeFp = fopen(argv[2],"r");
    assert(typeFp != NULL);
	assert(fp != NULL);
	readInChunks(fp,typeFp);
	cout<<"totalLogicChunkNumber "<<totalLogicChunkNumber<<endl;

	return 0;
}
