#include <assert.h>
#include <stdio.h>
#include <queue>
#include <stack>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <deque>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <inttypes.h>
#include "leveldb/db.h"
#include <cmath>

using namespace std;

#define FP_SIZE 6
#define MAX_UO 4000
#define MAX_UT 4000
#define ENTROPY_THESHOLD 1
#define RADIUS 2
#define MAX_INT 9999999
#define E_ATTACK 0
#define MIN_ENTROPY 0
uint64_t TH_K;
uint64_t INIT;
uint64_t QUEUE_LIMIT;
double LEAK_RATE;
uint64_t smallChunkAmount = 0;
uint64_t bigChunkError = 0;
uint64_t smallChunkError = 0;
struct node
{
	char key[FP_SIZE];
	uint64_t count;
};

struct cmp
{
	bool operator()(node a, node b) 
	{
		return a.count > b.count;
	}
};

leveldb::DB *origin;	// F_M
leveldb::DB *left_o;	// L_M
leveldb::DB *right_o;	// R_M

leveldb::DB *target;	// F_C
leveldb::DB *left_t;	// L_C
leveldb::DB *right_t;	// R_C

// unique db is used to record all inferred chunks
leveldb::DB *uniq;

priority_queue<node, vector<node>, cmp > pq;
priority_queue<node, vector<node>, cmp > pc;

// q_o and q_t implement inferred set G
queue<node> q_o;
queue<node> q_t;


uint64_t total = 0;
uint64_t common = 0;
uint64_t correct = 0;
uint64_t uniq_count = 0;
uint64_t involve = 0;
uint64_t leak = 0;

void init_db(std::string db_name, int type) 
{
	leveldb::DB *db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, db_name.c_str(), &db);
	assert(status.ok());
	assert(db != NULL);

	if(type == 1) origin = db;
	if(type == 11) left_o = db;
	if(type == 12) right_o = db;
	if(type == 2) target = db;
	if(type == 21) left_t = db;
	if(type == 22) right_t = db;
	if(type == 3) uniq = db;
}
leveldb::DB *relate; //refer to ground-truth db, using for simulate attack
void init_relate(const char *rel)
{
      leveldb::Options options;
      options.create_if_missing = true;
      leveldb::Status status = leveldb::DB::Open(options, rel, &relate);
      assert(status.ok());
        assert(relate != NULL);
}
// enqueue leaked chunks based on leakage rate
void stat_db()
{
	leveldb::Iterator* it = target->NewIterator(leveldb::ReadOptions());
	leveldb::Status status;
	std::string existing_value;

	for (it->SeekToFirst(); it->Valid(); it->Next())
	{
		total ++;
		status = origin->Get(leveldb::ReadOptions(), it->key(), &existing_value);
		if(LEAK_RATE != 0 && rand()%10000 <= LEAK_RATE*10000)
		{
			common ++;
			if (status.ok())
			{
				node entry_o;
				memcpy(entry_o.key, it->key().ToString().c_str(), FP_SIZE);
			//	entry_o.count = strtoimax(existing_value.data(), NULL, 10);
				entry_o.count = *(uint64_t *)existing_value.c_str();
				q_o.push(entry_o);

				node entry_t;
				memcpy(entry_t.key, it->key().ToString().c_str(), FP_SIZE);
			//	entry_t.count = strtoimax(it->value().ToString().c_str(), NULL, 10);
				entry_t.count = *(uint64_t *)it->value().ToString().c_str();
				leveldb::Status s;
				leveldb::Slice k(entry_t.key, FP_SIZE);
				char buf[32];
				memset(buf, 0, 32);
				sprintf(buf, "%lu", entry_t.count);
				leveldb::Slice u(buf, sizeof(uint64_t));
				s = uniq->Put(leveldb::WriteOptions(), k, u);
				q_t.push(entry_t);

				leak ++;		
			}else
			if(!status.IsNotFound())printf("%s\n", status.ToString().c_str());
		}
	}

	printf("Total number of unique ciphertext chunks: %lu\nLeakage rate: %lf%%\n", total,(double)(LEAK_RATE * 100.0));
}

void print_fp(node a)
{
	int len = 0;
	for (len = 0; len < FP_SIZE; len++)
	{
		printf("%02x", (unsigned char)a.key[len]);
		if (len < FP_SIZE - 1) printf(":");
	}
	printf("\t");
}

void left_insert(int type, char* fp, uint64_t k)
{
	leveldb::Status status;
	leveldb::Slice key(fp, FP_SIZE);
	std::string existing_value;
	priority_queue<node, vector<node>, cmp >* pt;

	if(type == 0) pt = &pq;
	else if (type == 1)pt = &pc;


	uint64_t len = 0;
	char tar[FP_SIZE];
	uint64_t tmp;

	if(type == 0) status = left_o->Get(leveldb::ReadOptions(), key, &existing_value);
	else 	status = left_t->Get(leveldb::ReadOptions(), key, &existing_value);

	if(status.ok())
	{
		while(len < existing_value.size())
		{
			memcpy(tar, existing_value.c_str()+len, FP_SIZE);

			const char* t_int = existing_value.c_str()+len+FP_SIZE;
			tmp = *(uint64_t*)t_int;
			node entry;
			memcpy(entry.key, tar, FP_SIZE);
			entry.count = tmp;

			if(pt->size()<k)
			{
				pt->push(entry);
			}else
			{
				node min = pt->top();
				if(tmp > min.count)
				{
					pt->pop();
					pt->push(entry);
				}
			}


			len += (FP_SIZE+sizeof(uint64_t));	
		}
	}else
	if(!status.IsNotFound())printf("%s\n", status.ToString().c_str());
}

void right_insert(int type, char* fp, uint64_t k)
{
	leveldb::Status status;
	leveldb::Slice key(fp, FP_SIZE);
	std::string existing_value;
	priority_queue<node, vector<node>, cmp >* pt;

	if(type == 0) pt = &pq;
	else pt = &pc;


	uint64_t len = 0;
	char tar[FP_SIZE];
	uint64_t tmp;

	if(type == 0)status = right_o->Get(leveldb::ReadOptions(), key, &existing_value);
	else status = right_t->Get(leveldb::ReadOptions(), key, &existing_value);

	if(status.ok())
	{
		while(len < existing_value.size())
		{
			memcpy(tar, existing_value.c_str()+len, FP_SIZE);
			const char* t_int = existing_value.c_str()+len+FP_SIZE;
			tmp = *(uint64_t*)t_int;
			node entry;
			memcpy(entry.key, tar, FP_SIZE);
			entry.count = tmp;

			if(pt->size()<k)
			{
				pt->push(entry);
			}else
			{
				node min = pt->top();
				if(tmp > min.count)
				{
					pt->pop();
					pt->push(entry);
				}
			}


			len += (FP_SIZE+sizeof(uint64_t));	
		}
	}else
	if(!status.IsNotFound())printf("%s\n", status.ToString().c_str());

}

// insert top-k frequent chunks (in db) into pq
void db_insert(leveldb::DB* db, uint64_t k)
{
	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for(it->SeekToFirst(); it->Valid(); it->Next())
	{
		//uint64_t val = strtoimax(it->value().ToString().c_str(), NULL, 10);
		uint64_t val = *(uint64_t *)it->value().ToString().c_str();
		node entry;
		memcpy(entry.key, it->key().ToString().c_str(), FP_SIZE);
		entry.count = val;
		if(pq.size()<k) 
		{
			pq.push(entry);
		}else
		{
			node min = pq.top();
			if(val > min.count)
			{
				pq.pop();
				pq.push(entry);
			}
		}
	}	

}
vector <node>ansq;
double entropy(vector <node>&tp){
	double ansEntropy = 0;
	uint64_t countSum = 0;
	double minEntropy = 9999999;
	vector <node>::iterator it;
	for(it = tp.begin(); it != tp.end(); it++){
		countSum += it->count;
	}
	if(countSum == 0)return 0;
	for(it = tp.begin(); it != tp.end(); it++){
		double po = (double)it->count / (double)countSum;
		if(po != 0){
			ansEntropy += (-po * (log(po) / log(2)));
			if((-po * (log(po) / log(2)) < minEntropy)) minEntropy = (-po * (log(po) / log(2)));
		}
	}
	if(MIN_ENTROPY)ansEntropy = minEntropy;
	
	return ansEntropy;
}
void getEntropy(stack <node>&inStack, double leftEntropys[], double rightEntropys[], int type, int num){
	double leftEntropy, rightEntropy;
	vector <node> cal;
	stack  <node> invent;
	priority_queue<node, vector<node>, cmp > * ui;
	if (type == 0){
		ui = &pq;
	}
	else{
		ui = &pc;
	}
	int ansPosision = 0;
	stack<node> tp(inStack);
	while(!tp.empty()){
		node ne = tp.top();
		tp.pop();
		cal.clear();
		while(!invent.empty()) invent.pop();
		while(!pq.empty()) pq.pop();
		while(!pc.empty()) pc.pop();
		left_insert(type, ne.key, num);
		while(!ui->empty()){
			invent.push(ui->top());
			ui->pop();
		}
		while(!invent.empty()){
			cal.push_back(invent.top());
			invent.pop();
		}
		leftEntropy = entropy(cal);
		//--------------------------------------------------
		cal.clear();
		while(!invent.empty()) invent.pop();
		while(!pq.empty()) pq.pop();
		while(!pc.empty()) pc.pop();
		right_insert(type, ne.key, num);
		while(!ui->empty()){
			invent.push(ui->top());
			ui->pop();
		}
		while(!invent.empty()){	
			cal.push_back(invent.top());
			invent.pop();
		}
		rightEntropy = entropy(cal);
		//----------------------------------------------------
		leftEntropys[ansPosision] = leftEntropy;
		rightEntropys[ansPosision] = rightEntropy;
		ansPosision++;
	}
}
double dist(double l1,double r1, double l2, double r2){
	return sqrt((l1 - l2) * (l1 - l2) + (r1 - r2) * (r1 - r2));
}
void peakToPair(double leftOrigin[], double rightOrigin[], double leftTarget[], double rightTarget[], stack<node> &originStack, stack<node> &targetStack, int flag){
	vector <node> originTemp, targetTemp;
	originTemp.clear();
	targetTemp.clear();
	while(!originStack.empty()){
		originTemp.push_back(originStack.top());
		originStack.pop();
	}
	while(!targetStack.empty()){
		targetTemp.push_back(targetStack.top());
		targetStack.pop();
	}
	int originNum = originTemp.size();
	int targetNum = targetTemp.size();
//	printf("%d   %d\n", originNum, targetNum);
	double minDist = MAX_INT;int minpos = -1;
	double nowDist;
	for(int i = 0; i < targetNum; i++){
		int r = 2;
		if(flag == 1) r = 0;
		minDist = MAX_INT, minpos = -1;
		for(int j = max(0, i - r); j < min(originNum, i + RADIUS); j++){
			nowDist = dist(leftTarget[i], rightTarget[i], leftOrigin[j], rightOrigin[j]);
			//printf("dist=%lf\n", nowDist);
			if(nowDist < ENTROPY_THESHOLD ){
				//this may be a pair
				if(minDist > nowDist){
					minDist = nowDist;
					minpos = j;
				}
			}
		}
		if(minpos != -1){
			leveldb::Status findStatus;
			leveldb::Slice inputKey(targetTemp[i].key, FP_SIZE);
			std::string getValue;
			findStatus = uniq->Get(leveldb::ReadOptions(), inputKey, &getValue);
			
	                //-------------------add test---------------------
                        //print_fp(originTemp[minpos]);print_fp(targetTemp[i]);printf("\n");
			//printf("the dist of this two is %lf:\n", minDist);
        	        //-------------------test end---------------------
				
			if(!findStatus.ok()){
				if(q_o.size() + 1 > QUEUE_LIMIT) break;
				leveldb::Slice valueKey(originTemp[minpos].key, FP_SIZE);
				findStatus = uniq->Put(leveldb::WriteOptions(), inputKey, valueKey);
				findStatus = uniq->Put(leveldb::WriteOptions(), valueKey, inputKey);
				leftOrigin[minpos] = MAX_INT;rightOrigin[minpos] = MAX_INT;
				q_o.push(originTemp[minpos]);
				q_t.push(targetTemp[i]);
			}
		}
	}
}
bool equal(char* ptext, char* ctext)
{
	if(!E_ATTACK){
		if(memcmp(ptext, ctext, FP_SIZE) == 0)return 1;
		else return 0;
	}else
	{
		leveldb::Status cst;
                leveldb::Slice key(ctext, FP_SIZE);
                std::string existing_value = "";
                cst = relate->Get(leveldb::ReadOptions(), key, &existing_value);
		if(memcmp(ptext, existing_value.c_str(), FP_SIZE) == 0)return 1;
		else return 0;
	}
}
void main_loop()
{
	ansq.clear();
	stack<node> tmp;
	stack<node> tmpA;
	stack<node> tmpB;
	stack<node> omp;
	double originLeftEntropy[MAX_UO];
	double targetLeftEntropy[MAX_UT];
	double originRightEntropy[MAX_UO];
	double targetRightEntropy[MAX_UT];
	if(LEAK_RATE == 0)
	{
		db_insert(origin, INIT);
		while (!pq.empty())//inverting chunk sequence (i.e., sort u-frequent chunks by frequency) 
		{
			tmpA.push(pq.top());
			pq.pop();
		}
		db_insert(target, INIT);
		while (!pq.empty())//inverting the sequence (i.e., sort u-frequent chunks by frequency) 
		{
			tmpB.push(pq.top());
			pq.pop();
		}
		getEntropy(tmpA, originLeftEntropy, originRightEntropy, 0, 5000);
		getEntropy(tmpB, targetLeftEntropy, targetRightEntropy, 1, 5000);
		peakToPair(originLeftEntropy, originRightEntropy, targetLeftEntropy, targetRightEntropy, tmpA, tmpB, 0);
		
		while(!tmpA.empty())tmpA.pop();while(!tmpB.empty())tmpB.pop();
	}
	// MAIN LOOP
	while(!q_o.empty() && !q_t.empty())
	{
	//Not E attck mode
		//if(memcmp(q_o.front().key, q_t.front().key, FP_SIZE) == 0) 
		
		if(equal(q_o.front().key, q_t.front().key))
		{
			ansq.push_back(q_o.front());
			if(q_t.front().count < 50)smallChunkAmount++;
			correct++;
		}else
		{
		//-----------add test here-----------------------
	//		printf("error pair:");
	//		print_fp(q_o.front());print_fp(q_t.front());
	//		printf("\n");
			if(q_o.front().count > 50)bigChunkError++;
			else smallChunkError++;
		//-----------test end ---------------------------
		}
		
		// clear
		while(!pq.empty()) pq.pop();
		while(!pc.empty()) pc.pop();
		while(!omp.empty()) omp.pop();
		while(!tmp.empty()) tmp.pop();

		left_insert(0, q_o.front().key, TH_K);
		left_insert(1, q_t.front().key, TH_K);

		// sort chunks by frequency
		while (!pq.empty() && !pc.empty())
		{
			omp.push(pq.top());
			tmp.push(pc.top());
			pq.pop();
			pc.pop();
		}
		while(!pq.empty())pq.pop();while(!pc.empty())pc.pop();
		getEntropy(omp, originLeftEntropy, originRightEntropy, 0, 5000);
       	        getEntropy(tmp, targetLeftEntropy, targetRightEntropy, 1, 5000);
                peakToPair(originLeftEntropy, originRightEntropy, targetLeftEntropy, targetRightEntropy, omp, tmp, 1);

		while(!pq.empty()) pq.pop();
		while(!pc.empty()) pc.pop();
		while(!omp.empty()) omp.pop();
		while(!tmp.empty()) tmp.pop();

		right_insert(0, q_o.front().key, TH_K);
		right_insert(1, q_t.front().key, TH_K);

		while (!pq.empty() && !pc.empty())
		{
			omp.push(pq.top());
			tmp.push(pc.top());
			pq.pop();
			pc.pop();
		}
		while(!pq.empty())pq.pop();while(!pc.empty())pc.pop();
		getEntropy(omp, originLeftEntropy, originRightEntropy, 0, 5000);
                getEntropy(tmp, targetLeftEntropy, targetRightEntropy, 1, 5000);
                peakToPair(originLeftEntropy, originRightEntropy, targetLeftEntropy, targetRightEntropy, omp, tmp, 1);
		
		q_o.pop();
		q_t.pop();
		involve ++;
	}
	printf("Leaked chunks:%lu\nLeaked chunks appearing in auxiliary information:%lu\n", common, leak);
	printf("Inferred chunks: %lu\nInference rate: %lf%%\n", correct + common -leak, (double)((double)(correct + common - leak)/total)*100.0);
}

int main (int argc, char *argv[])
{
	init_db(argv[5], 1);// refer to original F_db
	init_db(argv[6], 11);// refer to original L_db
	init_db(argv[7], 12);// refer to original R_db
	init_db(argv[8], 2);// refer to targrt F_db
	init_db(argv[9], 21);// refer to target L_db
	init_db(argv[10], 22);// refer to target R_db

	init_db("./inference-db/", 3);
	init_relate("./ground-truth/");
	INIT = atoi(argv[1]);	// u
	TH_K = atoi(argv[2]);	// v
	QUEUE_LIMIT = atoi(argv[3]);	// w

	LEAK_RATE = atof(argv[4]);

	stat_db();
	main_loop();
	printf("\nSuccessfully inferred following chunks:\n");
/*	while(!ansq.empty())
	printf("\nSuccessfully inferred following chunks:\n");
	while(!ansq.empty())
        {
                node tmp = ansq.back();
                printf("%.2hhx",tmp.key[0]);
                for (int i = 1;i < FP_SIZE; i++)
                        printf(":%.2hhx", tmp.key[i]);
                printf("\n");
                ansq.pop_back();
        }*/
	printf("involve:%lu\n", involve);
	printf("small chunk:%lu\n", smallChunkAmount);
	printf("big chunk error:%lu\nsmall error:%lu\n", bigChunkError, smallChunkError);
	return 0;
}
