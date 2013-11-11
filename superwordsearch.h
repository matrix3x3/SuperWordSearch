#ifndef __SUPERSEARCH__
#define __SUPERSEARCH__

#include <string.h>
#include <list>
#include <iostream>
#include <fstream>

// type declarations
typedef struct _Pos Pos, *pPos;
typedef std::pair<Pos, Pos> LocationOfWord;
typedef struct _searchArgs SearchArgs, *pSearchArgs;

class SuperWordSearch
{
public:
	SuperWordSearch();
	SuperWordSearch(const char* filename);
	~SuperWordSearch();
	// initilize data members using data read from input file
	bool init(const char* filename);
	// search for words in puzzle board
	void superSearch();

private:
	SuperWordSearch(SuperWordSearch &);
	SuperWordSearch& operator = (SuperWordSearch&);	
	
	// 8 directions need searching for
	static const int _numOfDirects = 8;
	// dimentions of board
	static const int _dimentions = 2;
	// increments in each direction
	static const int _increments[_numOfDirects][_dimentions];
	// maxmium word length need searching
	static const int maxWordLength = 255;
	std::ifstream _infile;
	int _N, _M, _K;
	char ** _board;
	char ** _words;
	char * _isWrap;
	// search word P in board , return the location
	static void* search(void* args_void);
};

// defintions
struct _Pos
{
	int x;
	int y;
	_Pos():x(-1),y(-1) {}
	_Pos(int x, int y):x(x),y(y){}
	~_Pos(){}
};
// encapsulate 'this' pointer, word 'P' and location 'loc' in a struct to make
// function 'void* search((void*)args)' can be passed into 'pthread_create(...)'
struct _searchArgs
{
	SuperWordSearch* pObj;
	const char* P;
	LocationOfWord* loc;
	_searchArgs(): pObj(NULL), P(NULL), loc(NULL){}
	_searchArgs(SuperWordSearch *pObj, const char* P, LocationOfWord* loc): pObj(pObj), P(P), loc(loc){}
	~_searchArgs() {}
	
};

#endif
