// System dependent sizes and values

#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include"userconfig.h"
#include<limits>

#ifdef LOG_NEW
#include "mynew.h"
#endif

using namespace std;

#ifndef REPORTFREQUENCY
#define REPORTFREQUENCY 100
#endif

#if HASHSIZE < 1
#undef HASHSIZE
#define HASHSIZE 1
#endif

#if (HASHSIZE > 2) && (HASHSIZE < 4)
#undef HASHSIZE
#define HASHSIZE 2
#endif

#if (HASHSIZE > 4) && (HASHSIZE < 8)
#undef HASHSIZE
#define HASHSIZE 4
#endif

#if (HASHSIZE > 8) && (HASHSIZE < 16)
#undef HASHSIZE
#define HASHSIZE 8
#endif

#if (HASHSIZE > 16) && (HASHSIZE < 32)
#undef HASHSIZE
#define HASHSIZE 16
#endif

#if (HASHSIZE > 32) && (HASHSIZE < 64)
#undef HASHSIZE
#define HASHSIZE 32
#endif

#if (HASHSIZE > 64) && (HASHSIZE < 128)
#undef HASHSIZE
#define HASHSIZE 64
#endif

#if (HASHSIZE > 128) && (HASHSIZE < 256)
#undef HASHSIZE
#define HASHSIZE 128
#endif

#if (HASHSIZE > 256) && (HASHSIZE < 512)
#undef HASHSIZE
#define HASHSIZE 256
#endif

#if (HASHSIZE > 512) && (HASHSIZE < 1024)
#undef HASHSIZE
#define HASHSIZE 512
#endif

#if (HASHSIZE > 1024) && (HASHSIZE < 2048)
#undef HASHSIZE
#define HASHSIZE 1024
#endif

#if (HASHSIZE > 2048) && (HASHSIZE < 4096)
#undef HASHSIZE
#define HASHSIZE 2048
#endif

#if (HASHSIZE > 4096) && (HASHSIZE < 8192)
#undef HASHSIZE
#define HASHSIZE 4096
#endif

#if (HASHSIZE > 8192) && (HASHSIZE < 16384)
#undef HASHSIZE
#define HASHSIZE 8192
#endif

#if (HASHSIZE > 16384) && (HASHSIZE < 32768)
#undef HASHSIZE
#define HASHSIZE 16384
#endif

#if (HASHSIZE > 32768) && (HASHSIZE < 65536)
#undef HASHSIZE
#define HASHSIZE 32768
#endif

#if (HASHSIZE > 65536) && (HASHSIZE < 131072)
#undef HASHSIZE
#define HASHSIZE 65536
#endif

#if (HASHSIZE > 131072) && (HASHSIZE < 262144)
#undef HASHSIZE
#define HASHSIZE 131072
#endif

#if (HASHSIZE > 262144) && (HASHSIZE < 524288)
#undef HASHSIZE
#define HASHSIZE 262144
#endif

#if (HASHSIZE > 524288) && (HASHSIZE < 1048576)
#undef HASHSIZE
#define HASHSIZE 524288
#endif

#if (HASHSIZE > 1048576) && (HASHSIZE < 2097152)
#undef HASHSIZE
#define HASHSIZE 1048576
#endif

#if (HASHSIZE > 2097152) && (HASHSIZE < 4194304)
#undef HASHSIZE
#define HASHSIZE 2097152
#endif

#if (HASHSIZE > 4194304) && (HASHSIZE < 8388608)
#undef HASHSIZE
#define HASHSIZE 4194304
#endif

#if (HASHSIZE > 8388608) && (HASHSIZE < 16777216)
#undef HASHSIZE
#define HASHSIZE 8388608
#endif

#if (HASHSIZE > 16777216) && (HASHSIZE < 33554432)
#undef HASHSIZE
#define HASHSIZE 16777216
#endif

#if (HASHSIZE > 33554432) && (HASHSIZE < 67108864)
#undef HASHSIZE
#define HASHSIZE 33554432
#endif

#if (HASHSIZE > 67108864) && (HASHSIZE < 134217728)
#undef HASHSIZE
#define HASHSIZE 67108864
#endif

#if (HASHSIZE > 134217728) && (HASHSIZE < 268435456)
#undef HASHSIZE
#define HASHSIZE 134217728
#endif

#if (HASHSIZE > 268435456) && (HASHSIZE < 536870912)
#undef HASHSIZE
#define HASHSIZE 268435456
#endif

#if (HASHSIZE > 536870912) && (HASHSIZE < 1073741824)
#undef HASHSIZE
#define HASHSIZE 536870912
#endif

#if (HASHSIZE > 1073741824) && (HASHSIZE < 2147483648)
#undef HASHSIZE
#define HASHSIZE 1073741824
#endif

#if (HASHSIZE > 2147483648)
#undef HASHSIZE
#define HASHSIZE  2147483648
#endif

#ifndef HASHSIZE
#define HASHSIZE 65536;
#endif

#define VERYLARGE UINT_MAX
#ifndef CAPACITY
#define CAPACITY -1
#endif

void reportconfiguration();

#endif
