#include <bits/stdc++.h>
#include "leveldb/db.h"

int FPSIZE = 6;
int bucketSize = 16;
using namespace std;
int insertSFDB(string size, string chunkHash);


// data counter--------------------

uint64_t totalLogicChunkNumber = 0;

// store DBs-----------------------
leveldb::DB *SFdb; //M chunk
leveldb::DB *NEWSFdb; //M chunk
//init leveldb======================================================
int initDB(char *db1, char *db2) {

    leveldb::Options options;
    options.create_if_missing = true;

	status = leveldb::DB::Open(options, db2, &SFdb);
    assert(status.ok());
    assert(SFdb != NULL);

    status = leveldb::DB::Open(options, db2, &NEWSFdb);
    assert(status.ok());
    assert(NEWSFdb != NULL);
}

int readInChunks() {

    leveldb::Iterator* it = SFdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){
        string mKey = it->value().ToString();
        string sizeStr = it->key().ToString();
        uint64_t size = atoi((const char*)sizeStr.c_str());
        uint64_t tmpSize = ceil((double)size/bucketSize);
	    string tmpSizeStr = to_string(tmpSize);
        insertSFDB(tmpSizeStr, mKey);
    }
    assert(it->status().ok());
    delete it;
}

int insertSFDB(string size, string key) {

	string exs="";
    leveldb::Status status = NEWSFdb->Get(leveldb::ReadOptions(),size,&exs);
    if (status.ok() == 0) {

        status = NEWSFdb->Put(leveldb::WriteOptions(),size,key); 
		return 1;
    }
	else {
		string insertKey = exs + key;
		if (insertKey.size() % 6 != 0) {
			cout<<"insert key error"<<endl;
			exit(0);
		}
		status = NEWSFdb->Put(leveldb::WriteOptions(), size, insertKey);
		return -1; 
	}
}


int main (int argc, char *argv[]){

	//old db , new db

	initDB(argv[1], argv[2]);
	readInChunks();

	return 0;
}
