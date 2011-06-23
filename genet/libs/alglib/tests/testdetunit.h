
#ifndef _testdetunit_h
#define _testdetunit_h

#include "ap.h"
#include "ialglib.h"

#include "lu.h"
#include "det.h"


/*************************************************************************
Main unittest subroutine
*************************************************************************/
bool testdet(bool silent);


/*************************************************************************
Silent unit test
*************************************************************************/
bool testdetunit_test_silent();


/*************************************************************************
Unit test
*************************************************************************/
bool testdetunit_test();


#endif