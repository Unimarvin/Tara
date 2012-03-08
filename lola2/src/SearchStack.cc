/*!
\file SearchStack.cc
\author Karsten
\status new
\brief organizes the search stack for dfs or bfs
*/

#include "Dimensions.h"
#include "SearchStack.h"

SearchStack::SearchStack()
{
	StackPointer = 0;
	currentchunk = NULL;
}
void SearchStack::push(index_t c,index_t* f)
{
	if((StackPointer % SIZEOF_STACKCHUNK) == 0)
	{
		// need new chunk
		Chunk * newchunk = new Chunk();
		newchunk -> prev = currentchunk;
		currentchunk = newchunk;
	}
	currentchunk -> current[StackPointer % SIZEOF_STACKCHUNK] = c;
	currentchunk -> list[StackPointer++ % SIZEOF_STACKCHUNK] = f;
}

void  SearchStack::pop(index_t * c, index_t ** f)
{
	if((StackPointer % SIZEOF_STACKCHUNK) == 0)
	{
		// need to jump to previous chunk
		Chunk * tempchunk = currentchunk;
		currentchunk = currentchunk -> prev;
		delete tempchunk;
	}
	* c = currentchunk -> current[--StackPointer];
	* f = currentchunk -> list[StackPointer]; 
}