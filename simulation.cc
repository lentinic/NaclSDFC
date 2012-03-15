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

#include <algorithm>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "Util.h"
#include "DistanceField.h"

struct Particle
{
	float 	x;
	float 	y;
	float 	vx;
	float 	vy;	
};

void UpdateSimulation(float);
void ResolveCollisions(Particle &, float);

#define RES 64
#define TANK_SIZE 10.f

// Simulation Parameters
float	fGravity;
float	fRestitution;
float	fFriction;

// Visualization Parameters
bool	bRenderSurface;
bool	bRenderDistance;
bool	bRenderFiltered;

DistanceField 		SDF;
Particle *			aParticles;
int 				nParticles;
///////////////////////////////////////////////////////////////////////////////
void InitSimulation(int count)
{
	fGravity = -9.8f;
	fRestitution = 0.7f;
	fFriction = 0.3f;

	bRenderDistance = false;
	bRenderSurface = true;
	bRenderFiltered = true;

	aParticles = new Particle[count];
	nParticles = count;

	for (int i=0; i<count; i++)
	{
		Particle & p = aParticles[i];
		p.x = (TANK_SIZE / 4.f) + frand() * (TANK_SIZE / 2.f);
		p.y = (TANK_SIZE - 3.f) + frand() * 0.5f;
		p.vx = 3.f - 6.f * frand();
		p.vy = 8.f;
	}

	SDF.Create(32, TANK_SIZE);
	SDF.SubCircle(5.f, 5.f, 4.5f);
	SDF.AddCircle(5.f, 5.f, 1.25f);
	SDF.AddCircle(0.f, 5.f, 2.f);
	SDF.AddCircle(10.f, 5.f, 2.f);
}
///////////////////////////////////////////////////////////////////////////////
void ShutdownSimulation()
{
	delete [] aParticles;
}
///////////////////////////////////////////////////////////////////////////////
void DrawCircle(int32_t * pixels, int xres, int yres, int x, int y, int r, int rgb = 0xff0000ff)
{
	int ulx = std::max(x - r, 0);
	int uly = std::max(y - r, 0);
	int lrx = std::min(x + r, xres - 1);
	int lry = std::min(y + r, yres - 1);
	for (int i=uly; i<lry; i++)
	{
		for (int j=ulx; j<lrx; j++)
		{
			int d2 = ((x - j) * (x - j)) + ((y - i) * (y - i));
			if (d2 <= (r * r))
			{
				pixels[(i * xres) + j] = rgb;
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
void RenderSimulation(int32_t * pixels, int xres, int yres)
{
	memset(pixels, 0, sizeof(int32_t) * xres * yres);
	
	if (bRenderDistance || bRenderSurface)
	{
		float dx = (1.f / xres) * TANK_SIZE;
		float dy = (1.f / yres) * TANK_SIZE;
		float fx = 0.f, fy = TANK_SIZE;

		for (int y=0; y<yres; y++, fy-=dy)
		{
			fx = 0.f;
			for (int x=0; x<xres; x++, fx+=dx)
			{
				float d;

				if (!bRenderFiltered)
				{
					int mx = ((x * 32) / xres);
					int my = ((y * 32) / yres);
					d = SDF.SampleDistance(mx,my);	
				}
				else
				{
					d = SDF.SampleDistance(fx, fy);	
				}

				int id = y*xres+x;
				if (d < 0.015f && d > -0.015f && bRenderSurface)
				{
					pixels[id] = 0xffff0000;
				}
				else if (bRenderDistance)
				{
					int v = fabs(d) * 255.f;
					v = v > 255 ? 255 : v;
					pixels[id] = 0xff000000 | v << 16 | v << 8 | v;
				}
			}
		}
	}

	for (int i=0; i<nParticles; i++)
	{
		Particle & p = aParticles[i];
		int x = (int)((p.x / TANK_SIZE) * (xres - 1));
		int y = (yres - 1) - (int)((p.y / TANK_SIZE) * (yres - 1));
		DrawCircle(pixels, xres, yres, x, y, 2, 0xffff7f00);
	}
}
///////////////////////////////////////////////////////////////////////////////
void UpdateSimulation(float dt)
{
	for (int i=0; i<nParticles; i++)
	{
		Particle & p = aParticles[i];

		float vy = p.vy;
		p.vy += dt * fGravity;
		p.y += (vy + p.vy) * 0.5f * dt;
		p.x += p.vx * dt;

		ResolveCollisions(p, dt);
	}
}
///////////////////////////////////////////////////////////////////////////////
float FindCollisionDT(Particle & pt, float dt0, float dt1, int iter = 0)
{
	float dt = (dt0 + dt1) * 0.5f;

	for (int i=0; i<4; i++)
	{
		float x = pt.x - pt.vx * dt;
		float y = pt.y - pt.vy * dt;
		float d = SDF.SampleDistance(x, y);

		if (d < -1e-4)
			dt0 = dt;
		else if (d > 1e-4)
			dt1 = dt;
		else
			break;

		dt = (dt0 + dt1) * 0.5f;
	}
	
	return dt;
}
///////////////////////////////////////////////////////////////////////////////
void ResolveCollisions(Particle & p, float dt)
{
	float d0 = SDF.SampleDistance(p.x, p.y);
	if (d0 < 0.f)
	{
		float dtc = FindCollisionDT(p, 0.f, dt);

		p.x -= p.vx * dtc;
		p.y -= p.vy * dtc;

		float nx, ny, l;
		d0 = SDF.SampleDistance(p.x, p.y);
		SDF.SampleNormal(p.x, p.y, &nx, &ny);

		p.x -= nx * d0;
		p.y -= ny * d0;

		SDF.SampleNormal(p.x, p.y, &nx, &ny);
	
		float tx = ny;
		float ty = -nx;

		float vdn = nx * p.vx + ny * p.vy;
		float vdt = tx * p.vx + ty * p.vy;

		float i = -fFriction * vdt;
		float j = -(1.f + fRestitution) * vdn;
		p.vx += nx * j + tx * i;
		p.vy += ny * j + ty * i;
	}
}
///////////////////////////////////////////////////////////////////////////////
void AddMousePuff(float x, float y)
{
	for (int i=0; i<nParticles; i++)
	{
		Particle & p = aParticles[i];
		float dx = p.x - (x * TANK_SIZE);
		float dy = p.y - (y * TANK_SIZE);
		float d = sqrt(dx*dx + dy*dy);
		dx /= d;
		dy /= d;
		float strength = 90.8f / 100.f;

		p.vx += (dx + (0.05f - frand() * 0.1f)) * strength;
		p.vy += (dy + (0.05f - frand() * 0.1f)) * strength;
	}
}
///////////////////////////////////////////////////////////////////////////////
void ToggleSurface()
{
	bRenderSurface = !bRenderSurface;
}
///////////////////////////////////////////////////////////////////////////////
void ToggleDistance()
{
	bRenderDistance = !bRenderDistance;
}
///////////////////////////////////////////////////////////////////////////////
void ToggleFiltering()
{
	bRenderFiltered = !bRenderFiltered;	
}
///////////////////////////////////////////////////////////////////////////////
