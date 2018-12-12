#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <inttypes.h>
#include <vector>
#include "leveldb/db.h"

// #define ANALYSIS_DB "./db/"
#define FP_SIZE 6
using namespace std;
leveldb::DB *db;
leveldb::DB *store_left;
leveldb::DB *store_right;

void init_left(char *left)
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, left, &store_left);
	assert(status.ok());
	assert(store_left != NULL);
}

void init_right(char *right)
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, right, &store_right);
	assert(status.ok());
	assert(store_right != NULL);
}

void init_db(char *db_name) 
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, db_name, &db);
	assert(status.ok());
	assert(db != NULL);
}

void read_hashes(FILE *fp) 
{
	char read_buffer[256];
	char *item;
	int flag = 0;
	char last[FP_SIZE];
	memset(last, 0, FP_SIZE);
	while (fgets(read_buffer, 256, fp)) 
	{
		// skip title line
		if (strpbrk(read_buffer, "Chunk")) continue;
		//---------------------------start counting frequency db-----------------------------
		// a new chunk
		char hash[FP_SIZE];
		memset(hash, 0, FP_SIZE);

		// store chunk hash and size
		item = strtok(read_buffer, ":\t\n ");
		int idx = 0;
		while (item != NULL && idx < FP_SIZE)
		{
			hash[idx++] = strtol(item, NULL, 16);
			item = strtok(NULL, ":\t\n");
		}

		leveldb::Status status;
		leveldb::Slice key(hash, FP_SIZE);

		// reference count
		uint64_t count;
		std::string existing_value;
		status = db->Get(leveldb::ReadOptions(), key, &existing_value);

		if (status.ok()) 
		{
			//increment counter
		//	count = strtoimax(existing_value.data(), NULL, 10);
			count = *(uint64_t *)existing_value.c_str();
			count++;
			status = db->Delete(leveldb::WriteOptions(), key);
		}else 
		{
			if(status.IsNotFound())
				count = 1;
			else
				printf("There is %s\n", status.ToString().c_str());
		}//char count_buf[32];
		//memset(count_buf, 0, 32);
		//sprintf(count_buf, "%lu", count);
		string count_buf;
		count_buf.resize(sizeof(uint64_t));
		count_buf.assign((char *)&count, sizeof(uint64_t));
		leveldb::Slice update(count_buf.c_str(), sizeof(uint64_t));
		status = db->Put(leveldb::WriteOptions(), key, update);

		if (status.ok() == 0) 
			fprintf(stderr, "error msg=%s\n", status.ToString().c_str());

		//-------------------count frequecy db finished, start counting	left database ----------------------
		
		if(flag){
			//Count left database
			status = store_left->Get(leveldb::ReadOptions(), key, &existing_value);
			std::string last_value;
			last_value.assign(last);
			last_value.resize(FP_SIZE);

			if(status.ok())
			{
				int pos = 0;
				int xi = 0;
				while((unsigned int)pos < existing_value.size()-1)
				{
					if(memcmp(existing_value.c_str()+pos, last_value.c_str(), FP_SIZE) == 0)
					{
						xi = 1;//this chunk is already existed in left_db
						break;
					}
					pos += FP_SIZE + sizeof(uint64_t);
				}
				if(xi == 0)
				{
					existing_value += last_value;
					std::string str_count;
					str_count.resize(sizeof(uint64_t));
					uint64_t init_value = 1;
					str_count.assign((char*)&init_value, sizeof(uint64_t));
					existing_value += str_count;
				}else
				{
					const char* tc = existing_value.c_str()+pos+FP_SIZE;
					uint64_t icm = *(uint64_t*)tc;
					icm ++;
					char tmp[sizeof(uint64_t)];
					memcpy(tmp, (const char*)&icm, sizeof(uint64_t));
					existing_value.replace(pos+FP_SIZE, sizeof(uint64_t), tmp, sizeof(uint64_t));
				}
				status = store_left->Delete(leveldb::WriteOptions(), key);
			}else
			{	if(status.IsNotFound())
				{
					existing_value = last_value;
					std::string i_str;
					i_str.resize(sizeof(uint64_t));
					uint64_t i_v = 1;
					i_str.assign((char*)&i_v, sizeof(uint64_t));
					existing_value += i_str;
				}else printf("some error:%s\n", status.ToString().c_str());
			}

			leveldb::Slice current(existing_value.c_str(), existing_value.size());
			status = store_left->Put(leveldb::WriteOptions(), key, current);
		//-----------------------count left database finished, start count right database ---------------------- 
			//Count right database, using the same method as left count;
			leveldb::Slice pre(last, FP_SIZE);
			status = store_right->Get(leveldb::ReadOptions(), pre, &existing_value);
			std::string next_value;
			next_value.assign(hash);
			next_value.resize(FP_SIZE);

			if(status.ok())
			{
				int ind = 0;
				int yi = 0;
				while((unsigned int)ind < existing_value.size()-1)
				{
					if(memcmp(existing_value.c_str()+ind, next_value.c_str(), FP_SIZE) == 0)
					{
						yi = 1;
						break;
					}
					ind += FP_SIZE + sizeof(uint64_t);
				}
				if(yi == 0)
				{
					existing_value += next_value;
					std::string str_count;
					str_count.resize(sizeof(uint64_t));
					uint64_t init_value = 1;
					str_count.assign((char*)&init_value, sizeof(uint64_t));
					existing_value += str_count;
				}else
				{
					const char* tc = existing_value.c_str()+ind+FP_SIZE;
					uint64_t icm = *(uint64_t*)tc;
					icm ++;
					char tmp[sizeof(uint64_t)];
					memcpy(tmp, (const char*)&icm, sizeof(uint64_t));
					existing_value.replace(ind+FP_SIZE, sizeof(uint64_t), tmp, sizeof(uint64_t));
				}
				status = store_right->Delete(leveldb::WriteOptions(), pre);
			}else
			{
				if(status.IsNotFound())
				{
					existing_value = next_value;
					std::string i_str;
					i_str.resize(sizeof(uint64_t));
					uint64_t i_v = 1;
					i_str.assign((char*)&i_v, sizeof(uint64_t));
					existing_value += i_str;
				}else printf("there is %s\n", status.ToString().c_str());
			}

			leveldb::Slice now(existing_value.c_str(), existing_value.size());
			status = store_right->Put(leveldb::WriteOptions(), pre, now);
		}

		// update last chunk
		memcpy(last, hash, FP_SIZE);
		if(flag == 0) flag = 1;
	
	}
	//-------------------------------------counting finished--------------------------------------- 
}

int main (int argc, char *argv[])
{
	assert(argc >= 5);
	// argv[1] points to hash file; argv[2] points to frequncy db, argv[3] points to left  db, argv[4] points to right db;  

	init_db(argv[2]);
	init_left(argv[3]);
	init_right(argv[4]);

	FILE *fp = NULL;
	fp = fopen(argv[1], "r");
	assert(fp != NULL);
	read_hashes(fp);

	fclose(fp);
	delete db;
	db = NULL;
	return 0;
}
