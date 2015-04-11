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
}
