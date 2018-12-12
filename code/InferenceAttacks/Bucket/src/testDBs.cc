#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>

#include "leveldb/db.h"

int FPSIZE = 6;
int DBNUM = 3;
using namespace std;

int insertdbs(string key, string count);

// store struct -----------------------------
// fingerPrint-count pair

// data counter--------------------

uint64_t totalLogicChunkNumber = 0;

// store DBs-----------------------
leveldb::DB *FCdb[50];

//init leveldb======================================================
int initDB(){

    leveldb::Options options;
    options.create_if_missing = true;
    
    for (int i = 0; i < DBNUM; i++) {
        string fileName = "./";
        fileName += to_string(i);
        leveldb::Status status = leveldb::DB::Open(options, fileName, &FCdb[i]);
        assert(status.ok());
        assert(FCdb[i] != NULL);
    }
}
int insertFCdb(string key, int i) {

	string exs="";
	uint64_t count;
    leveldb::Status status = FCdb[i]->Get(leveldb::ReadOptions(),key,&exs);
    if (status.ok() == 0) {
		count = 1;
		string countInsert = "1";
        status = FCdb[i]->Put(leveldb::WriteOptions(),key,countInsert); 
		return 1;
    }
	else {
		count = atoi((const char*)exs.c_str());
		count++;
		string countInsert = to_string(count);
		status = FCdb[i]->Put(leveldb::WriteOptions(),key,countInsert); 
		return count;
	}
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
        for (int i = 0; i < DBNUM; i++) {
            insertFCdb(chunkHash, i);
        }
	}
}

int main (int argc, char *argv[]){

    initDB();
	FILE *fp = NULL;
	fp = fopen(argv[1], "r");
	assert(fp != NULL);
	readInChunks(fp);
	cout<<"totalLogicChunkNumber "<<totalLogicChunkNumber<<endl;
	return 0;
}
