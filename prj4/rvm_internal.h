#ifndef __LIBRVM_INT__
#define __LIBRVM_INT__


#include <iostream>
#include <map>
#include <vector>

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
 * Undo log data str
 * Segment name: name of the segment to which this undo log belongs
 * data of the segment before modifcation
 * segment size before modification
 * segment offet before modificaiton*/
typedef struct _undo_log_segment_t{
	char* segment_addr;
	char* segment_data;
	int segment_size;
	int segment_offset;
}undo_log_segment_t;

/*
 * Redo log data str
 * Segment name: name of the segment to which this undo log belongs
 * data of the segment before modifcation
 * segment size before modification
 * segment offet before modificaiton*/
typedef struct _redo_log_segment_t{
	char* segment_addr;
	char* segment_data;
	int segment_offset;
}redo_log_segment_t;

/*Typedfs*/

//RVM typedefing to directory name
typedef const char* rvm_t;

//transaction type defined to a long long int
typedef long long int trans_t;

/*
 * Segment map:
 * Contains name to data structure mapping*/
typedef map<const char*, segment_t*> segment_map_t;

/*
 * Segment map pair:
 * Contains name to data structure pair*/
typedef pair<const char*, segment_t*> segment_pair_t;

/*
 * Segment name to addr map :
 * Contains address to segment name mapping*/
typedef map<char*, const char*> segment_addrmap_t;

/*
 * Transaction id and rvm direcotry map:
 * Contains Transaction id to rvm directory map*/
typedef map<long long,char*> transaction_rvm_map_t;

/*
 * Transaction id and rvm direcotry pair:
 * Contains Transaction id to rvm directory pair*/
typedef pair<long long,rvm_t> transaction_rvm_pair_t;

/*
 * Undo and Redo log maps
 * Containing mapping from transaction id to vector of redo and undo log segment pointers*/
typedef vector<undo_log_segment_t*> undo_log_vector_t;
typedef map<trans_t, undo_log_vector_t> undo_log_map_t;
typedef pair<trans_t, undo_log_vector_t> undo_log_pair_t;

typedef vector<redo_log_segment_t*> redo_log_vector_t;
typedef map<trans_t, redo_log_vector_t > redo_log_map_t;
typedef pair<trans_t, redo_log_vector_t> redo_log_pair_t;



int get_file_size(const char* filepath);

#endif
