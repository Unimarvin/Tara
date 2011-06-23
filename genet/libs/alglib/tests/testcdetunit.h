
#ifndef _testcdetunit_h
#define _testcdetunit_h

#include "ap.h"
#include "ialglib.h"

#include "clu.h"
#include "cdet.h"


/*************************************************************************
Main unittest subroutine
*************************************************************************/
bool testcdet(bool silent);


/*************************************************************************
Silent unit test
*************************************************************************/
bool testcdetunit_test_silent();


/*************************************************************************
Unit test
*************************************************************************/
bool testcdetunit_test();


#endif