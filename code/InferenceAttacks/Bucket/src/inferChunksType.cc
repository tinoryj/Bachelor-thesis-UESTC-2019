#include <bits/stdc++.h>
#include <openssl/bn.h>
#include <openssl/md5.h>
#include <sys/time.h>

#include "leveldb/db.h"
int FPSIZE = 6;
 
using namespace std;

struct FC {
	string key;
	uint64_t count = 0;
};
struct cmp {
	bool operator()(FC a, FC b) {
		return a.count < b.count; 
	}
};
priority_queue<FC, vector<FC>, cmp > cQue;
priority_queue<FC, vector<FC>, cmp > mQue;
// data counter--------------------

uint64_t rawInferChunkNumber = 0;
uint64_t uniqueInferChunkNumber = 0;
uint64_t correctInferChunkNumber = 0;

// store DBs-----------------------
leveldb::DB *CFCdb; //fp-count pair
leveldb::DB *CSFdb; //size-fp pair
leveldb::DB *MFCdb; //C chunk
leveldb::DB *MSFdb; //M chunk
leveldb::DB *OUTdb; //out put correct pair

void initDB(char *db1, char *db2, char *db3, char *db4, char *db5){

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, db1, &CFCdb);
    assert(status.ok());
    assert(CFCdb != NULL);

	status = leveldb::DB::Open(options, db2, &CSFdb);
    assert(status.ok());
    assert(CFCdb != NULL);

	status = leveldb::DB::Open(options, db3, &MFCdb);
    assert(status.ok());
    assert(CFCdb != NULL);

	status = leveldb::DB::Open(options, db4, &MSFdb);
    assert(status.ok());
    assert(CFCdb != NULL);

	status = leveldb::DB::Open(options, db5, &OUTdb);
    assert(status.ok());
    assert(OUTdb != NULL);

}


void inferTest(char *input) {

	char *tmp;
    tmp = strtok(input, ":\t\n");

    //string typeSearch(input);
    char typeHash[FPSIZE];
   	unsigned char md5full[64];
    MD5((unsigned char*)tmp, strlen(tmp), md5full);
    memcpy(typeHash, md5full, FPSIZE);
    string typeHashStr(typeHash,FPSIZE);

	char mk1[256];
    string mKey = typeHashStr;
    sprintf(mk1,"%02x:%02x:%02x:%02x:%02x:%02x",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF);
    cout<<mk1<<endl;


    if (typeHashStr.length() != 6) 
        cout<<"error"<<endl;

	string currentSzie;
    leveldb::Iterator* it = CSFdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()){

		string mTempFpSet;
		currentSzie = it->key().ToString();
		leveldb::Status s = MSFdb->Get(leveldb::ReadOptions(), currentSzie, &mTempFpSet);

		if (s.ok() == 0) {

			continue;
    	}
		string cTempFpSet = it->value().ToString();
		uint64_t cFpNum = cTempFpSet.size();
		uint64_t mFpNum = mTempFpSet.size();

		for (uint64_t i = 0; i < cFpNum; i += FPSIZE) {
			FC newFC;
			string key = cTempFpSet.substr(i,FPSIZE);
            key = key + typeHashStr;
            if (key.length() != 12) { 
                cout<<"error"<<endl;
                exit(0);
            }
			newFC.key = key;
			string count;
			leveldb::Status s = CFCdb->Get(leveldb::ReadOptions(), key, &count);
            if (s.ok() == 0) {
                continue;
            }
            else {
                newFC.count = atoi((const char*)count.c_str());
			    cQue.push(newFC);
            }
		}
		for (uint64_t i = 0; i < mFpNum; i += FPSIZE) {
			FC newFC;
			string key = mTempFpSet.substr(i,FPSIZE);
            key = key + typeHashStr;
            if (key.length() != 12) { 
                cout<<"error"<<endl;
                exit(0);
            }
			newFC.key = key;
			string count;
			leveldb::Status s = MFCdb->Get(leveldb::ReadOptions(), key, &count);
            if (s.ok() == 0) {
                continue;
            }
            else {
			    newFC.count = atoi((const char*)count.c_str());
			    mQue.push(newFC);
            }
		}
		
		uint64_t inferSize;
		if (mQue.size() <= cQue.size()) 
			inferSize = mQue.size();
		else 
			inferSize = cQue.size();
        //cout<<inferSize<<endl;
		for (uint64_t i = 0; i < inferSize; i++) {
			
			uniqueInferChunkNumber++;

			char hash2[FPSIZE];
            string calHash = mQue.top().key.substr(0,FPSIZE);
            unsigned char md5full[64];
    		MD5((unsigned char*)calHash.c_str(), FPSIZE,md5full);
    		memcpy(hash2,md5full,FPSIZE);

			string mKey(hash2, FPSIZE); //get key

            /*
            char mk1[256];
        	sprintf(mk1,"%02x:%02x:%02x:%02x:%02x:%02x\t",mKey[0]&0xFF,mKey[1]&0xFF,mKey[2]&0xFF,mKey[3]&0xFF,mKey[4]&0xFF,mKey[5]&0xFF);
			char mk2[256];
        	sprintf(mk2,"%02x:%02x:%02x:%02x:%02x:%02x\t",mQue.top().key[0]&0xFF,mQue.top().key[1]&0xFF,mQue.top().key[2]&0xFF,mQue.top().key[3]&0xFF,mQue.top().key[4]&0xFF,mQue.top().key[5]&0xFF);
			char mk3[256];
        	sprintf(mk3,"%02x:%02x:%02x:%02x:%02x:%02x\t",cQue.top().key[0]&0xFF,cQue.top().key[1]&0xFF,cQue.top().key[2]&0xFF,cQue.top().key[3]&0xFF,cQue.top().key[4]&0xFF,cQue.top().key[5]&0xFF);
			cout<<mk1<<" "<<mk2<<"\t"<<mk3<<endl;
            */



            string cData = cQue.top().key.substr(0,FPSIZE);

			if (memcmp(mKey.c_str(), cData.c_str(), FPSIZE) == 0) {
				rawInferChunkNumber += cQue.top().count;
				correctInferChunkNumber++;

    			leveldb::Status outStatus =  OUTdb->Put(leveldb::WriteOptions(),calHash,cData); 
				//key-message value-cipher
			}
			mQue.pop();
			cQue.pop();
		}		
    }
    assert(it->status().ok());
    delete it;
}

int main (int argc, char *argv[]){

	//assert(argc>=6);

	initDB(argv[1], argv[2], argv[3], argv[4],argv[5]);

	inferTest(argv[6]);

	cout<<"rawinferchunknumber "<<rawInferChunkNumber<<endl;
	cout<<"uniqueinferchunknumber "<<uniqueInferChunkNumber<<endl;
	cout<<"correctinferchunknumber "<<correctInferChunkNumber<<endl;

	return 0;
}
