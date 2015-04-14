#include "rvm.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
using namespace std;

//edit test
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
		temp_seg_ptr->already_being_used = false; //CHANGE 
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
			temp_seg_ptr->already_being_used = false; //CHANGE
			temp_seg_ptr->size_occupied = size_to_create;
			segment_map.insert(segment_pair_t(segname,temp_seg_ptr));

			//CHANGE
			//COMMIT LOG FILE TO SEGMENT FILE
			//COPY SEGMENT FROM DISK
		}

		//If in the hash map
		else{
			//If it is mapped
			if(segment_map[segname]->mapped == true){

				//If data file size is lesser than size_to_create
				if(present_filesize < size_to_create){
					segment_map[segname]->base_addr = (char*)realloc(segment_map[segname]->base_addr,size_to_create);
					segment_map[segname]->mapped =  true;
					segment_map[segname]->already_being_used = false; //CHANGE
					segment_map[segname]->size_occupied = size_to_create;

					//CHANGE
					//COMMIT LOG FILE TO SEGMENT FILE
					//COPY SEGMENT FROM DISK
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
				segment_map[segname]->already_being_used = false; //CHANGE
				segment_map[segname]->size_occupied = size_to_create;
			}

		}

	}


	return NULL;
}


void rvm_unmap(rvm_t rvm, void *segbase){
	const char* temp_segment_name = segment_addrmap[(char* ) segbase];
	segment_map[temp_segment_name]->base_addr = NULL;
	segment_map[temp_segment_name]->mapped = false;
}


void rvm_destroy(rvm_t rvm, const char *segname){
	//check whether the segment is mapped
	//if yes then cannot destroy
	if(segment_map[segname]->mapped == true)
	{
		cout << "Cannot deleted a mapped segment" <<endl;
		return;
	}


	const char* filepath = create_segpath(rvm, segname);
	string rm("rm");
	rm = rm + string(" ") + string(filepath);
	 //Delete entry from Hash Map

	segment_map_t::iterator it = segment_map.find(segname);

	if(it == NULL)
		cout << "Invalid Call to Destroy as segment does not exist" << endl;

	segment_map.erase(it);

	system(rm.c_str());


}

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

		//CHANGE: SHOULDN'T we set the in_use flag of these segments here?
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
			ofstream logfile_handler;
			logfile_handler.open(logfile,std::ios_base::app);
			logfile_handler << redo_log_map[tid][i]->segment_offset << "~~::separator::~~" << redo_log_map[tid][i]->segment_data;
			logfile_handler.close();
		}

		//Remove log from undo, redo and transaction maps
		transaction_rvm_map.erase(tid);
		undo_log_map.erase(tid);
		redo_log_map.erase(tid);
	}
}


void rvm_abort_trans(trans_t tid){
	//Check if this tid is valid or not
	if(tid > transaction_id_count){
		cout << "Error: rvm_commit_trans() : tid is invalid, please enter correct valid id" << endl;
		return;
	}
	else if(transaction_rvm_map.find(tid)==transaction_rvm_map.end()){
		cout << "Error: rvm_commit_trans(): tid not found in transaction rvm map" << endl;
		return;
	}

	else if(undo_log_map.find(tid) == undo_log_map.end()){
		cout << "Error: rvm_abort_trans(): tid not found in transaction undo log map" << endl;
		return;
	}

	else{

		for(int i=0; i<undo_log_map[tid].size() ; i++){
			char * temp_addr = undo_log_map[tid][i]->segment_addr;
			const char* temp_segment_name = segment_addrmap[temp_addr];
			int temp_offset = undo_log_map[tid][i]->segment_offset;
			int temp_size = undo_log_map[tid][i]->segment_size;
			strncpy(segment_map[temp_addr]->base_addr + temp_offset ,undo_log_map[tid][i]->segment_data, temp_size);
		}

		//Remove log from undo, redo and transaction maps
		transaction_rvm_map.erase(tid);
		undo_log_map.erase(tid);
		redo_log_map.erase(tid);
	}
}


void rvm_truncate_log(rvm_t rvm){
	DIR *mylocation;
	vector<string> filelist;
	struct dirent* dir;
	mylocation = opendir(("./"+string(rvm)).c_str());
	if (mylocation)
	{
		while (dir = (readdir(mylocation)) != NULL)
		{
			string filename(dir->d_name);
			if (filename.find(".logfile") != string::npos)
			{
				filelist.push_back(dir->d_name);
			}

		}
		closedir(mylocation);
	}
	for (vector<string>::iterator it = filelist.begin(); it != filelist.end();
			++it)
	{
		string file = *it;
		file = file.substr(0, s.length() - 8); //filename - '.logfile'
 		char *segment_name = (char *) file.c_str();
		segment_truncate(rvm, segment_name);
	}

}

void segment_truncate(rvm_t rvm_dir, const char* segment_name){
	const char* segment_file = create_segpath(rvm_dir,segment_name);
	const char* segment_logfile = string(string(segment_file) + ".logfile").c_str();

	//No log, just remove logfile even if it does not exist
	if(get_file_size(segment_logfile) == -1){
		const char* remove_command = string(string("rm ") + string(segment_logfile)).c_str();
		system(remove_command);
		return;
	}
	else{
		string line;
		ifstream logfile_handler(segment_logfile);
		string delim = "~~::seperator::~~"; //CHANGE
		int offset;
		char* data;
		while(getline(logfile_handler,line)){
			//Get offset
			offset = atoi(line.substr(0,line.find(delim)).c_str());
			//Get data
			data = line.substr(line.find(delim)+delim.length()).c_str();

			ofstream segment_file_handler;
			segment_file_handler.open(segment_file, std::ios_base::in | std::ios_base::out| std::ios_base::binary);
			streamoff position = offset;
			//Check this
			segment_file_handler.seekp(position-1,ios::beg);
			segment_file_handler << data;
			segment_file_handler.close();
		}
	}

}
