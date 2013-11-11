#include <iostream>
#include "superwordsearch.h"

using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		cout << "Need one input file..." << endl;
		return -1;
	}

	SuperWordSearch sws(argv[1]);
	sws.superSearch();

	return 0;
}
