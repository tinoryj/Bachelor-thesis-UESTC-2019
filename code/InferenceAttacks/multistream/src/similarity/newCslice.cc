#include <bits/stdc++.h>

/* clustering */
#include "../alglib/stdafx.h"
#include "../alglib/dataanalysis.h"

/* timing */
#include <sys/time.h>

/* leveldb */
#include "leveldb/db.h"

/* crypto */
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

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
struct node {

	char key[FP_SIZE];
	uint64_t size;
};

struct mapNode {

	uint64_t index;
	string content;
	uint64_t count;
};

struct pairNode {

	string content;
	uint64_t count;
};

struct cmp {

	bool operator()(node a, node b) {

		return memcmp(b.key, a.key, FP_SIZE); // b > a return 1
	}
};
#define K_MINHASH 1


vector<mapNode> mapList;

/* clustering report */	
alglib::ahcreport rep;


void init_dbs(const char *seg_name, unsigned char *slice_name); 
void read_hashes(FILE *fp, std::map<uint64_t, std::map<std::string, uint64_t>> &count_map);
void process_seg(std::map<uint64_t, std::map<std::string, uint64_t>> &count_map, std::queue<node> sq);
alglib::ae_int_t port2slice(std::map<uint64_t, std::map<std::string, uint64_t>> count_map, double r); 
void port2cluster(std::map<uint64_t, std::map<std::string, uint64_t>> count_map);
void clustering(); 

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

	// test output for distance 
	// for (int i = 0; i < seg_count * seg_count; i++ ) { 
	// 		printf("%.2f\t", dist[i]); 
	// } 
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


void process_seg(queue<node> sq) {

	/* record all chunk fingerprints in segment */
	string segmentValue = "";
	
	mapNode newMapNode;

	while(!sq.empty())	{

		node now = sq.front();
		string chunkHash(now.key, FP_SIZE);

		leveldb::Slice chunkKey(now.key, FP_SIZE);
        leveldb::Slice chunkSize((char*)(&now.size), sizeof(uint64_t));
        leveldb::Status cts = chunksize->Put(leveldb::WriteOptions(), chunkKey, chunkSize);
		segmentValue += chunkHash;
		newMap += 1;
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

void read_hashes(FILE *fp) {

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
		while (item != NULL && idx < FP_SIZE) 

			hash[idx++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}

		uint64_t size = atoi((const char*)item);

		if (sq_size + size > SEG_SIZE) {
			process_seg(sq);
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
	process_seg(sq);
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



