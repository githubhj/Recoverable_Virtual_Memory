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
transaction_dir_map_t transaction_dir_map;

trans_t transaction_id_count=0;


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
		temp_seg_ptr->base_addr = (char*)malloc(size_to_create);
		temp_seg_ptr->mapped = true;
		temp_seg_ptr->already_being_used = true;
		temp_seg_ptr->size_occupied = size_to_create;

		segment_map.insert(segment_pair_t(segname,temp_seg_ptr));

	}
	//If segment is present in the directory
	else{
		//If not on the hash map
		if(segment_map.find(segname) == segment_map.end()){
			segment_t* temp_seg_ptr = (segment_t*)malloc(sizeof(segment_t));
			temp_seg_ptr->base_addr = (char*)malloc(size_to_create);
			temp_seg_ptr->mapped = true;
			temp_seg_ptr->already_being_used = true;
			temp_seg_ptr->size_occupied = size_to_create;
			segment_map.insert(segment_pair_t(segname,temp_seg_ptr));
		}

		//If in the hash map
		else{
			//If it is mapped
			if(segment_map[segname]->mapped == true){

				//If data file size is lesser than size_to_create
				if(present_filesize < size_to_create){
					segment_map[segname]->base_addr = realloc(segment_map[segname]->base_addr,size_to_create);
					segment_map[segname]->mapped =  true;
					segment_map[segname]->already_being_used = true;
					segment_map[segname]->size_occupied = size_to_create;
				}

				//Else return
				else{
					return NULL;
				}

			}

			//If not mapped
			else{
				segment_map[segname]->base_addr = (char*)malloc(size_to_create);
				segment_map[segname]->mapped = true;
				segment_map[segname]->already_being_used = true;
				segment_map[segname]->size_occupied = size_to_create;
			}

		}

	}


	return NULL;
}


void rvm_unmap(rvm_t rvm, void *segbase){}


void rvm_destroy(rvm_t rvm, const char *segname){}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){

	//Check if any segment is not mapped or if any segment is being used in anyother transaction
	for(int i =0 ; i<numsegs ; i++){
		//Get segment name from segment addr - name map
		char* segment_name = segment_addrmap[segbases[i]];

		//Look for segment name in segment name to segment data_str map
		if(segment_map.find(segment_name) == segment_map.end()){
			cout << "Error: rvm_begin_trans(): segment not found in segment map, mapping required" << endl;
			return -1;
		}
		else if(segment_map[segment_name]->already_being_used == true){
			cout << "Error: rvm_begin_trans(): segment found is in use, Segment name: " << segment_name << endl;
			return -1;
		}
	}

	//Reached here means everything good
	//Increment global transaction count
	transaction_id_count++;

	//insert in transaction to directory map
	transaction_dir_map.insert(transaction_dir_pair_t(transaction_id_count,rvm));

	//Return transaction id
	return transaction_id_count;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){}


void rvm_commit_trans(trans_t tid){}


void rvm_abort_trans(trans_t tid){}


void rvm_truncate_log(rvm_t rvm){}
