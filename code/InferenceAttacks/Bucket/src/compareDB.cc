#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>
#include <fstream>
#include "leveldb/db.h"

int FPSIZE = 6;
int overHold = 10;
using namespace std;


int insertFCDB(string metaStr, string chunkHash);


// data counter--------------------
uint64_t correctLogicChunkNumber = 0;
uint64_t totalLogicChunkNumber = 0;
uint64_t correctUniqueChunkNumber = 0;
uint64_t totalUniqueChunkNumber = 0;

uint64_t uniqueOver10 = 0;
uint64_t logicOver10 = 0;
uint64_t allOver10 = 0;

uint64_t totalFileNumber = 0;
// store DBs-----------------------
leveldb::DB *FCdb;  // file chunk db
leveldb::DB *ANSdb;  // chunk pair db

//init leveldb======================================================
int initDB(char *db1, char *db2) {

    leveldb::Options options;
    options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(options, db1, &FCdb);
    assert(status.ok());
    assert(FCdb != NULL);

    status = leveldb::DB::Open(options, db2, &ANSdb);
    assert(status.ok());
    assert(ANSdb != NULL);
}

int compareDB(char *input){

    char *tmp;
    tmp = strtok(input, ":\t\n");
    char typeHash[FPSIZE];
   	unsigned char md5full[64];
    MD5((unsigned char*)tmp, strlen(tmp), md5full);
    memcpy(typeHash, md5full, FPSIZE);
    string typeHashStr(typeHash,FPSIZE);

    leveldb::Iterator* it = FCdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){

        string key = it->key().ToString();
        uint64_t index = *(uint64_t*)key.substr(0,8).c_str();
        uint64_t count = *(uint64_t*)key.substr(8,8).c_str();  
        //char mk1[256];
        string mKey = key.substr(16,6);
        //sprintf(mk1,"%02x:%02x:%02x:%02x:%02x:%02x\t",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF);
        //cout<<index<<"\t"<<count<<"\t"<<mk1<<"\t\t";

        if (memcmp(mKey.c_str(), typeHashStr.c_str(), FPSIZE) != 0) {
            continue;
        }
        else {
            totalFileNumber++;
            totalLogicChunkNumber += count;
            
            string mValue = it->value().ToString();
            uint64_t len = mValue.length();

            // ratio
            uint64_t uniqueTmp = len / 14;
            uint64_t correctUniqueTmp = 0;
            uint64_t correctLogicTmp = 0;

            totalUniqueChunkNumber += len / 14;

		    for (uint64_t i = 0; i < len; i += 14) {

                string chunkHashTmp = mValue.substr(0+i,6);
                uint64_t countTmp = *(uint64_t*)mValue.substr(i+6,8).c_str();
                string exs = "";
                leveldb::Status status = ANSdb->Get(leveldb::ReadOptions(), chunkHashTmp, &exs);
                if (status.ok() == 0) {
                    continue;
                }
                else {
                    correctUniqueChunkNumber++;
                    correctLogicChunkNumber += countTmp;
                    correctLogicTmp += countTmp;
                    correctUniqueTmp++;
                }
		    }

            if (correctLogicTmp * overHold >= count) {
                logicOver10++;
            }
            if (correctUniqueTmp * overHold >= uniqueTmp) {
                uniqueOver10++;
            }
            if ( (correctLogicTmp * overHold >= count) && (correctUniqueTmp * overHold >= uniqueTmp) )  {
                allOver10++;
            }
        }
    }
    assert(it->status().ok());
    delete it;
}

int main (int argc, char *argv[]){

	//assert(argc >= 5); 
	//filename CFCdb CSFdb MFCdb MSFdb

	initDB(argv[1],argv[2]);
    compareDB(argv[3]);
    /*
    cout<<"correctLogicChunkNumber\t"<<correctLogicChunkNumber<<endl;
    cout<<"totalLogicChunkNumber\t"<<totalLogicChunkNumber<<endl;
    cout<<"correctUniqueChunkNumber\t"<<correctUniqueChunkNumber<<endl;
    cout<<"totalUniqueChunkNumber\t"<<totalUniqueChunkNumber<<endl;

    cout<<"uniqu over "<<overHold<<"\t"<<uniqueOver10<<endl;
	cout<<"logic over "<<overHold<<"\t"<<logicOver10<<endl;
    cout<<"all over "<<overHold<<"\t"<<allOver10<<endl;
    
    cout<<"total file number in that type\t"<<totalFileNumber<<endl;
    */
    cout<<correctLogicChunkNumber<<endl;
    cout<<totalLogicChunkNumber<<endl;
    cout<<correctUniqueChunkNumber<<endl;
    cout<<totalUniqueChunkNumber<<endl;

    cout<<uniqueOver10<<endl;
	cout<<logicOver10<<endl;
    cout<<allOver10<<endl;
    
    cout<<totalFileNumber<<endl;
    return 0;

}
