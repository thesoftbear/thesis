#include "error.hpp"

#include <iostream>

error::error(string message)
{
	cerr << "[error] " << message << endl;
	cin.get();
}

error::~error()
{
	exit(-1);
}