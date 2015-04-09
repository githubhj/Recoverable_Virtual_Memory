#include "rvm.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
using namespace std;

/*Global Variables*/
segment_map_t segment_map;
segment_addrmap_t segment_addrmap;


int get_file_size(const char* filepath){
	//Read the file to find out its size

	if(filepath == NULL){
		cout  << "ERROR: get_file_size(): filepath is incorrect" << endl;
	}

	//Read the file
	ifstream fp(filepath, ios::binary | ios::ate);

	//check if file exits
	if(fp.good()){
		//Return the present pointer in the file
		return fp.tellg();
	}

	else{
		return -1;
	}
}

const char* create_segpath(const char* dir, const char* filename){
	return string( string(dir) + string("\\") + string(filename)).c_str();
}

/*Initialize a directory containing logs*/
rvm_t rvm_init(const char* directory){
	string make_dir("mkdir");
	make_dir = make_dir + string(" ") + string(directory);
	system(make_dir.c_str());
	return directory;
}



void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){
	const char* filepath = create_segpath(rvm,segname);
	int present_filesize = get_file_size(filepath);

	//Segment not present in directory
	if(present_filesize == -1){
		ofstream segfile(filepath);
		segfile.close();

		segment_t* temp_seg_ptr = (segment_t*)malloc(sizeof(segment_t));
		temp_seg_ptr->base_addr = (char*)malloc(sizeof(size_to_create));
		temp_seg_ptr->mapped = 1;
		temp_seg_ptr->size_occupied = size_to_create;

		segment_map.insert(segment_pair_t(segname,temp_seg_ptr));
	}


	return NULL;
}


void rvm_unmap(rvm_t rvm, void *segbase){}


void rvm_destroy(rvm_t rvm, const char *segname){}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){}


void rvm_commit_trans(trans_t tid){}


void rvm_abort_trans(trans_t tid){}


void rvm_truncate_log(rvm_t rvm){}
