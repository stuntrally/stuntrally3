#include "pch.h"
#include "par.h"

SParams::SParams()
{

/// 🧰 _Tool_  force go back time rewind
	backTime = 0;  //0 !in release

/// 🧰 _Tool_  force camera preview
	carPrv = 0;  //0 !in release  1 norm, 2 far, 3 close

//  GAME
	rewindSpeed = 5.f;  // 5 secs in 1 sec
	rewindCooldown = 1.f;  // 1 sec

/// <><> Damage factors <><>
	dmgFromHit  = 0.5f;  dmgFromScrap  = 1.0f;   // reduced
	dmgFromHit2 = 1.5f;  dmgFromScrap2 = 11.5f;  // normal
	dmgPow2 = 1.4f;

//  start pos next car distance
	startNextDist = 6.f;

//  HUD
//  time in sec to show wrong check warning
	timeShowChkWarn = 2.f;
	timeWonMsg = 4.f;
	fadeLapResults = 0.1f;  //0.04f;

//  chk beam size
	chkBeamSx = 5.f;  chkBeamSy = 44.f,  chkBeamYofs = -10.f,
//  ghost
	ghostHideTime = 0.2f;

//  camera bounce, force factors
	camBncF  = -0.0016f;  camBncFo = -0.0016f;  camBncFof = -0.0016f;
	camBncFHit = 4.5f;  camBncFHitY = 1.0f;
	camBncSpring = -1500.f;  camBncDamp = -150.f;  camBncMass = 0.02f;
	camBncScale = 10.f;  camBncLess = 3.f;  // less bnc for hovercars and drones
}

//  for many other params search in sources for: //par or // par

SParams gPar;  // almost const
