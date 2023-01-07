#pragma once

//  viewport dimensions, for each player
struct ViewDim
{
    float top0,left0, width0,height0, right0,bottom0, avgsize0;  //  0..1
    float top1,left1, width1,height1, right1,bottom1, avgsize1;  // -1..1
    void Default();
    
    void Set01(float left, float top, float width, float height);
    void Upd1();
    void SetDim(int views, bool horizontal, int i/*player view*/);
    
    ViewDim()
    {	Default();  }
};
