#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>

#include "leveldb/db.h"
int FPSIZE = 6;
#define FP_SIZE 12
using namespace std;

// store struct -----------------------------
// fingerPrint-count pair

// data counter--------------------

uint64_t totalLogicChunkNumber = 0;
uint64_t totalUniqueChunkNumber = 0;

// store DBs-----------------------
leveldb::DB *db; 

//init leveldb======================================================
int initDB(char *dbname){

    leveldb::Options options;
    options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(options, dbname, &db);
    assert(status.ok());
    assert(db != NULL);
}

void MFC() {

    freopen("MFC.txt", "w", stdout);
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){
        totalUniqueChunkNumber++;
		string countStr = it->value().ToString();
        totalLogicChunkNumber += atoi((const char*)countStr.c_str());
    }
    assert(it->status().ok());
    delete it;
}
int main (int argc, char *argv[]){

    initDB(argv[1]);
    MFC();
	cout<<"totalLogicChunkNumber "<<totalLogicChunkNumber<<endl;
    cout<<"totalUniqueChunkNumber "<<totalUniqueChunkNumber<<endl;
	return 0;
}
