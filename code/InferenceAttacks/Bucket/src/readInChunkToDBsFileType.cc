#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>
#include <fstream>
#include "leveldb/db.h"

int FPSIZE = 6;
 
using namespace std;
int insertTypeDB_Pair(string typeName, string typeHash);
int insertTypeDB(string typeName, uint64_t chunkNum);
int insertSFDB(string size, string chunkHash);
int insertFCDB(string chunkHash);


// data counter--------------------

uint64_t totalLogicChunkNumber = 0;

// store DBs-----------------------
leveldb::DB *FCdb; //C chunk
leveldb::DB *SFdb; //M chunk
leveldb::DB *Tdb; //type 
leveldb::DB *TPdb;

//init leveldb======================================================
int initDB(char *db1, char *db2, char *db3) {

    leveldb::Options options;
    options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(options, db1, &FCdb);
    assert(status.ok());
    assert(FCdb != NULL);

	status = leveldb::DB::Open(options, db2, &SFdb);
    assert(status.ok());
    assert(SFdb != NULL);

	status = leveldb::DB::Open(options, db3, &Tdb);
    assert(status.ok());
    assert(Tdb != NULL);
	
	status = leveldb::DB::Open(options, "./TP", &TPdb);
    assert(status.ok());
    assert(TPdb != NULL);
}

int readInChunks(FILE *fp, FILE *typeFp) {

	char read_buffer[256];
    char typeBuffer[50];
    char typeNumBuffer[256];
    
	fgets(read_buffer, 256, fp);// skip title line

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
		insertTypeDB(typeName, chunkNumber);
		insertTypeDB_Pair(typeName, typeHashStr);

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
            string inputHash = chunkHash + typeHashStr;

		    uint64_t size = atoi((const char*)item);   //string-->int
			uint64_t bucketSize = ceil((double)size/16);
		    string sizeStr = to_string(bucketSize);
		    if (insertFCDB(inputHash) == 1) {
			    insertSFDB(sizeStr, chunkHash);
		    }
        }
    }
}


int insertFCDB(string key) {

	string exs="";
	uint64_t count;
    leveldb::Status status = FCdb->Get(leveldb::ReadOptions(),key,&exs);
    if (status.ok() == 0) {
		count = 1;
		string countInsert = "1";
        status = FCdb->Put(leveldb::WriteOptions(),key,countInsert); 
		return 1;
    }
	else {
		count = atoi((const char*)exs.c_str());
		count++;
		string countInsert = to_string(count);
		status = FCdb->Put(leveldb::WriteOptions(),key,countInsert); 
		return count;
	}
}

int insertTypeDB(string typeName, uint64_t chunkNum) {

	string exs="";
	uint64_t count = 0;
    leveldb::Status status = Tdb->Get(leveldb::ReadOptions(),typeName,&exs);
    if (status.ok() == 0) {
		string countInsert = to_string(chunkNum);
        status = Tdb->Put(leveldb::WriteOptions(),typeName,countInsert); 
		return 1;
    }
	else {
		count = atoi((const char*)exs.c_str());
		count += chunkNum;
		string countInsert = to_string(count);
		status = Tdb->Put(leveldb::WriteOptions(),typeName,countInsert); 
		return count;
	}
}

int insertTypeDB_Pair(string typeName, string typeHash) {

	string exs="";
	uint64_t count;
    leveldb::Status status = TPdb->Get(leveldb::ReadOptions(),typeName,&exs);
    if (status.ok() == 0) {
        status = TPdb->Put(leveldb::WriteOptions(),typeName,typeHash); 
		return 1;
    }
	else {
		return 0;
	}
}

int insertSFDB(string size, string key) {

	string exs="";
    leveldb::Status status = SFdb->Get(leveldb::ReadOptions(),size,&exs);
    if (status.ok() == 0) {

        status = SFdb->Put(leveldb::WriteOptions(),size,key); 
		return 1;
    }
	else {
		string insertKey = exs + key;
		if (insertKey.size() % 6 != 0) {
			cout<<"insert key error"<<endl;
			exit(0);
		}
		status = SFdb->Put(leveldb::WriteOptions(), size, insertKey);
		return -1; 
	}
}


int main (int argc, char *argv[]){

	assert(argc >= 5); 
	//filename CFCdb CSFdb MFCdb MSFdb

	initDB(argv[3], argv[4],argv[5]);
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
