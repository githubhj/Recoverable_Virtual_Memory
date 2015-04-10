#ifndef __LIBRVM_INT__
#define __LIBRVM_INT__


#include <iostream>
#include <map>

using namespace std;

/*
 * Segment data structure:
 * Stores mapping info
 * size occupied
 * base address*/
typedef struct _segment_t{
	bool mapped;
	bool already_being_used;
	int size_occupied;
	char* base_addr;
}segment_t;

/*
 * Segment map:
 * Contains name to data structure mapping*/
typedef map<const char*, segment_t*> segment_map_t;
typedef pair<const char*, segment_t*> segment_pair_t;
typedef map<char*, const char*> segment_addrmap_t;
typedef map<long long,char*> transaction_dir_map_t;
typedef pair<long long,char*> transaction_dir_pair_t;

typedef const char* rvm_t;

typedef long long trans_t;

int get_file_size(const char* filepath);

#endif
