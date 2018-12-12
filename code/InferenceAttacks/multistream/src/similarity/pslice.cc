/* c/c++ */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>

/* stl */
#include <queue>
#include <string.h>
#include <vector>

/* timing */
#include <time.h>

/* leveldb */
#include "leveldb/db.h"

/* MinHash */
#define K_MINHASH 1

/* fingerprint size */
#define FP_SIZE 6

/* fixed-size segmentation, 1MB by default */
#define SEG_SIZE ((1<<23)) 

struct node
{
	char key[FP_SIZE];
	uint64_t size;
};

struct cmp
{
	bool operator()(node a, node b)
	{
		return memcmp(b.key, a.key, FP_SIZE); // b > a return 1
	}
};

leveldb::DB *seg_db;
leveldb::DB *slice_db;
leveldb::DB *chunk_size;
/* number of segments */
uint64_t seg_count = 0;
/* number of slices */
uint64_t slice_count = 0;

/* minimum fingerprint in segment */
/* priority_queue<node, vector<node>, cmp > pq; */


void init_dbs(const char *seg_name, unsigned char *slice_name); 
void process_seg(std::priority_queue<node, std::vector<node>, cmp > pq, std::queue<node> sq);
void read_hashes(FILE *fp);
void print_test();

void init_dbs(const char *seg_name, const char *slice_name, const char* chunk_size_name) {
      leveldb::Options options;
      options.create_if_missing = true;
	  leveldb::Status status;

	  /* init segment db */
      status = leveldb::DB::Open(options, seg_name, &seg_db);
      assert(status.ok());
      assert(seg_db != NULL);

	  /* init slice db */
      status = leveldb::DB::Open(options, slice_name, &slice_db);
      assert(status.ok());
      assert(slice_db != NULL);
	
      status = leveldb::DB::Open(options, chunk_size_name, &chunk_size);
      assert(status.ok());
      assert(chunk_size != NULL);
}





void process_seg(std::priority_queue<node, std::vector<node>, cmp > pq, std::queue<node> sq)
{
	/* k-minHash, by default K_MINHASH = 1 */
	int k = rand()%K_MINHASH;
	for (int i = 0; i < k; i++)
	{
		pq.pop();
	}
	node core = pq.top();

	/* record all chunk fingerprints in segment */
	std::string segmentValue = "";
	//chamge: add write chunk size here.
	while(!sq.empty())
	{
		node now = sq.front();
		std::string chunkHash(now.key, FP_SIZE);
		
		leveldb::Slice chunkKey(now.key, FP_SIZE);
		leveldb::Slice chunkSize((char*)(&now.size), sizeof(uint64_t));
		leveldb::Status cts = chunk_size->Put(leveldb::WriteOptions(), chunkKey, chunkSize);

        	segmentValue += chunkHash;
		sq.pop();
	   
	}

    //--------------Write segment db-----------------
	// key = segment id; value = concatenation of chunk fingerprints in segment 
    leveldb::Status cst;
    leveldb::Slice segkey((char *)&seg_count, sizeof(uint64_t));
    leveldb::Slice segvalue(segmentValue);
    cst = seg_db->Put(leveldb::WriteOptions(), segkey, segvalue);
    if(!cst.ok()) {
		printf("IO_er\n");
	}

    //--------------Write slice db-------------------------
	// key = MinHash; value = concatenation of segment fingerprints in MinHash class
	std::string seg_id;
	seg_id.resize(sizeof(uint64_t));
	seg_id.assign((char *)&seg_count, sizeof(uint64_t));
    leveldb::Slice sliceKey(core.key, FP_SIZE);
	std::string sliceValue = "";
    cst = slice_db->Get(leveldb::ReadOptions(), sliceKey, &sliceValue);
    if(cst.ok()) {
        sliceValue += seg_id;
    }else {
        sliceValue = seg_id;
		slice_count++;
    }
    cst = slice_db->Put(leveldb::WriteOptions(), sliceKey, sliceValue);
    if(!cst.ok()) {
		printf("IO_er\n");
	}
	seg_count++;
}

void read_hashes(FILE *fp) 
{
	char read_buffer[256];
	char *item;

	/* queue of chunks in current segment */
	std::queue<node> sq;
	/* priority queue of chunks (with minimum fingerprints) in current segment */
	std::priority_queue<node, std::vector<node>, cmp > pq;
	/* size of current segment */
	uint64_t sq_size = 0;

	while (fgets(read_buffer, 256, fp)) {
		// skip title line
		if (strpbrk(read_buffer, "Chunk")) {continue;}
		// a new chunk
		char hash[FP_SIZE];
		memset(hash, 0, FP_SIZE);
		// store chunk hash and size
		item = strtok(read_buffer, ":\t\n ");
		int idx = 0;
		while (item != NULL && idx < FP_SIZE) {
			hash[idx++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}

		uint64_t size = atoi((const char*)item);
		if (sq_size + size > SEG_SIZE) {
			process_seg(pq, sq);

			/* clear */
			while(!pq.empty()) {
				pq.pop();
			}
			while(!sq.empty()) {
				sq.pop();
			}
			sq_size = 0;
		}

		node entry;
		memcpy(entry.key, hash, FP_SIZE);
		entry.size = size;

		/* include current chunk into current segment */
		sq_size += size;
		sq.push(entry);
		if (pq.size() < K_MINHASH)
		{
			pq.push(entry);
		} else {
			node max = pq.top();
			if(memcmp(max.key, entry.key, FP_SIZE) > 0)
			{
				pq.pop();
				pq.push(entry);
			}
		}
	}

	process_seg(pq, sq);
}

void print_test() {
	uint64_t seg_id;
	leveldb::Iterator* slice_it = slice_db->NewIterator(leveldb::ReadOptions());

	for (slice_it->SeekToFirst(); slice_it->Valid(); slice_it->Next()) {
		/* print slice id */
		printf("Slice ");
		for (int i = 0; i < slice_it->key().ToString().size(); i++) {
			printf("%02x", (unsigned char)(slice_it->key().ToString().c_str()[i]));
			if (i % FP_SIZE < FP_SIZE - 1) {
				printf(":");
			} else {
					printf("\n");
				/* if (i < str.size() - 1) { */
				/* 	printf("\t"); */
				/* } */
			}
		} 

		/* print segment id */
		for (int j = 0; j < slice_it->value().size() - sizeof(uint64_t) + 1; j += sizeof(uint64_t)) {
			printf("\t");
			printf("Segment %llu:\n", *(uint64_t *)(slice_it->value().ToString().c_str() + j));

			/* done print of one segment id */
			memcpy(&seg_id, slice_it->value().ToString().c_str() + j, sizeof(uint64_t));
			/* print chunk hash */
			printf("\t\t");
			leveldb::Slice seg_search((char *)&seg_id, sizeof(uint64_t));
			std::string str;
			leveldb::Status status = seg_db->Get(leveldb::ReadOptions(), seg_search, &str);
			if (status.ok() == 0) {
				printf("segment not found, error\n");
			} else {
				for (int k = 0; k < str.size(); k++) {
					printf("%02x", (unsigned char)(str.c_str()[k]));
					if (k % FP_SIZE < FP_SIZE - 1) {
						printf(":");
					} else {
						printf("\n");
						if (k < str.size() - 1) {
							printf("\t\t");
						}
					}
				} 
			}
		}
	}
}

void test_minhash(){
	leveldb::Iterator* slice_it = slice_db->NewIterator(leveldb::ReadOptions());

	for (slice_it->SeekToFirst(); slice_it->Valid(); slice_it->Next()) {
		/* print slice id */
		printf("Slice ");
		for (int i = 0; i < slice_it->key().ToString().size(); i++) {
			printf("%02x", (unsigned char)(slice_it->key().ToString().c_str()[i]));
			if (i % FP_SIZE < FP_SIZE - 1) {
				printf(":");
			} else {
					printf("\n");
				/* if (i < str.size() - 1) { */
				/* 	printf("\t"); */
				/* } */
			}
		}
	}
}


int main (int argc, char *argv[])
{
	/* argv[1] points to hash file of plaintext workload */ 
	assert(argc == 2);

	FILE *fp = fopen(argv[1], "r");
	assert(fp != NULL);
	init_dbs("./output/plain/seg", "./output/plain/slice", "./output/chunksize");
	read_hashes(fp);
	printf("Partition plaintext chunks into %llu segments which form %llu slices\n\n", seg_count, slice_count);
	/* print_test(); */
	/* printf("\n\n"); */
	/* test_minhash(); */
	fclose(fp);
	return 0;
}
