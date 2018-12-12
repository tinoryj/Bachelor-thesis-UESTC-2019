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
#include <map>

/* timing */
#include <time.h>

/* leveldb */
#include "leveldb/db.h"

/* min & abs */
#include <algorithm>    
#include <cmath>


#define FP_SIZE 6
#define MAXIMUM_ENTROPY 9999999
#define MIN_ENTROPY 0
#define USE_CHUNK_SIZE 1
#define E_ATTACK 0
/* structure for slice */
bool equal(const char* ptext, const char* ctext);
struct sliceNode{
    friend bool operator < (sliceNode n1, sliceNode n2){
		return n1.size < n2.size || ((n1.size == n2.size) && (n1.key < n2.key)) ;
	}
	std::string key;
    int size;
	std::string value;
    double sliceEntropy;
	std::map<std::string, uint64_t> sliceChunkCount;
	uint64_t sum_size;
};

struct chunkNode {
	friend bool operator < (chunkNode n1, chunkNode n2){
		return n1.count< n2.count;
	}
    char key[FP_SIZE];
    uint64_t count;
};

struct segmentNode{
	friend bool operator < (segmentNode n1, segmentNode n2){
		return n1.size < n2.size;
	}
	friend bool operator == (segmentNode n1, segmentNode n2){
		std::map<std::string, uint64_t>::iterator it1 = n1.segmentChunkCount.begin();
		std::map<std::string, uint64_t>::iterator it2 = n2.segmentChunkCount.begin();
		for(; it1 != n1.segmentChunkCount.end() && it2 != n2.segmentChunkCount.end(); it1++, it2++){
			if(!equal(it2->first.c_str(), it1->first.c_str()))return false;
		}
		return true;
	}
	uint64_t sum_size;
	std::string ID;
	int size;
	double segmentEntropy;
	std::map<std::string, uint64_t> segmentChunkCount;
};

uint64_t infer = 0;
uint64_t correct = 0;
uint64_t total = 0;


double test_overlap(std::vector<sliceNode>::iterator tar_it, std::vector<sliceNode>::iterator ori_it); 
void init_dbs(const char *p_seg_name, const char *p_slice_name, const char *c_seg_name, const char *c_slice_name); 
int count_chunk(std::string slice_value, std::map<std::string, uint64_t> &count_result, int flag);
void load_slice(std::priority_queue<sliceNode> &que, int flag);
double entropy(std::map<std::string, uint64_t>& tp, int flag);
void pair (std::priority_queue<sliceNode> p_slice, std::priority_queue<sliceNode> c_slice, uint64_t u, uint64_t r, double t); 



/* segment (slice) dbs for plaintext and ciphertext */
leveldb::DB *p_seg_db;
leveldb::DB *p_slice_db;
leveldb::DB *c_seg_db;
leveldb::DB *c_slice_db;

/* db for inferred ciphertext-plaintext pairs */
leveldb::DB *infer_db;
leveldb::DB *chunksize;
/* map ciphertext slice to plaintext slice */
std::map<sliceNode, sliceNode>slice_pairing;
std::map<sliceNode, sliceNode>segment_pairing;

void init_dbs(const char *p_seg_name, const char *p_slice_name, const char *c_seg_name, const char *c_slice_name, const char *refer_name, const char* chunksize_name) {
      leveldb::Options options;
      options.create_if_missing = true;
	  leveldb::Status status;

	  /* init segment db for plaintext */
      status = leveldb::DB::Open(options, p_seg_name, &p_seg_db);
      assert(status.ok());
      assert(p_seg_db != NULL);

	  /* init slice db for plaintext */
      status = leveldb::DB::Open(options, p_slice_name, &p_slice_db);
      assert(status.ok());
      assert(p_slice_db != NULL);

	  /* init segment db for ciphertext */
      status = leveldb::DB::Open(options, c_seg_name, &c_seg_db);
      assert(status.ok());
      assert(c_seg_db != NULL);

	  /* init slice db for ciphertext */
      status = leveldb::DB::Open(options, c_slice_name, &c_slice_db);
      assert(status.ok());
      assert(c_slice_db != NULL);
	//init referdb.
      status = leveldb::DB::Open(options, refer_name, &infer_db);
      assert(status.ok());
      assert(infer_db != NULL);
	//inits sizedb
      status = leveldb::DB::Open(options, chunksize_name, &chunksize);
      assert(status.ok());
      assert(chunksize != NULL);
}
leveldb::DB *testdb;
int countTest = 0;
void test_ini(const char *path)
{	
	leveldb::Options options;
      options.create_if_missing = true;
          leveldb::Status status;
	status = leveldb::DB::Open(options, path, &testdb);
      assert(status.ok());
      assert(infer_db != NULL);
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
//----------------calculate entropy------------
double entropy(std::map<std::string, uint64_t>& tp, int flag){ // 0 infer to sum entropy, 1 is min-entropy
	double ansEntropy = 0;
	uint64_t sum = 0;
	double minEntropy = MAXIMUM_ENTROPY;
	std::map<std::string, uint64_t>::iterator it;

	/* total count of chunk frequency */
	for (it = tp.begin(); it != tp.end(); it++){
		sum += it->second;
	}
	if (sum == 0) {
		return 0;
	}

	for (it = tp.begin(); it != tp.end(); it++){
		double po = (double)it->second / (double)sum;
		if(po != 0) {
			ansEntropy += (-po * (log(po) / log(2)));
			if((-po * (log(po) / log(2))) < minEntropy) minEntropy = (-po * (log(po) / log(2)));
		}
		//if(po < minEntropy) minEntropy = po;
	}
	if(flag)ansEntropy = minEntropy;
	
	return ansEntropy;
}
bool equal(const char* ptext, const char* ctext)
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
	return 0;
}
//count chunk for every slice
int count_chunk(std::string slice_value, std::map<std::string, uint64_t> &count_result, int flag, uint64_t& sum_size){
	leveldb::DB *seg_db;
	int chunks = 0;
	/* non-zero for plaintext segment and zero for ciphertext segment */
	if (flag) {
		seg_db = p_seg_db;
	} else {
		seg_db = c_seg_db;
	}

    count_result.clear();
	sum_size = 0;
	for (int i = 0; i < slice_value.size() - sizeof(uint64_t) + 1; i+= sizeof(uint64_t)) {
		/* traverse and retrieve each segment */
		leveldb::Status status;
		leveldb::Slice seg_key((char *)(slice_value.c_str() + i), sizeof(uint64_t));
		std::string existing_value = "";
		status = seg_db->Get(leveldb::ReadOptions(), seg_key, &existing_value);
		if (status.ok() == 0) {
			printf("segment not found, error\n");
		}
		for (int j = 0; j < existing_value.size(); j += FP_SIZE) {
			/* count frequency of each chunk */
			std::string hash(existing_value.c_str() + j, FP_SIZE);

			leveldb::Slice chunkkey(hash.c_str(), FP_SIZE);
			std::string exv = "";
			status = chunksize->Get(leveldb::ReadOptions(), chunkkey, &exv);
			assert(status.ok());
			uint64_t uio = *((uint64_t *)exv.c_str());
			sum_size += uio; 
			count_result[hash] += 1;
			chunks++;
		}
	}
	/* count total number of ciphertext chunks (incorrect) */
	/* if (flag == 0) { */
	/* 	total += count_result.size(); */
	/* } */
	if(!USE_CHUNK_SIZE)sum_size= count_result.size();
	return chunks;
}


 /* load & sort slice by size */ 
void load_slice(std::priority_queue<sliceNode> &que, int flag){
	leveldb::DB *slice_db;
	/* non-zero for plaintext slice and zero for ciphertext slice */
	if (flag) {
		slice_db = p_slice_db;
	} else {
		slice_db = c_slice_db;
	}
	leveldb::Iterator* it = slice_db->NewIterator(leveldb::ReadOptions());
	leveldb::Status status;
	for (it->SeekToFirst(); it->Valid(); it->Next()){
		sliceNode slice;
		slice.key = it->key().ToString();
		slice.value = it->value().ToString();


		slice.sliceChunkCount.clear();

		/* number of included segments */
		slice.size = count_chunk(slice.value, slice.sliceChunkCount, flag, slice.sum_size);
		slice.sliceEntropy = entropy(slice.sliceChunkCount, MIN_ENTROPY);
		//if(!USE_CHUNK_SIZE)slice.sum_size = slice.sliceChunkCount.size();
		que.push(slice);

		/* test output of slice info */
		/* if (flag) { */
		/* 	printf("Slice ID: "); */
		/* 	for (int j = 0; j < slice.key.size(); j++) { */
		/* 		printf("%02x", (unsigned char)(slice.key.c_str()[j])); */
		/* 		if (j % FP_SIZE < FP_SIZE - 1) { */
		/* 			printf(":"); */
		/* 		} else { */
		/* 			printf("\n"); */
		/* 		} */ 
		/* 	} */
		/* } else { */
		/* 	printf("Slice ID: %llu \n", *(uint64_t *)slice.key.c_str()); */
		/* } */
		/* printf("Slice value: "); */
		/* for (int i = 0; i < slice.value.size() - sizeof(uint64_t) + 1; i += sizeof(uint64_t)) { */
		/* 	printf("%llu\t", *(uint64_t *)(slice.value.c_str() + i)); */
		/* } */
		/* printf("\nSlice size: %d\n", slice.size); */
		/* printf("Slice entropy: %.2f\n", slice.sliceEntropy); */
	}
}

/* u: number of slices to be paired */
/* r: search radius of slices */
/* t: upper boundary of entropy difference */
void pair (std::priority_queue<sliceNode> p_slice, std::priority_queue<sliceNode> c_slice, uint64_t u, uint64_t r, double t) {
	/* sorted slices for plaintext and ciphertext */ 
	std::vector<sliceNode> ori_vec;
	std::vector<sliceNode> tar_vec;

	double overall_minhash = 0;
	uint64_t overall_chunks = 0;
	uint64_t overall_pairs = 0;

	/* enqueue in order */
	while (!p_slice.empty()) {
		ori_vec.push_back(p_slice.top());
		p_slice.pop();
	}
	while (!c_slice.empty()) {
		tar_vec.push_back(c_slice.top());
		c_slice.pop();
	}

	uint64_t min_size = std::min(ori_vec.size(), tar_vec.size());
	uint64_t pair_counter = 0;
	std::vector<sliceNode>::iterator ori_it, tar_it, min_it;
	/* to pair top-frequent slices */
	for (tar_it = tar_vec.begin(); \
			tar_it != tar_vec.end() && pair_counter < min_size && pair_counter < u; \
			tar_it++, pair_counter++) {
		/* print ciphertext slice that is to be paired */
		printf("[TO PAIR] key %llu, size %d, entropy %.2f\n", *(uint64_t *)tar_it->key.c_str(), tar_it->size, tar_it->sliceEntropy);

		/* move ori_it into corresponding index positions */
		ori_it = ori_vec.begin(); 
		for (int i = 0; i < (int)(pair_counter - r) && ori_it != ori_vec.end(); i++) {
			ori_it++;
		}

		/* define pair range */ 
		uint64_t range;   
		if (pair_counter < r) {
			range = pair_counter + r;
		} else {
			range = 2 * r;
		}

		double min_diff = MAXIMUM_ENTROPY;
		for (uint64_t j = 0; j < range && ori_it != ori_vec.end(); j++, ori_it++) {
			//double diff = abs(ori_it->sliceEntropy - tar_it->sliceEntropy);
			//abs() may Integer the diff 
			double diff = (ori_it->sliceEntropy - tar_it->sliceEntropy);
			if(diff < 0) diff = -diff; 
			/* print candidate plaintext slice */
			printf("[CANDIDATE] key ");
			for (int ii = 0; ii < FP_SIZE; ii++) {
				printf("%02x", (unsigned char)ori_it->key.c_str()[ii]);
			}
			printf(", size %d, entropy %.2f, overlap %.2f\n", ori_it->size, ori_it->sliceEntropy, test_overlap(tar_it, ori_it));


			/* find plaintext slice that leads to minimum difference of entropy */
			if (diff < t && min_diff > diff) {
				min_diff = diff;
				min_it = ori_it;
			} 
		}
		if (min_diff < t) {
			/* add mapping into slice_pairing */
			std::cout<<tar_it->sum_size<<"    "<<min_it->sum_size<<std::endl;
                	if (min_diff < 1e-6 && min_it->size == tar_it->size && tar_it->sum_size == min_it->sum_size){
        	                //these two slice are totally the same;
				std::cout<<"samesizechunk\n";
				if(segment_pairing.insert(std::make_pair(*tar_it, *min_it)).second == false){
					printf("Insertion failed\n");
				}
         	       }else
			if (slice_pairing.insert(std::make_pair(*tar_it, *min_it)).second == false) {
				printf("Insertion failed\n");
			}

			/* print chosen plaintext slice */
			printf("[PAIR] key ");
			for (int ii = 0; ii < FP_SIZE; ii++) {
				printf("%02x", (unsigned char)min_it->key.c_str()[ii]);
			}
			overall_minhash += test_overlap(tar_it, min_it);
			overall_chunks += tar_it->sliceChunkCount.size();
			overall_pairs++;
			printf(", num of unique chunks %lu, entropy %.2f, overlap %.2f\n", min_it->sliceChunkCount.size(), min_it->sliceEntropy, test_overlap(tar_it, min_it));
			printf("\n");

			min_it->sliceEntropy = MAXIMUM_ENTROPY;
		} else {
			printf("[NO PAIR] none of slice under minimum entropy threshold can be paired\n\n");
		}
	}
	printf("[OVERALL] infer %llu/%llu pairs of slices that cover %llu chunks; minhash ratio %.2f\n", overall_pairs, u, overall_chunks, overall_minhash / overall_pairs);
}

double test_overlap(std::vector<sliceNode>::iterator tar_it, std::vector<sliceNode>::iterator ori_it) {
	int overlap = 0;

	for (std::map<std::string, uint64_t>::const_iterator chunk_it = tar_it->sliceChunkCount.begin(); chunk_it != tar_it->sliceChunkCount.end(); chunk_it++) {
		if (ori_it->sliceChunkCount.find(chunk_it->first) != ori_it->sliceChunkCount.end()) {
			overlap++;
		}
	}
	return (double)(overlap)/(tar_it->sliceChunkCount.size() + ori_it->sliceChunkCount.size() - overlap); 
}
int segmentInferChunk = 0, segmentcount = 0, infersegment = 0;
void segment_pair(double minP) {
	//-------------------segmentPair-----------------------
	printf("start!------\n");
	std::priority_queue<segmentNode> p_segment, c_segment;
	for(std::map<sliceNode, sliceNode>::iterator it = segment_pairing.begin(); it != segment_pairing.end(); it++){
		//clear queue;
		while(!p_segment.empty())p_segment.pop();
		while(!c_segment.empty())c_segment.pop();
		//load segment to p_segment & c_segment;i
		//printf("count c&p\n");
		for (int i = 0; i < it->first.value.size() - sizeof(uint64_t) + 1; i+= sizeof(uint64_t)){
			std::string tmps((it->first.value.c_str()+i), sizeof(uint64_t));
			segmentNode tmpnode;
			tmpnode.ID = tmps;
			tmpnode.size = count_chunk(tmps, tmpnode.segmentChunkCount, 0, tmpnode.sum_size);	
			tmpnode.segmentEntropy = entropy(tmpnode.segmentChunkCount, MIN_ENTROPY);
			c_segment.push(tmpnode);
		}
                for (int i = 0; i < it->second.value.size() - sizeof(uint64_t) + 1; i+= sizeof(uint64_t)){
                        std::string tmps((it->second.value.c_str() + i), sizeof(uint64_t));
                        segmentNode tmpnode;
                        tmpnode.ID = tmps;
                        tmpnode.size = count_chunk(tmps, tmpnode.segmentChunkCount, 1, tmpnode.sum_size);
			tmpnode.segmentEntropy = entropy(tmpnode.segmentChunkCount, MIN_ENTROPY);
                        p_segment.push(tmpnode);
                }//printf("count end\n");
		//pair segment
		std::vector<segmentNode> cv,pv;
		cv.clear();pv.clear();
		while(!p_segment.empty()){pv.push_back(p_segment.top()); p_segment.pop();}
		while(!c_segment.empty()){cv.push_back(c_segment.top()); c_segment.pop();}
		int p_size = pv.size();int c_size = cv.size();
		double minDis = MAXIMUM_ENTROPY;int minpos = -1;
		for(int i = 0; i < c_size; i++){
			minDis = MAXIMUM_ENTROPY;
			minpos = -1;
			for(int j = 0; j < p_size; j++){
				if(cv[i].sum_size != pv[j].sum_size)continue;
				double tmpdis = cv[i].segmentEntropy - pv[j].segmentEntropy;
				if(tmpdis < 0)tmpdis = -tmpdis;
				if(tmpdis < minDis){minDis = tmpdis; minpos = j;}
			}
			if(minDis < minP){
			//	std::cout<<"minDis:"<<minDis<<" "<<"minPos:"<<minpos<<std::endl;	
				segmentcount ++;
				pv[minpos].segmentEntropy = MAXIMUM_ENTROPY;
				if(cv[i] == pv[minpos]){
					infersegment ++;
					std::map<std::string, uint64_t>::iterator it;
					for( it = cv[i].segmentChunkCount.begin(); it!=cv[i].segmentChunkCount.end(); it++){
						leveldb::Slice cipher(it->first.c_str(), FP_SIZE);
                         			std::string existing_value;
                         			leveldb::Status status = infer_db->Get(leveldb::ReadOptions(), cipher, &existing_value);
						if(!status.ok()){
							segmentInferChunk++;
							status = infer_db->Put(leveldb::WriteOptions(), cipher, cipher);
						}
					}
				}
			}
		}
		/*while(!p_segment.empty() && !c_segment.empty()){
			if(p_segment.top() == c_segment.top()){
				segmentInferChunk += p_segment.top().segmentChunkCount.size();
				infersegment ++;
			}else
			{
				printf("p:%d, c:%d\n",p_segment.top().size,c_segment.top().size);
			}
			segmentcount ++;
			p_segment.pop();
			c_segment.pop();
		}*///printf("one end \n");
	}
	
}
void freq_analysis(uint64_t v) {
	std::priority_queue<chunkNode> p_chunk, c_chunk;
	std::map<std::string, double> c_chunk_entropy, p_chunk_entropy;

	for (std::map<sliceNode, sliceNode>::iterator it = slice_pairing.begin(); it != slice_pairing.end(); it++) {
		/* clear chunk queue */
		while (!p_chunk.empty()) {
			p_chunk.pop();
		}
		while (!c_chunk.empty()) {
			c_chunk.pop();
		}

		/* enqueue ciphertext chunks in each paired slice */
		for (std::map<std::string, uint64_t>::const_iterator chunk_it = it->first.sliceChunkCount.begin(); \
				chunk_it != it->first.sliceChunkCount.end(); \
				chunk_it++){
			chunkNode tmp;
			/* tmp.key = std::string(chunk_it->first.key.c_str()); */
			memcpy(tmp.key, chunk_it->first.c_str(), FP_SIZE);
			tmp.count = chunk_it->second;
			c_chunk.push(tmp);
		}

		/* enqueue plaintext chunks in each paired slice */
		for (std::map<std::string, uint64_t>::iterator chunk_it = it->second.sliceChunkCount.begin(); \
				chunk_it != it->second.sliceChunkCount.end(); \
				chunk_it++){
			chunkNode tmp;
			memcpy(tmp.key, chunk_it->first.c_str(), FP_SIZE);
			/* tmp.key = std::string(chunk_it->first.c_str()); */
			tmp.count = chunk_it->second;
			p_chunk.push(tmp);
		}

		uint64_t chunk_counter = 0;
		/* while(!c_chunk.empty() && !p_chunk.empty() && chunk_counter < v){ */
		while(!c_chunk.empty() && !p_chunk.empty()){
			//printf("%lld\n", c_chunk.top().count);
			 leveldb::Slice cipher(c_chunk.top().key, FP_SIZE);
			 std::string existing_value;
			 leveldb::Status status = infer_db->Get(leveldb::ReadOptions(), cipher, &existing_value);
			 if (!status.ok()) {
				 infer++;
				 leveldb::Slice plain(p_chunk.top().key, FP_SIZE);
                                if(USE_CHUNK_SIZE)
                                {
                                        std::string exvp = "";
                                        status = chunksize->Get(leveldb::ReadOptions(), plain, &exvp);
                                        assert(status.ok());
                                        uint64_t uiop = *((uint64_t *)exvp.c_str());

                                        std::string exvc = "";
                                        status = chunksize->Get(leveldb::ReadOptions(), cipher, &exvc);
                                        assert(status.ok());
                                        uint64_t uioc = *((uint64_t *)exvc.c_str());

					if(uioc != uiop)continue;//chunk size is not equal
                                }
				 status = infer_db->Put(leveldb::WriteOptions(), cipher, plain);
				 if (!status.ok()) {
					 printf("IO error\n");
				 }
				 if (equal(p_chunk.top().key, c_chunk.top().key)) {
					
					/*for (int ii = 0; ii < FP_SIZE; ii++) {
                        		        printf("%02x:", (unsigned char)p_chunk.top().key[ii]);
                        		}printf("\n");*/
					/*uint64_t db_count = 0;
					leveldb::Slice kEy(p_chunk.top().key,FP_SIZE);
					std::string vAlue;
					status = testdb->Get(leveldb::ReadOptions(), kEy, &vAlue);
					const char* sst = vAlue.c_str();
					db_count = *(uint64_t *)sst;
					if(db_count < 2)countTest ++;*/
					correct++;
				 }
			 }
			 c_chunk.pop();
			 p_chunk.pop();
			 chunk_counter++;
		 }
	}
}

int main(int argc, char* argv[]){

	std::priority_queue<sliceNode> p_slice;
	std::priority_queue<sliceNode> c_slice;

	init_dbs("output/plain/seg", \
			"output/plain/slice", \
			"output/cipher/seg", \
			"output/cipher/slice", "output/referdb", "output/chunksize");

	/* iniDb("./inference-db/", resultDb); */

	load_slice(p_slice, 1); 
	load_slice(c_slice, 0);
	pair(p_slice, c_slice, 4000, 536, 0.2);

	/* test output of slice_pairing */
	/* for (std::map<sliceNode, sliceNode>::iterator it = slice_pairing.begin(); \ */
	/* 		it != slice_pairing.end(); \ */
	/* 		it++) { */
	/* 	printf("Map ciphertext slice with id of %llu to plaintext slice identified by ", *(uint64_t *)it->first.key.c_str()); */
	/* 	for (int i = 0; i < FP_SIZE; i++) { */
	/* 		printf("%02x:", (unsigned char)it->second.key.c_str()[i]); */
	/* 	} */
	/* 	printf("\n"); */
	/* } */

	/* test correctness of slice pairing */
	/* uint64_t good_pair = 0; */
	/* uint64_t total_pair = 0; */
	/* for (std::map<sliceNode, sliceNode>::iterator it = slice_pairing.begin(); \ */
	/* 		it != slice_pairing.end(); \ */
	/* 		it++) { */
	/* 	printf("map ciphertext slice with id of %llu to plaintext slice identified by ", *(uint64_t *)it->first.key.c_str()); */
	/* 	for (int i = 0; i < FP_SIZE; i++) { */
	/* 		printf("%02x", (unsigned char)it->second.key.c_str()[i]); */
	/* 		if (i < FP_SIZE - 1) { */
	/* 			printf(":"); */
	/* 		} else { */
	/* 			printf("\n"); */
	/* 		} */
	/* 	} */
	/* 	uint64_t overlap = 0; */
	/* 	for (std::map<std::string, uint64_t>::const_iterator chunk_it = it->first.sliceChunkCount.begin(); chunk_it != it->first.sliceChunkCount.end(); chunk_it++) { */
	/* 		if (it->second.sliceChunkCount.find(chunk_it->first) != it->second.sliceChunkCount.end()) { */
	/* 			overlap++; */
	/* 		} */
	/* 	} */
	/* 	printf("overlap ratio: %.2f\n", (double)(overlap)/(it->first.sliceChunkCount.size() + it->second.sliceChunkCount.size() - overlap)); */
	/* 	total_pair++; */
	/* 	if ((double)(overlap)/(it->first.sliceChunkCount.size() + it->second.sliceChunkCount.size() - overlap) > 0.5) { */
	/* 		good_pair++; */
	/* 	} */
	/* } */
	/* printf("have %llu of good pairs (that have overlap ratio greater than 0.5) out of %llu total pairs\n", good_pair, total_pair); */


	/* leveldb::Options options; */
	/* options.create_if_missing = true; */
	/* leveldb::Status status = leveldb::DB::Open(options, "output/infer", &infer_db); */
	/* assert(status.ok()); */
	/* assert(infer_db != NULL); */
	//test_ini("./dbs/");
	
	//IMPROVE: infer db is not initiated, this may lead to segmentation fault!
	freq_analysis(1000);
	segment_pair(0.1);	
	printf("\n");
	printf("infer segment:%d. correctly segment:%d. correctly segments` unique chunks:%d.\n", segmentcount, infersegment, segmentInferChunk);
	printf("correctly infer %llu out of %llu inferred chunks:\n", correct, infer);
	//printf("unique: %d\n", countTest);
	/* test output of inferred chunks */
	/* leveldb::Iterator* level_it = infer_db->NewIterator(leveldb::ReadOptions()); */
	/* for (level_it->SeekToFirst(); level_it->Valid(); level_it->Next()){ */
	/* 	printf("infer original plaintext of "); */
	/* 	for (int i = 0; i < FP_SIZE; i++) { */
	/* 		printf("%02x", (unsigned char)level_it->key().ToString().c_str()[i]); */
	/* 		if (i < FP_SIZE - 1) { */
	/* 			printf(":"); */
	/* 		} else { */
	/* 			printf(" as "); */
	/* 		} */
	/* 	} */
	/* 	for (int j = 0; j < FP_SIZE; j++) { */
	/* 		printf("%02x", (unsigned char)level_it->value().ToString().c_str()[j]); */
	/* 		if (j < FP_SIZE - 1) { */
	/* 			printf(":"); */
	/* 		} else { */
	/* 			printf("\n"); */
	/* 		} */
	/* 	} */
	/* } */
    return 0;
}
