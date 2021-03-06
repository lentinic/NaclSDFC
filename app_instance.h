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

#ifndef HH_APP_INSTANCE_HH
#define HH_APP_INSTANCE_HH

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/image_data.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/input_event.h>

class AppInstance : public pp::Instance 
{
public:
	explicit AppInstance(PP_Instance instance);
	virtual ~AppInstance();

	virtual void HandleMessage(const pp::Var & var_message);
	virtual bool HandleInputEvent(const pp::InputEvent & event);
	virtual void DidChangeView(const pp::Rect & position, const pp::Rect & clip);
	virtual void Paint();

	void FlushComplete() { bFlushIsPending = false; }

private:
	void FlushPixelBuffer();
	void CreateContext(const pp::Size & size);
	void DestroyContext();

	pp::Graphics2D *	context;
	pp::ImageData *		pixels;
	int 				nWidth;
	int 				nHeight;
	bool				bFlushIsPending;
};

#endif // HH_APP_INSTANCE_HH
