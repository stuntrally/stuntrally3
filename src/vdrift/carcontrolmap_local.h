#pragma once
#include "cardefs.h"
#include <vector>


enum PlayerActions
{	A_Throttle, A_Brake, A_Steering, A_HandBrake,
	A_Boost, A_Flip,
	A_ShiftUp, A_ShiftDown, A_PrevCamera, A_NextCamera,
	A_LastChk, A_Rewind, A_Lights,
	NumPlayerActions
};


class CARCONTROLMAP_LOCAL
{
private:
	std::vector <float> inputs;  // indexed by CARINPUT values
	
	// shift
	bool grUpOld[8], grDnOld[8];

public:
	CARCONTROLMAP_LOCAL()
	{
		for (int i=0; i < 8; ++i)  //par
		{	grUpOld[i] = false;  grDnOld[i] = false;  }
		Reset();
	}
	
	void Reset()
	{
		inputs.resize(CARINPUT::ALL, 0.f);
	}
	
	const std::vector <float> & ProcessInput(
		const float* channels, int player,
		float carspeed,
		float sss_range, float sss_effect, float sss_velfactor,
		bool oneAxisThrBrk = false,  bool forceBrake = false,  // for race countdown
		//  auto performance test
		bool bPerfTest = false, EPerfTest iPerfTestStage = PT_StartWait);
		
	//*  speed sensitive steering sss  (decrease steer angle range with higher speed)
	static float GetSSScoeff(
		float speed, float steer_range, float sss_velfactor, float sss_effect);
};
