#include "rvm.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
using namespace std;

/*Global Variables*/
segment_map_t segment_map;
segment_addrmap_t segment_addrmap;
transaction_rvm_map_t transaction_rvm_map;
undo_log_map_t undo_log_map;
redo_log_map_t redo_log_map;

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
					segment_map[segname]->base_addr = (char*)realloc(segment_map[segname]->base_addr,size_to_create);
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


void rvm_unmap(rvm_t rvm, void *segbase){

}


void rvm_destroy(rvm_t rvm, const char *segname){}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){

	//Check if any segment is not mapped or if any segment is being used in anyother transaction
	for(int i =0 ; i<numsegs ; i++){
		//Get segment name from segment addr - name map
		const char* segment_name = segment_addrmap[(char* )segbases[i]];

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
	transaction_rvm_map.insert(transaction_rvm_pair_t(transaction_id_count,rvm));

	//Return transaction id
	return transaction_id_count;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){
	if(tid > transaction_id_count){
		cout << "Error: rvm_about_to_modify() : tid is invalid, please enter correct valid id" << endl;
		return;
	}
	else if(transaction_rvm_map.find(tid)==transaction_rvm_map.end()){
		cout << "Error: rvm_about_to_modify(): tid not found in transaction rvm map" << endl;
		return;
	}
	else{
		//Transaction is valid(Hope so)

		//Get a undo log segment on heap
		undo_log_segment_t* undo_log_segment_ptr = (undo_log_segment_t*)malloc(sizeof(undo_log_segment_t));

		//Undo log data structure
		undo_log_segment_ptr->segment_addr = (char*)segbase;
		undo_log_segment_ptr->segment_data = (char*) malloc(size);
		undo_log_segment_ptr->segment_data = strncpy(undo_log_segment_ptr->segment_data, (char*)segbase + offset, size);
		undo_log_segment_ptr->segment_size = size;
		undo_log_segment_ptr->segment_offset = offset;

		//Create a redo log
		redo_log_segment_t* redo_log_segment_ptr = (redo_log_segment_t*)malloc(sizeof(redo_log_segment_t));

		//Undo log data structure
		redo_log_segment_ptr->segment_addr = (char*)segbase;
		redo_log_segment_ptr->segment_data = NULL;
		redo_log_segment_ptr->segment_offset = offset;

		//Push undo log on transaction - undo log map
		//If not found on map
		if(undo_log_map.find(tid) == undo_log_map.end()){
			undo_log_vector_t temp_undo_log_vector;
			temp_undo_log_vector.push_back(undo_log_segment_ptr);
			undo_log_map.insert(undo_log_pair_t(tid,temp_undo_log_vector));
		}
		else{
			undo_log_map[tid].push_back(undo_log_segment_ptr);
		}

		//Push redo log on transaction - redo log map
		if(redo_log_map.find(tid) == redo_log_map.end()){
			redo_log_vector_t temp_redo_log_vector;
			temp_redo_log_vector.push_back(redo_log_segment_ptr);
			redo_log_map.insert(redo_log_pair_t(tid,temp_redo_log_vector));
		}
		else{
			redo_log_map[tid].push_back(redo_log_segment_ptr);
		}

	}
}

void rvm_commit_trans(trans_t tid){
	//Committing transaction, means commit the redo log, don't truncate
	//Means just reate a log file with offset info and data

	//Check if this tid is valid or not
	if(tid > transaction_id_count){
		cout << "Error: rvm_commit_trans() : tid is invalid, please enter correct valid id" << endl;
		return;
	}
	else if(transaction_rvm_map.find(tid)==transaction_rvm_map.end()){
		cout << "Error: rvm_commit_trans(): tid not found in transaction rvm map" << endl;
		return;
	}

	else if(redo_log_map.find(tid) == redo_log_map.end()){
		cout << "Error: rvm_commit_trans(): tid not found in trasaction redo log map" << endl;
		return;
	}

	else{
		for(int i=0; i<redo_log_map[tid].size() ; i++){
			char * temp_addr = redo_log_map[tid][i]->segment_addr;
			const char* temp_segment_name = segment_addrmap[temp_addr];
			int temp_offset = redo_log_map[tid][i]->segment_offset;
			redo_log_map[tid][i]->segment_data = temp_addr + temp_offset;

			//Now we need to create a log file

			const char* logfile = string( string(transaction_rvm_map[tid]) + "\\" + temp_segment_name + ".logfile").c_str();
			ofstream logfile_handler(logfile, ios::out | ios::app);

		}
	}
}


void rvm_abort_trans(trans_t tid){

}


void rvm_truncate_log(rvm_t rvm){

}
