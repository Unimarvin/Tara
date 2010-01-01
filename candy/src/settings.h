/* *****************************************************
 * this header includes all globally used headers
 * *****************************************************/

// if you need assertions, always include these headers in _this_ order
// header from configure and cassert library
#include "config.h"
#include <cassert>

// C Standard General Utilities Library: EXIT_SUCCESS, atoi(), etc.
#include <cstdlib>
// C Header for UINT_MAX, etc.
#include <climits>

// standard header for declaring objects that control reading from
// and writing to the standard streams
#include <iostream>
#include <fstream>
// C++ strings library
#include <string>

// map container
#include <map>
// list container
#include <list>

// header from gengetopt
#include "cmdline.h"

// used namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::string;

using std::pair;
using std::map;
using std::list;
