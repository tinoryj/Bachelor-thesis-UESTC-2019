#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>
#include <sys/time.h>

#include "leveldb/db.h"

#define FPSIZE 6
using namespace std;

// store DBs-----------------------
leveldb::DB *FCdb; //fp-count pair
leveldb::DB *ANSdb; //chunk pair



void initDB(char *FC, char *ANS){

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, FC, &FCdb);
    assert(status.ok());
    assert(FCdb != NULL);

    status = leveldb::DB::Open(options, ANS, &ANSdb);
    assert(status.ok());
    assert(ANSdb != NULL);
}


void FC() {

    freopen("case.txt", "w", stdout);
    leveldb::Iterator* it = FCdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){

        string key = it->key().ToString();
        uint64_t index = *(uint64_t*)key.substr(0,8).c_str();
        uint64_t count = *(uint64_t*)key.substr(8,8).c_str();  
        char mk1[256];
        string mKey = key.substr(16,6);
        sprintf(mk1,"%02x:%02x:%02x:%02x:%02x:%02x\t",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF);
        cout<<index<<"\t"<<count<<"\t"<<mk1<<"\t\t";

        string mValue = it->value().ToString();
        uint64_t len = mValue.length();

		for (uint64_t i = 0; i < len; i += 14) {

            char mk2[256];
            sprintf(mk2,"%02x:%02x:%02x:%02x:%02x:%02x\t",mValue[0+i]&0xFF,mValue[1+i]&0xFF,mValue[2+i]&0xFF,mValue[3+i]&0xFF,mValue[4+i]&0xFF,mValue[5+i]&0xFF);
            uint64_t countTmp = *(uint64_t*)mValue.substr(i+6,8).c_str();
            cout<<mk2<<" "<<countTmp<<" ";
		}
        cout<<endl;
    }
    assert(it->status().ok());
    delete it;
}

void ANS(){

    freopen("ans.txt", "w", stdout);
    leveldb::Iterator* it = ANSdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){

        char mk1[256];
        string mKey = it->key().ToString();
        sprintf(mk1,"%02x:%02x:%02x:%02x:%02x:%02x\t",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF);

        char mk2[256];
        string mValue = it->value().ToString();
        sprintf(mk2,"%02x:%02x:%02x:%02x:%02x:%02x\t",mValue[0]&0xFF,mValue[1]&0xFF,mValue[2]&0xFF,mValue[3]&0xFF,mValue[4]&0xFF,mValue[5]&0xFF);
            
        cout<<mk1<<"\t"<<mk2<<endl;
    }
    assert(it->status().ok());
    delete it;

}


int main (int argc, char *argv[]){

	initDB(argv[1],argv[2]);
    FC();
    ANS();
	return 0;
}
