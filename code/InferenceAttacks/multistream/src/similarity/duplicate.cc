#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include "leveldb/db.h"

#define FP_SIZE 6

leveldb::DB *db;

uint64_t total = 0;
uint64_t duplicate = 0;


void init_db(char *db_name) {
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, db_name, &db);
	assert(status.ok());
	assert(db != NULL);
}

void read_hashes(FILE *fp) {
	char read_buffer[256];
	char *item;
	char last[FP_SIZE];
	memset(last, 0, FP_SIZE);

	while (fgets(read_buffer, 256, fp)) {
		// skip title line
		if (strpbrk(read_buffer, "Chunk")) {
			continue;
		}

		// a new chunk
		char hash[FP_SIZE];
		memset(hash, 0, FP_SIZE);
		total++;


		// store chunk hash and size
		item = strtok(read_buffer, ":\t\n ");
		int idx = 0;
		while (item != NULL && idx < FP_SIZE){
			hash[idx++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}

		leveldb::Status status;
		leveldb::Slice key(hash, FP_SIZE);

		// reference count
		uint64_t count;
		std::string existing_value;
		status = db->Get(leveldb::ReadOptions(), key, &existing_value);

		if (status.ok()) {
			//increment counter
			count = *(uint64_t *)existing_value.c_str();
			count++;
			status = db->Delete(leveldb::WriteOptions(), key);
		} else 
			count = 1;	// set 1
		std::string count_buf = "";
		count_buf.resize(sizeof(uint64_t));
		count_buf.assign((char *)&count, sizeof(uint64_t));
		leveldb::Slice update(count_buf.c_str(), sizeof(uint64_t));
		status = db->Put(leveldb::WriteOptions(), key, update);

		if (status.ok() == 0) 
			fprintf(stderr, "error msg=%s\n", status.ToString().c_str());

		}

}

void remove_unique(FILE *fp) {
	char read_buffer[256];
	char *item;
	char last[FP_SIZE];
	memset(last, 0, FP_SIZE);

	rewind(fp);
	while (fgets(read_buffer, 256, fp)) {
		// skip title line
		if (strpbrk(read_buffer, "Chunk")) {
			continue;
		}

		// a new chunk
		char hash[FP_SIZE];
		memset(hash, 0, FP_SIZE);

		// store chunk hash
		item = strtok(read_buffer, ":\t\n ");
		int idx = 0;
		while (item != NULL && idx < FP_SIZE){
			hash[idx++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}

		uint64_t size = atoi((const char*)item);


		leveldb::Status status;
		leveldb::Slice key(hash, FP_SIZE);

		// reference count
		uint64_t count = 0;
		std::string existing_value;
		status = db->Get(leveldb::ReadOptions(), key, &existing_value);

		if (status.ok() != 0) {
			//increment counter
			count = *(uint64_t *)existing_value.c_str();
		} else {
			printf("non-count chunk \n");
			exit(1);
		}

		if (count > 1) {
			/* output duplicate chunk */
			duplicate++;
			for (int i = 0; i<6; i++) {
				printf("%02x", (unsigned char)hash[i]); 
				if (i != 5) {
					printf(":");
				}
			}
			printf("\t%llu\t10\n", size);
		}
	}
}

int main (int argc, char *argv[]){
	// argv[1] points to hash file; argv[2] points to analysis db  

	assert(argv[2] != NULL);
	init_db(argv[2]);

	FILE *fp = NULL;
	fp = fopen(argv[1], "r");
	assert(fp != NULL);
	read_hashes(fp);

	remove_unique(fp);
	printf("[REMOVE UNIQUE] number of duplicate chunks: %llu/%llu\n", duplicate, total);
	fclose(fp);
	delete db;
	db = NULL;
	return 0;
}
