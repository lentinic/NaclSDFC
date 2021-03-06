/*
Copyright (c) 2012 Chris Lentini
http://divergentcoder.com

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef HH_SDFC_UTIL_HH
#define HH_SDFC_UTIL_HH
#include <math.h>
#include <sys/time.h>

///////////////////////////////////////////////////////////////////////////////
inline int64_t GetTimeMS() 
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (int64_t)(t.tv_sec) * 1000 + (t.tv_usec / 1000);
}
///////////////////////////////////////////////////////////////////////////////
inline float frand()
{
	return rand() / (float)RAND_MAX;
}
///////////////////////////////////////////////////////////////////////////////

#endif // HH_SDFC_UTIL_HH
