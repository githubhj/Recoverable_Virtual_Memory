/*
 * main.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: harshit
 */


#include "rvm.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>


using namespace std;

int main(int argc, char** argv){

	const char* dir = "My_new_dir";
	//rvm_t rvm = rvm_init((const char*)"dir");
	//cout << rvm << endl;

	//void * ptr = rvm_map(rvm, "myseg", 1000);

	int* a = (int*) malloc(2*sizeof(int));

	a[0]= 1;
	a[1] = 2;

	a = (int*) realloc(a,3*sizeof(int));

	cout << a[0] << "\n" << a[1] << "\n" << a[2]<< endl;

	char * b = (char*)malloc(10);
	char* c = "I am an indian\n";

	cout << c << endl;

	b=  strncpy(b,c,sizeof(char)*5);
	cout << string(b) << endl;

	ofstream outfile;
	outfile.open("logfile.txt", std::ios_base::out|std::ios_base::app);
	outfile << "I am not home" << " ::seperator:: " << "The end" << "\n";
	outfile << "I am not home done" << " ::seperator:: " << "The end done" << "\n";
	outfile.close();
	ofstream newfile;
	newfile.open("logfile.txt", std::ios_base::in | std::ios_base::out| std::ios_base::binary);
	streamoff pos =9;
	newfile.seekp(pos,std::ios_base::beg);
	newfile << "I am what I am\n"	;
	newfile.close();

	string s = "datadatadata::seperator::i1231o231";
	string delim = "::seperator::";
	cout << s.substr(0,s.find(delim)) << endl;
	cout << s.substr(s.find(delim)+13) << endl;



}
