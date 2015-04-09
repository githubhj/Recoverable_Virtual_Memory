/*
 * main.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: harshit
 */


#include "rvm.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv){

	const char* dir = "My_new_dir";
	rvm_t rvm = rvm_init((const char*)"dir");
	cout << rvm << endl;

	void * ptr = rvm_map(rvm, "myseg", 1000);
}
