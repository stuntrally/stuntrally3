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
	avgsize1= (width1 + height1) * 0.5f;
}


void ViewDim::SetDim(int views, bool horizontal, int i/*player view*/)
{
	const auto t = 1.0/3.0;
	switch (views)
	{
	case 1:
		Set01(0.0, 0.0, 1.0, 1.0);  // [  ]
		break;
	case 2:
		if (horizontal)                             // [     ]
		{	if (i == 0)	Set01(0.0, 0.0, 1.0, 0.5);  // [-----]
			else		Set01(0.0, 0.5, 1.0, 0.5);  // [     ]
		}else
		{   if (i == 0) Set01(0.0, 0.0, 0.5, 1.0);  // [  |  ]
			else		Set01(0.5, 0.0, 0.5, 1.0);  // [  |  ]
		}	break;                                  // [  |  ]
	case 3:
		if (horizontal)
		{	if (i == 0)       Set01(0.0, 0.0, 0.5, 0.5);  // [   |   ]
			else if (i == 1)  Set01(0.5, 0.0, 0.5, 0.5);  // [-------]
			else if (i == 2)  Set01(0.0, 0.5, 1.0, 0.5);  // [       ]
		}else
		{	if (i == 0)       Set01(0.0, 0.0, 0.5, 1.0);  // [   |   ]
			else if (i == 1)  Set01(0.5, 0.0, 0.5, 0.5);  // [   |---]
			else if (i == 2)  Set01(0.5, 0.5, 0.5, 0.5);  // [   |   ]
		}	break;
	case 4:                                         // [   |   ]
		Set01((i%2) * 0.5, (i/2) * 0.5, 0.5, 0.5);  // [---+---]
		break;                                      // [   |   ]
	case 5:
		if (horizontal)
		{	if (i < 3)        Set01(i * t, 0.0, t, 0.5);  // [  |   |  ]
			else if (i == 3)  Set01(0.0, 0.5, 0.5, 0.5);  // [---------]
			else if (i == 4)  Set01(0.5, 0.5, 0.5, 0.5);  // [    |    ]
		}else{
			if (i < 3)        Set01(0.0, i*t, 0.5, t);    // [--|  ]
			else if (i == 3)  Set01(0.5, 0.0, 0.5, 0.5);  // [--|--]
			else if (i == 4)  Set01(0.5, 0.5, 0.5, 0.5);  // [--|  ]
		}	break;
	case 6:                                      // [  |  |  ]
		Set01((i%3) * t, (i/3) * 0.5, t, 0.5);   // [--+--+--]
		break;                                   // [  |  |  ]
	}
	Upd1();    
}
