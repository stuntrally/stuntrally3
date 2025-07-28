#pragma once

#define MAX_Players  6
#define MAX_Vehicles 8  // = players and 2 ghosts
#define MAX_ArrCol   3  // hud arrows to collectibles gems
#define NumWeather   2

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
	,camBncSpring, camBncDamp, camBncMass
	,camBncScale, camBncLess;

//  take back time in rewind (for track's ghosts)
	bool backTime;

//  force car preview  camera & inputs  info in cameras.xml
	int carPrv;

	//  🌟 ctor, init values
	SParams();
};

extern SParams gPar;
