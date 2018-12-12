#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>

#include "leveldb/db.h"
#define size0 50000
int FPSIZE = 6;
#define FP_SIZE 12
using namespace std;

// store struct -----------------------------
// fingerPrint-count pair

// data counter--------------------

uint64_t totalLogicChunkNumber = 0;
// store DBs-----------------------
leveldb::DB *db; 

//init leveldb======================================================
int initDB(){

    leveldb::Options options;
    options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(options, "./count", &db);
    assert(status.ok());
    assert(db != NULL);
}
int readInChunks(FILE *fp) {

	char read_buffer[256];
	char *item;
    freopen("Basic.txt","w",stdout);
	fgets(read_buffer, 256, fp);// skip title line
	int cnt = 50000;

	while (fgets(read_buffer, 256, fp)) {
		
		cnt--;
		totalLogicChunkNumber++;
		//cout<<totalLogicChunkNumber<<endl;

		string chunkHash;
		item = strtok(read_buffer, ":\t\n ");
		int index = 0;
		while (item != NULL && index < 6){

            cout<<item;
			chunkHash += item;
			index++;
			item = strtok(NULL, ":\t\n");
		}
        cout<<endl;
        uint64_t size = atoi((const char*)item);   //string-->int
		string indexStr = to_string(totalLogicChunkNumber);
        leveldb::Status status = db->Put(leveldb::WriteOptions(),indexStr,chunkHash);
        if(status.ok() == 0) {
            cout<<"error"<<endl;
        }
    } 
}

int outPutdb() {

	string exs="";
    freopen("MFC.txt", "w", stdout);
	uint64_t count = totalLogicChunkNumber;
    //cout<<count<<endl;
    for (int i = 1; i <= count; i++){
        string key = to_string(i);
        leveldb::Status status = db->Get(leveldb::ReadOptions(),key,&exs);
        if (status.ok() == 0) {
            cout << "error"<<endl;
        }
        cout<<exs<<endl;
    }
}
void MFC() {

    freopen("MFC.txt", "w", stdout);
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){
        cout<<it->key().ToString()<<"\t";
		cout<<it->value().ToString()<<endl;
    }
    assert(it->status().ok());
    delete it;
}
int main (int argc, char *argv[]){

	FILE *fp = NULL;
	fp = fopen(argv[1], "r");
    initDB();
	readInChunks(fp);
    outPutdb();
    //MFC();
	//cout<<"totalLogicChunkNumber "<<totalLogicChunkNumber<<endl;

	return 0;
}
