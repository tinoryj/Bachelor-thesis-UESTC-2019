CC = g++ 
CFLAGS = -O3 -Wall -fno-operator-names -std=c++0x
LIBS = -lcrypto -lssl -lpthread -lsnappy 
INCLUDES =-I./leveldb/include/
MAIN_OBJS = attack count tran2vm_without_zero tran2vm_with_zero


locality_attack: ./locality_count.cc
	$(shell ! test -d "dbs" && mkdir dbs)
	$(shell ! test -d "tmp" && mkdir tmp)
	$(CC) $(CFLAGS) -o count locality_count.cc $(INCLUDES) ./leveldb/out-static/libleveldb.a $(LIBS)
	$(CC) $(CFLAGS) -o tran2vm_without_zero tran2vm_without_zero.cc $(INCLUDES) ./leveldb/out-static/libleveldb.a $(LIBS)
	$(CC) $(CFLAGS) -o tran2vm_with_zero tran2vm_with_zero.cc $(INCLUDES) ./leveldb/out-static/libleveldb.a $(LIBS)
	$(CC) $(CFLAGS) -o attack entropy_attack.cc $(INCLUDES) ./leveldb/out-static/libleveldb.a $(LIBS) 
clean:
	@rm -rf tmp/
	@rm -f $(MAIN_OBJS)
	@rm -rf dbs/
