/* c/c++ */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* clustering */
#include "../alglib/stdafx.h"
#include <math.h>
#include "../alglib/dataanalysis.h"

/* timing */
#include <sys/time.h>

/* leveldb */
#include "leveldb/db.h"

/* stl */
#include <vector>
#include <queue>
#include <map>
#include <string>

/* crypto */
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

/* uint64_t */
#include <inttypes.h>

/* fingerprint size */
#define FP_SIZE 6

/* fixed-size segmentation, 1MB by default */
#define SEG_SIZE ((1<<23)) 

/* number of segments */
uint64_t seg_count = 0;

/* distance matrix */
double *dist = NULL;

/* segment (slice) db */
leveldb::DB *seg_db;
leveldb::DB *slice_db;
leveldb::DB *chunksize;
/* chunk information */
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
#define K_MINHASH 1

/* clustering report */	
alglib::ahcreport rep;


void init_dbs(const char *seg_name, unsigned char *slice_name); 
void read_hashes(FILE *fp, std::map<uint64_t, std::map<std::string, uint64_t>> &count_map);
void process_seg(std::map<uint64_t, std::map<std::string, uint64_t>> &count_map, std::queue<node> sq);
alglib::ae_int_t port2slice(std::map<uint64_t, std::map<std::string, uint64_t>> count_map, double r); 
void port2cluster(std::map<uint64_t, std::map<std::string, uint64_t>> count_map);
void clustering(); 
void print_test();

alglib::ae_int_t port2slice(std::map<uint64_t, std::map<std::string, uint64_t>> count_map, double r) {
	alglib::integer_1d_array cidx;
    alglib::integer_1d_array cz;
	alglib::ae_int_t k;
	/* generate clustering report */
	alglib::clusterizerseparatedbydist(rep, r, k, cidx, cz);

	printf("Partition into %llu segments, which form %ld slices with a minimum inter-cluster distance of %.2f.\n", seg_count, k, r);
	/* test output for clustering report */
	/* printf("%s\n", cidx.tostring().c_str()); */

	uint64_t i = 0;
	/* storing segment in slice db */
	for (std::map<uint64_t, std::map<std::string, uint64_t>>::iterator seg = count_map.begin(); \
			seg != count_map.end() && i < cidx.length(); \
			seg++, i++) {
		leveldb::Slice key((char *)&cidx[i], sizeof(int));
		leveldb::Status cst;
		std::string existing_value;
		std::string seg_id;
		seg_id.resize(sizeof(uint64_t));
		seg_id.assign((char *)&seg->first, sizeof(uint64_t));
		cst = slice_db->Get(leveldb::ReadOptions(), key, &existing_value);
		if (cst.ok()) {
			/* existing slice */
			existing_value += seg_id;
		} else {
			existing_value = seg_id;
		}
		cst = slice_db->Put(leveldb::WriteOptions(), key, existing_value);
		if(!cst.ok()) {
			printf("IO_er\n");
		}
	}
	return k;
}

void port2cluster(std::map<uint64_t, std::map<std::string, uint64_t>> count_map) {
	/* allocate memory for distance matrix */
	dist = new double [seg_count * seg_count]; 	

	/* derive distance between any two segments */
	/* int i = 0; */
	/* int j; */
    for (std::map<uint64_t, std::map<std::string, uint64_t>>::iterator seg_i = count_map.begin(); \
			seg_i != count_map.end(); \
			seg_i++) { 
		std::map<std::string, uint64_t> dist_i = seg_i->second;

		for (std::map<uint64_t, std::map<std::string, uint64_t>>::iterator seg_j = count_map.find(seg_i->first); \
			seg_j != count_map.end(); \
			seg_j++) {

			/* derive distance of seg_i and seg_j */
			std::map<std::string, uint64_t> merge = std::map<std::string, uint64_t>(dist_i);
			for (std::map<std::string, uint64_t>::iterator mit = (seg_j->second).begin(); mit != (seg_j->second).end(); mit++) {
				merge[mit->first] = 1;
			}

			dist[seg_i->first * seg_count + seg_j->first] = 1.0 - (double)(dist_i.size() + seg_j->second.size() - merge.size())/(double)(merge.size());
			dist[seg_j->first * seg_count + seg_i->first] = dist[seg_i->first * seg_count + seg_j->first]; 
			merge.clear();
		}
	}

	/* test output for distance */
	/* for (int i = 0; i < seg_count * seg_count; i++ ) { */
	/* 		printf("%.2f\t", dist[i]); */
	/* } */
}

void clustering() {
	alglib::clusterizerstate s;
	alglib::real_2d_array d;

	alglib::integer_1d_array cidx;
	alglib::integer_1d_array cz;
	d.setcontent(seg_count, seg_count, dist);

	alglib::clusterizercreate(s);
	alglib::clusterizersetdistances(s, d, 1);
	alglib::clusterizerrunahc(s, rep);
}


void process_seg(std::map<uint64_t, std::map<std::string, uint64_t>> &count_map, std::queue<node> sq)
{
	/* record all chunk fingerprints in segment */
	std::string segmentValue = "";
	std::map<std::string, uint64_t> *p_map = new std::map<std::string, uint64_t>;
	while(!sq.empty())
	{
		node now = sq.front();
		std::string chunkHash(now.key, FP_SIZE);

		leveldb::Slice chunkKey(now.key, FP_SIZE);
                leveldb::Slice chunkSize((char*)(&now.size), sizeof(uint64_t));
                leveldb::Status cts = chunksize->Put(leveldb::WriteOptions(), chunkKey, chunkSize);

        	segmentValue += chunkHash;
		(*p_map)[chunkHash] += 1;
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

	count_map.insert(make_pair(seg_count, *p_map));
	seg_count++;
}

void read_hashes(FILE *fp, std::map<uint64_t, std::map<std::string, uint64_t>> &count_map){ 
	char read_buffer[256];
	char *item;

	/* current segment size */
	uint64_t sq_size = 0;

	/* chunk fingerprints of current segment */
	std::queue<node> sq;

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
			process_seg(count_map, sq);
			/* clean current segment */
			while(!sq.empty()) {
				sq.pop();
			}
			sq_size = 0;
		}

		node entry;
		memcpy(entry.key, hash, FP_SIZE);
		entry.size = size;

		sq_size += size;
		sq.push(entry);
	}
	process_seg(count_map, sq);
}

void init_dbs(const char *seg_name, const char *slice_name, const char *chunk_size_name) {
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

      status = leveldb::DB::Open(options, chunk_size_name, &chunksize);
      assert(status.ok());
      assert(chunksize != NULL);
}

void print_test() {
	uint64_t seg_id;
	leveldb::Iterator* slice_it = slice_db->NewIterator(leveldb::ReadOptions());

	for (slice_it->SeekToFirst(); slice_it->Valid(); slice_it->Next()) {
		/* print slice id */
		printf("Slice %d:\n", *(int *)(slice_it->key().ToString().c_str()));

		/* print segment id */
		for (int i = 0; i < slice_it->value().size() - sizeof(uint64_t) + 1; i += sizeof(uint64_t)) {
			printf("\t");
			printf("Segment %llu:\n", *(uint64_t *)(slice_it->value().ToString().c_str() + i));

			/* done print of one segment hash */
			memcpy(&seg_id, slice_it->value().ToString().c_str() + i, sizeof(uint64_t));

			/* print chunk hash */
			printf("\t\t");
			leveldb::Slice seg_search((char *)&seg_id, sizeof(uint64_t));
			std::string str;
			leveldb::Status status = seg_db->Get(leveldb::ReadOptions(), seg_search, &str);
			if (status.ok() == 0) {
				printf("segment not found, error\n");
			} else {
				for (int j = 0; j < str.size(); j++) {
					printf("%02x", (unsigned char)(str.c_str()[j]));
					if (j % FP_SIZE < FP_SIZE - 1) {
						printf(":");
					} else {
						printf("\n");
						if (j < str.size() - 1) {
							printf("\t\t");
						}
					}
				} 
			}
			/* not end */
			/* if (i < slice_it->value().size() - 1) { */
			/* 	printf("\t"); */
			/* } */
		}
	}
}

/* test correctness of clustering */
void test_cluster(alglib::ae_int_t k) {
	uint64_t seg_id;
	uint64_t seg;
	uint64_t max_count;
	double ratio_sum = 0;
	leveldb::Iterator* slice_it = slice_db->NewIterator(leveldb::ReadOptions());
	std::map<std::string, uint64_t> minhash_count;
	std::map<std::string, uint64_t> minhash_slice;
	std::priority_queue<node, std::vector<node>, cmp > pq;

	for (slice_it->SeekToFirst(); slice_it->Valid(); slice_it->Next()) {
		/* print slice id */
		printf("slice %d:\n", *(int *)(slice_it->key().ToString().c_str()));
		/* clear content for slice 
		 * minhash_count: map MinHash to number of segments that share the MinHash 
		 * seg: number of segments
		 * max_count: maximum number of segments that share the same MinHash
		 * */
		minhash_count.clear();
		seg = 0;
		max_count = 0;
		
		for (int i = 0; i < slice_it->value().size() - sizeof(uint64_t) + 1; i += sizeof(uint64_t)) {
			/* each segment */
			while(!pq.empty()) {
				pq.pop();
			}
			seg++;
			printf("\t");

			memcpy(&seg_id, slice_it->value().ToString().c_str() + i, sizeof(uint64_t));

			leveldb::Slice seg_search((char *)&seg_id, sizeof(uint64_t));
			std::string str;
			leveldb::Status status = seg_db->Get(leveldb::ReadOptions(), seg_search, &str);
			if (status.ok() == 0) {
				printf("segment not found, error\n");
			} else {
				/* derive MinHash */
				for (int j = 0; j < str.size() - FP_SIZE + 1; j += FP_SIZE) {
					node chunk_entry;
					memcpy(chunk_entry.key, str.c_str() + j, FP_SIZE);
					/* not used entry */
					chunk_entry.size = 0;

					if (pq.size() < K_MINHASH) {
						pq.push(chunk_entry);
					} else {
						node max = pq.top();
						if (memcmp(max.key, chunk_entry.key, FP_SIZE) > 0) {
							pq.pop();
							pq.push(chunk_entry);
						}
					}
				}
				for (int k = 0; k < rand()%K_MINHASH; k++) {
					pq.pop();
				}
				std::string min_hash = std::string(pq.top().key, FP_SIZE);

				printf("segment %llu, minimum fingerprint ", (uint64_t)seg_id);
				for (int ii = 0; ii < FP_SIZE; ii++) {
					printf("%02x", *(unsigned char *)(min_hash.c_str() + ii));
					if (ii < FP_SIZE - 1) {
						printf(":");
					}
				}
				printf("\n");

				/* increase minhash counter and maximum count of minhash*/
				minhash_count[min_hash]++;
				minhash_slice[min_hash]++;
				if (minhash_count[min_hash] > max_count) {
					max_count = minhash_count[min_hash]; 
				}
			}
		}

		/* output MinHash ratio */
		/* printf("max_count = %llu, seg_count = %llu\n", max_count, seg); */
		/* printf("%.2f\n", (double)(max_count/seg)); */
		ratio_sum += (double)(max_count/seg);
	}

	printf("\nOverall MinHash ratio: %.2f; support ratio: %.2f\n", ratio_sum / k, (double) minhash_slice.size() / k);
}


int main (int argc, char *argv[]) {
	/* argv[1] points to hash file; */ 
	/* argv[2] points to parameter of clustering */  
	assert(argc == 3);

	FILE *fp = fopen(argv[1], "r");
	assert(fp != NULL);
	init_dbs("./output/cipher/seg", "./output/cipher/slice", "./output/chunksize");
	double r = atof(argv[2]);

	timeval start, end;  

	/* map of chunk frequency count for each segment */
	std::map<uint64_t, std::map<std::string, uint64_t>> count_map;
	//index fp count
	gettimeofday(&start, NULL); 
	read_hashes(fp, count_map);
	gettimeofday(&end, NULL); 
	printf("count time = %ldms\n", 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000);

	/* test output for mapping */ 
	/* std::map<uint64_t, std::map<std::string, uint64_t>>::iterator mit; */
    /* for (mit = count_map.begin(); mit != count_map.end(); mit++) { */
	/* 	printf("seg %llu:\n", mit->first); */
	/* 	for (std::map<std::string, uint64_t>::iterator cit = mit->second.begin(); cit != mit->second.end(); cit++) { */
	/* 		printf("\t%llu\n", cit->second); */
	/* 	} */
	/* } */

	gettimeofday(&start, NULL); 
	port2cluster(count_map);
	gettimeofday(&end, NULL); 
	/* printf("port2cluster = %ldms\n", 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000); */

	gettimeofday(&start, NULL); 
	clustering();
	gettimeofday(&end, NULL); 
	/* printf("clustering time = %ldms\n", 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000); */

	gettimeofday(&start, NULL); 
	alglib::ae_int_t k = port2slice(count_map, r);
	gettimeofday(&end, NULL); 
	/* printf("port2slice time = %ldms\n", 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000); */
	fclose(fp);

	test_cluster(k);

	printf("\n\n");

	/* print_test(); */

	delete seg_db;
	delete slice_db;
	delete dist;
	return 0;
}



