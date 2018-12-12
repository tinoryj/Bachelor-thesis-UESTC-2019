#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>

#include "leveldb/db.h"

int FPSIZE = 6;
 
using namespace std;

int insertFCdb(string key);
int insertSFdb(string size, string key);

// data counter--------------------

uint64_t totalLogicChunkNumber = 0;
uint64_t totalUniqueChunkNumber = 0;
// store DBs-----------------------
leveldb::DB *FCdb; //fp-count pair
leveldb::DB *SFdb; //size-fp pair

//init leveldb======================================================
int initDB(char *db1, char *db2){

    leveldb::Options options;
    options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(options, db1, &FCdb);
    assert(status.ok());
    assert(FCdb != NULL);

	status = leveldb::DB::Open(options, db2, &SFdb);
    assert(status.ok());
    assert(SFdb != NULL);
}

int readInChunks(FILE *fp) {

	char read_buffer[256];
	char *item;

	fgets(read_buffer, 256, fp);// skip title line
	while (fgets(read_buffer, 256, fp)) {
		
		totalLogicChunkNumber++;
		cout<<totalLogicChunkNumber<<endl;
		
		char hash[FPSIZE];
		memset(hash, 0, FPSIZE);

		item = strtok(read_buffer, ":\t\n ");
		int index = 0;
		while (item != NULL && index < FPSIZE){
			hash[index++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}
		string chunkHash(hash,FPSIZE);
		uint64_t size = atoi((const char*)item);   //string-->int
		string sizeStr = to_string(size);
		if (insertFCdb(chunkHash) == 1) {
			totalUniqueChunkNumber++;
			insertSFdb(sizeStr, chunkHash);
		}
	}
}


int insertFCdb(string key) {

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

int insertSFdb(string size, string key) {

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

	assert(argc >= 4); 
	//filename  CFCdb CSFdb MFCdb MSFdb

	initDB(argv[2], argv[3]);
	FILE *fp = NULL;
	fp = fopen(argv[1], "r");
	assert(fp != NULL);
	readInChunks(fp);

	cout<<"File Name "<<argv[1]<<endl;
	cout<<"totalLogicChunkNumber "<<totalLogicChunkNumber<<endl;
	cout<<"totalUniqueChunkNumber "<<totalUniqueChunkNumber<<endl;

	return 0;
}
