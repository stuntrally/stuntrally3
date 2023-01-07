#include "ViewDim.h"


void ViewDim::Default()
{
	left0 = 0.f;  top0 = 0.f;   width0 = 1.f;  height0 = 1.f;
	right0 = 1.f;  bottom0 = 1.f;  avgsize0 = 1.f;
	Upd1();
}

void ViewDim::Set01(float left, float top, float width, float height)
{
	left0 = left;  width0 = width;   right0 = left + width;
	top0 = top;   height0 = height;  bottom0 = top + height;
	avgsize0 = (width + height) * 0.25f;
	Upd1();
}

void ViewDim::Upd1()
{
	top1    = 2.f * top0    -1.f;
	left1   = 2.f * left0   -1.f;
	width1  = 2.f * width0  -1.f;
	height1 = 2.f * height0 -1.f;
	right1  = 2.f * right0  -1.f;
	bottom1 = 2.f * bottom0 -1.f;
	avgsize1= 2.f * avgsize0;
}


void ViewDim::SetDim(int views, bool horizontal, int i/*player view*/)
{
	if (views == 1)
	{
		Set01(0.0, 0.0, 1.0, 1.0);
	}
	else if (views == 2)
	{
		if (horizontal)
		{	if (i == 0)	Set01(0.0, 0.0, 1.0, 0.5);
			else		Set01(0.0, 0.5, 1.0, 0.5);
		}else
		{   if (i == 0) Set01(0.0, 0.0, 0.5, 1.0);
			else		Set01(0.5, 0.0, 0.5, 1.0);
	}   }
	else if (views == 3)
	{
		if (horizontal)
		{
			if (i == 0)			Set01(0.0, 0.0, 0.5, 0.5);
			else if (i == 1)	Set01(0.5, 0.0, 0.5, 0.5);
			else if (i == 2)	Set01(0.0, 0.5, 1.0, 0.5);
		}else{
			if (i == 0)			Set01(0.0, 0.0, 0.5, 1.0);
			else if (i == 1)	Set01(0.5, 0.0, 0.5, 0.5);
			else if (i == 2)	Set01(0.5, 0.5, 0.5, 0.5);
		}
	}
	else if (views == 4)
	{
		if (i == 0)			Set01(0.0, 0.0, 0.5, 0.5);
		else if (i == 1)	Set01(0.5, 0.0, 0.5, 0.5);
		else if (i == 2)	Set01(0.0, 0.5, 0.5, 0.5);
		else if (i == 3)	Set01(0.5, 0.5, 0.5, 0.5);
	}
	Upd1();    
}
