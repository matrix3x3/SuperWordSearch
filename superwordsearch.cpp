#include "./superwordsearch.h"
#include <pthread.h>

// increments in each direction
int const SuperWordSearch::_increments[_numOfDirects][_dimentions]
	= {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

SuperWordSearch::SuperWordSearch(): _N(0), _M(0), _K(0),
	_board(NULL), _words(NULL), _isWrap(NULL)
{
}

SuperWordSearch::~SuperWordSearch()
{
	for (int i = 0; i < _N; i++)
		delete [] _board[i];
	delete [] _board;

	for (int i = 0; i < _K; i++)
		delete [] _words[i];
	delete [] _words;

	delete _isWrap;

	_infile.close(); 
}

SuperWordSearch::SuperWordSearch(const char* filename) : _N(0), _M(0), _K(0),
	_board(NULL), _words(NULL), _isWrap(NULL)
{
	init(filename);
}

/*******************************************************************************
* Function Name: init
* Input Parameters: filename
* In/Out Parameters: 
* Out Parameters:
* Description: initilizes inner datastructures of Class SuperWordSearch using data
*              read from file 'filename'.
********************************************************************************/
bool SuperWordSearch::init(const char* filename)
{
	try
	{
		_infile.open(filename);
		if (!_infile)
		{
			std::cout << "File doesn't exist..." << std::endl;
			return false;
		}

		// number of rows and columns
		_infile >> _N;
		_infile >> _M;

		if (_N == 0 || _M == 0)
		{
			std::cout << "File data error..." << std::endl;
			return false;
		}

		// init puzzle board
		_board = new char*[_N];
		for (int i = 0; i < _N; i++)
		{
			_board[i] = new char[_M];
			for (int j = 0; j < _M; j++)
			{
				_infile >> _board[i][j];
			}
		}

		// WRAP or NO_WRAP
		_isWrap = new char[10];
		_infile >> _isWrap;
		if (strcmp(_isWrap, "WRAP") != 0 && strcmp(_isWrap, "NO_WRAP") != 0)
		{
			std::cout << "File data error..." << std::endl;
			return false;
		}

		// number of words
		_infile >> _K;

		if (_K == 0)
		{
			std::cout << "File data error..." << std::endl;
			return false;
		}

		_words = new char*[_K];
		for (int i = 0; i < _K; i++)
		{
			_words[i] = new char[maxWordLength];
			_infile >> _words[i];
		}

		return true;
	}
	catch(...)
	{
		std::cout << "Exception occured when reading data from input file..." << std::endl;
		return false;
	}
}


/*******************************************************************************
* Function Name: search
* Input Parameters:  
* In/Out Parameters: args of type 'struct _searchArgs'
* Out Parameters:
* Description: finds word 'args->P' in puzzle board and gives the location back via 
*              'args->low', if not found, 'end' will be set to (-1,-1)
********************************************************************************/
void* SuperWordSearch::search(void* args_void)
{	
	pSearchArgs args = (pSearchArgs)args_void;
	int length_p = strlen(args->P);
	std::list<Pos> sources;
	Pos start, end;

	// search the first character of P in board
	for (int i = 0; i < args->pObj->_N; i++)
		for (int j = 0; j < args->pObj->_M; j++)
			if (args->pObj->_board[i][j] == (args->P)[0])
				sources.push_back(Pos(i,j));

	for (std::list<Pos>::iterator it = sources.begin(); it != sources.end(); it++)
	{
		start.x = (*it).x;
		start.y = (*it).y;
		// 8 directions(clockwise): right, bottom-right, bottom, bottom-left, left, up-left, up, up-right
		// '1' means this direction is available to search
		int directions[_numOfDirects] = {1,1,1,1,1,1,1,1};

		// find available direction in 'NO_WRAP' mode
		if (strcmp(args->pObj->_isWrap, "NO_WRAP") == 0)
		{
			if (args->pObj->_M - start.y < length_p)
			{
				directions[0] = 0;
				directions[1] = 0;
				directions[7] = 0;
			}
			if (args->pObj->_N - start.x < length_p)
			{
				directions[1] = 0;
				directions[2] = 0;
				directions[3] = 0;
			}
			if (start.y + 1 < length_p)
			{
				directions[3] = 0;
				directions[4] = 0;
				directions[5] = 0;
			}
			if (start.x + 1 < length_p)
			{
				directions[5] = 0;
				directions[6] = 0;
				directions[7] = 0;
			}
		}

		// search for P in 8 directions
		for (int j = 0; j < _numOfDirects; j++)
		{
			int pos_x = (*it).x, pos_y = (*it).y;
			// in each direction, try to find the matching word
			for (int i = 1; i < length_p; i++)
			{
				if (directions[j])
				{
					pos_x = (pos_x + _increments[j][0]) % args->pObj->_N;
					if (pos_x < 0)
						pos_x += args->pObj->_N;

					pos_y = (pos_y + _increments[j][1]) % args->pObj->_M;
					if (pos_y < 0)
						pos_y += args->pObj->_M;

					if (args->pObj->_board[pos_x][pos_y] != (args->P)[i])
						directions[j] = 0;

					// P is found!
					else if (i == length_p - 1)
					{
						end.x = pos_x;
						end.y = pos_y;
						//printf("found: %d %d -> %d %d\n", start.x, start.y, end.x, end.y);
						goto end;
					}
				}
				else
				{
					// if current direction already set to 0 ---- means that cannot find matching
					// word follow this direction, so break out
					break;
				}
			}
		}
	}

end:
	// write location into 'low'
	(args->loc)->first.x = start.x;
	(args->loc)->first.y = start.y;
	(args->loc)->second.x = end.x;
	(args->loc)->second.y = end.y;

	return ((void *)0);
}

/*******************************************************************************
* Function Name: superSearch
* Input Parameters: 
* In/Out Parameters: 
* Out Parameters:
* Description: finds all words given from input file in puzzle board and gives
*              feedback on screen. 
********************************************************************************/
void SuperWordSearch::superSearch()
{
	LocationOfWord *locs = new LocationOfWord[_K];
	pthread_t * tids = new pthread_t[_K];
	pSearchArgs allArgs = new SearchArgs[_K];
	for (int i = 0; i < _K; i++)
	{
		allArgs[i].pObj = this;
		allArgs[i].P = _words[i];
		allArgs[i].loc = &locs[i];
	}

	for (int i = 0; i < _K; i++)
	{
		// search for each word in board		
		int err = pthread_create(&tids[i], NULL, search, (void*)(&allArgs[i]));
		if (err != 0) 
		{
			std::cout << "can't create thread " << i << "..." << std::endl;
			return;
		}
	}

	for (int i = 0; i < _K; i++)
	{
		pthread_join(tids[i], NULL);
	}

	for (int i = 0; i < _K; i++)
		
	{	
		// if end point is (-1,-1), current word cannot be found in board
		// start and end should not be the same
		Pos start = locs[i].first, end = locs[i].second;
		if ( (end.x == -1 && end.y == -1) || (start.x == end.x && start.y == end.y))
		{
			std::cout << "NOT FOUND" << std::endl;
		}
		else
		{
			std::cout << "(" << start.x << "," << start.y << ") (" << end.x << "," << end.y << ")" << std::endl;
		}			
	}

	delete [] locs;
	delete [] tids;
	delete [] allArgs;
}
