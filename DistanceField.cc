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

#include "DistanceField.h"
#include <math.h>
#include <float.h>

///////////////////////////////////////////////////////////////////////////////
//
// ------------------------------ DistanceField -------------------------------
//
///////////////////////////////////////////////////////////////////////////////
DistanceField::DistanceField()
:	pValues(NULL),
	fWidth(0.f),
	nResolution(0)
{}
///////////////////////////////////////////////////////////////////////////////
DistanceField::~DistanceField()
{
	delete [] pValues;
}
///////////////////////////////////////////////////////////////////////////////
void DistanceField::Create(int nresolution, float meters)
{
	nresolution++;
	pValues = new float[nresolution * nresolution];

	fWidth = meters * (nresolution / (float)(nresolution - 1));
	nResolution = nresolution;

	for (int y=0; y<nresolution; y++)
	{
		for (int x=0; x<nresolution; x++)
		{
			pValues[(y * nresolution) + x] = FLT_MAX;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
void DistanceField::AddCircle(float x, float y, float r)
{
	for (int iy=0; iy<nResolution; iy++)
	{
		float fy = (iy / (float) nResolution) * fWidth;

		for (int ix=0; ix<nResolution; ix++)
		{
			float fx = (ix / (float) nResolution) * fWidth;

			float dx = fx - x;
			float dy = fy - y;
			float d = sqrt(dx*dx + dy*dy) - r;

			int i = (iy * nResolution) + ix;
			float current = pValues[i];
			pValues[i] = (current < d) ? current : d;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
void DistanceField::SubCircle(float x, float y, float r)
{
	for (int iy=0; iy<nResolution; iy++)
	{
		float fy = (iy / (float) nResolution) * fWidth;

		for (int ix=0; ix<nResolution; ix++)
		{
			float fx = (ix / (float) nResolution) * fWidth;

			float dx = fx - x;
			float dy = fy - y;
			float d = r - sqrt(dx*dx + dy*dy);

			int i = (iy * nResolution) + ix;
			float current = pValues[i];
			pValues[i] = (current < d) ? current : d;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
float DistanceField::SampleDistance(int x, int y) const
{
	x = (x < 0) ? 0 : ((x > nResolution) ? nResolution : x);
	y = (y < 0) ? 0 : ((y > nResolution) ? nResolution : y);
	return pValues[y * nResolution + x];
}
///////////////////////////////////////////////////////////////////////////////
float DistanceField::SampleDistance(float x, float y) const
{
	x = (x / fWidth) * nResolution;
	y = (y / fWidth) * nResolution;
	int ix = (int) x;
	int iy = (int) y;
	float dx = x - floor(x);
	float dy = y - floor(y);

	float d0 = SampleDistance(ix, iy);
	float d1 = SampleDistance(ix + 1, iy);
	float d2 = SampleDistance(ix, iy + 1);
	float d3 = SampleDistance(ix + 1, iy + 1);

	d0 = d0 * (1.f - dx) + d1 * dx;
	d1 = d2 * (1.f - dx) + d3 * dx;
	return d0 * (1.f - dy) + d1 * dy;
}
///////////////////////////////////////////////////////////////////////////////
void DistanceField::SampleGradient(float x, float y, float * outx, float * outy) const
{
	float d0 = SampleDistance(x, y - (0.5f / nResolution));
	float d1 = SampleDistance(x - (0.5f / nResolution), y);
	float d2 = SampleDistance(x + (0.5f / nResolution), y);
	float d3 = SampleDistance(x, y + (0.5f / nResolution));

	*outx = (d2 - d1) * nResolution;
	*outy = (d3 - d0) * nResolution;
}
///////////////////////////////////////////////////////////////////////////////
float DistanceField::SampleNormal(float x, float y, float * outx, float * outy) const
{
	float gx, gy;
	SampleGradient(x,y,&gx,&gy);
	float len = sqrtf(gx*gx + gy*gy);
	*outx = gx / len;
	*outy = gy / len;
	return len;
}
///////////////////////////////////////////////////////////////////////////////
