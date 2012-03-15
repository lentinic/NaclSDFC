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

#include "app_instance.h"
#include <stdio.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/Var.h>
#include "Util.h"

///////////////////////////////////////////////////////////////////////////////
void FlushCallback(void * data, int32_t result)
{
	((AppInstance *) data)->FlushComplete();
}
///////////////////////////////////////////////////////////////////////////////

extern void InitSimulation(int);
extern void ShutdownSimulation();
extern void UpdateSimulation(float);
extern void RenderSimulation(int32_t *, int, int);
extern void AddMousePuff(float x, float y);
extern void ToggleSurface();
extern void ToggleDistance();
extern void ToggleFiltering();

///////////////////////////////////////////////////////////////////////////////
//
// ------------------------------- AppInstance --------------------------------
//
///////////////////////////////////////////////////////////////////////////////
AppInstance::AppInstance(PP_Instance instance)
	:	pp::Instance(instance),
		context(NULL),
		pixels(NULL),
		nWidth(0),
		nHeight(0),
		bFlushIsPending(false)
{
	RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
	InitSimulation(10000);
}
///////////////////////////////////////////////////////////////////////////////
AppInstance::~AppInstance()
{
	ShutdownSimulation();
	DestroyContext();
}
///////////////////////////////////////////////////////////////////////////////
void AppInstance::HandleMessage(const pp::Var & var_message)
{
	if (!var_message.is_string())
		return;

	std::string message = var_message.AsString();
	if (message == "paint")
	{
		Paint();
	}
	else if (message == "ToggleSurface")
	{
		ToggleSurface();
	}
	else if (message == "ToggleDistance")
	{
		ToggleDistance();
	}
	else if (message == "ToggleFiltering")
	{
		ToggleFiltering();
	}
}
///////////////////////////////////////////////////////////////////////////////
bool AppInstance::HandleInputEvent(const pp::InputEvent & event)
{
	if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE)
	{
		pp::MouseInputEvent mouse(event);
	
		float x = mouse.GetPosition().x() / (float) nWidth;
		float y = 1.f - mouse.GetPosition().y() / (float) nHeight;
		AddMousePuff(x, y);
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
void AppInstance::DidChangeView(const pp::Rect & position,
								const pp::Rect & clip)
{
	if (position.size().width() == nWidth &&
		position.size().height() == nHeight)
	{
		return;
	}

	nWidth = position.size().width();
	nHeight = position.size().height();

	DestroyContext();
	CreateContext(position.size());

	delete pixels;
	pixels = NULL;

	if (!context)
		return;

	pixels = new pp::ImageData(this, PP_IMAGEDATAFORMAT_BGRA_PREMUL,
		context->size(), false);
}
///////////////////////////////////////////////////////////////////////////////
void AppInstance::Paint()
{
	char msg[255];

	int64_t start, end;
	start = GetTimeMS();

	UpdateSimulation(1.f / 30.f);

	end = GetTimeMS();

	sprintf(msg, "Update Time: %d ms", (int)(end - start));
	PostMessage(pp::Var(msg));

	start = GetTimeMS();

	RenderSimulation((int32_t *) pixels->data(), nWidth, nHeight);
	FlushPixelBuffer();

	end = GetTimeMS();
	sprintf(msg, "Render Time: %d ms", (int)(end - start));
	PostMessage(pp::Var(msg));
}
///////////////////////////////////////////////////////////////////////////////
void AppInstance::FlushPixelBuffer()
{
	if (!context)
		return;
		
	context->PaintImageData(*pixels, pp::Point());

	if (bFlushIsPending)
		return;

	context->Flush(pp::CompletionCallback(&FlushCallback, this));
}
///////////////////////////////////////////////////////////////////////////////
void AppInstance::CreateContext(const pp::Size & size)
{
	context = new pp::Graphics2D(this, size, false);
	if (!BindGraphics(*context))
	{
		printf("Error:  could not bind device context\n");
	}
}
///////////////////////////////////////////////////////////////////////////////
void AppInstance::DestroyContext()
{
	delete context;
	context = NULL;
}
///////////////////////////////////////////////////////////////////////////////

