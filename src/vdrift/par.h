#pragma once

//  for making vehicle preview screens, see info in cameras.xml
//#define CAR_PRV

#define MAX_Players  6
#define MAX_Vehicles 8  // = players and 2 ghosts

///  const game params  -------
struct SParams
{
	float
//  ⏪ GAME
	 rewindSpeed
	,rewindCooldown  // time in sec

//  🔨 damage factors
	,dmgFromHit, dmgFromScrap    // reduced
	,dmgFromHit2, dmgFromScrap2  // normal
	,dmgPow2

//  🏁 start pos, next car distance
	,startNextDist

//  HUD
//  ⏱️ time in sec
	,timeShowChkWarn
	,timeWonMsg
	,fadeLapResults

//  📍 chk beam size
	,chkBeamSx, chkBeamSy, chkBeamYofs
//  👻 ghost
	,ghostHideTime

//  🎥 camera bounce sim
	,camBncF, camBncFo, camBncFof
	,camBncFHit,camBncFHitY
	,camBncSpring, camBncDamp
	,camBncScale, camBncMass;

//  take back time in rewind (for track's ghosts)
	bool backTime;

	//  🌟 ctor, init values
	SParams();
};

const static SParams gPar;
