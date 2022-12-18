#include "pch.h"
#include "dbl.h"
#include "Def_Str.h"
#include "pathmanager.h"
#include "mathvector.h"
#include "game.h"
#include "CGame.h"
#include "CScene.h"
#include "SceneXml.h"
#include "CarModel.h"
#include "FollowCamera.h"

using namespace Ogre;
using namespace std;
#define  FileExists(s)  PATHMANAGER::FileExists(s)


//  🌟 ctor
//------------------------------------------------------------------------------------------------------
CarModel::CarModel(int index, int colorId, eCarType type, const string& name,
	SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s,
	Camera* cam, App* app)

	:mSceneMgr(sceneMgr), pSet(set), pGame(game)
	,sc(s), mCamera(cam), pApp(app)
	,iIndex(index), iColor(colorId % 6), sDirname(name), cType(type), vType(V_Car)
	
	,color(0,1,0)
	,sChkMtr("checkpoint_normal")
{
	SetNumWheels(4);
	int i,w;
	for (w = 0; w < MAX_WHEELS; ++w)
	for (int p=0; p < PAR_ALL; ++p)
		par[p][w] = 0;

	for (i=0; i < PAR_BOOST; ++i)  parBoost[i] = 0;
	for (i=0; i < PAR_THRUST*2; ++i)  parThrust[i] = 0;
	parHit = 0;

	qFixWh[0].Rotate(2*PI_d,0,0,1);
	qFixWh[1].Rotate(  PI_d,0,0,1);

	Defaults();
}

void CarModel::SetNumWheels(int n)
{
	numWheels = n;
	whPos.resize(n);  whRadius.resize(n);  whWidth.resize(n);
	whTrail.resize(n);  whTemp.resize(n);
	ndWh.resize(n);  ndWhE.resize(n);  ndBrake.resize(n);
}

void CarModel::Defaults()
{
	int i,w;
	for (i=0; i < 3; ++i)
	{
		driver_view[i] = 0.f;  hood_view[i] = 0.f;  ground_view[i] = 0.f;
		interiorOffset[i] = 0.f;  boostOffset[i] = 0.f;  exhaustPos[i] = 0.f;
	}
	camDist = 1.f;
	for (i=0; i < PAR_THRUST; ++i)
	{
		for (w=0; w<3; ++w)  thrusterOfs[i][w] = 0.f;
		thrusterSizeZ[i] = 0.f;
		sThrusterPar[i] = "";
	}
	brakePos.clear();
	brakeClr = ColourValue(1,0,0);
	brakeSize = 0.f;

	bRotFix = false;
	sBoostParName = "Boost";  boostSizeZ = 1.f;

	for (w=0; w < numWheels; ++w)
	{
		whRadius[w] = 0.3f;  whWidth[w] = 0.2f;
	}
	manualExhaustPos = false;  has2exhausts = false;

	maxangle = 26.f;
	for (w=0; w < 2; ++w)
		posSph[w] = Vector3::ZERO;

	matStPos = Matrix4::IDENTITY;
	vStDist = Vector4(0,0,0,0);
}


//  📄 Load CAR
//------------------------------------------------------------------------------------------------------
void CarModel::Load(int startId, bool loop)
{
	//  names for local play
	// if (isGhostTrk())    sDispName = TR("#{Track}");
	// else if (isGhost())  sDispName = TR("#{Ghost}");
	// else if (eType == CT_LOCAL)
	// 	sDispName = TR("#{Player}") + toStr(iIndex+1);
	

	///  load config .car
	// string pathCar;
	// pApp->gui->GetCarPath(&pathCar, 0, 0, sDirname, pApp->mClient.get() != 0);  // force orig for newtorked games
	std::string file = sDirname + ".car",
		pathCar  = PATHMANAGER::CarSim()  + "/" + pSet->game.sim_mode + "/cars/" + file;
	LoadConfig(pathCar);

	
	///  Create CAR (dynamic)
	if (!isGhost())  // ghost has pCar, dont create
	{
		if (startId == -1)  startId = iIndex;
		// /*if (pSet->game.start_order == 1)  //;
		{	//  reverse start order
			int numCars = //pApp->mClient ? pApp->mClient->getPeerCount()+1 :  // networked
				pSet->game.local_players;  // splitscreen
			startId = numCars-1 - startId;
		}
		int i = pSet->game.collis_cars ? startId : 0;  // offset when cars collide

		//  start pos
		auto st = pApp->scn->sc->GetStart(i, loop);
		MATHVECTOR<float,3> pos = st.first;
		QUATERNION<float> rot = st.second;
		
		vStartPos = Vector3(pos[0], pos[2], -pos[1]);
		if (pSet->game.trackreverse)
		{	rot.Rotate(PI_d, 0,0,1);  rot[0] = -rot[0];  rot[1] = -rot[1];  }

		pCar = pGame->LoadCar(pathCar, sDirname, pos, rot, true, cType == CT_REMOTE, iIndex);

		if (!pCar)  LogO("Error: Creating CAR: " + sDirname + "  path: " + pathCar);
		else  pCar->pCarM = this;
	}
}


//------------------------------------------------------------------------------------------------------
///  📄 Load .car
//------------------------------------------------------------------------------------------------------
static void ConvertV2to1(float & x, float & y, float & z)
{
	float tx = x, ty = y, tz = z;
	x = ty;  y = -tx;  z = tz;
}
void CarModel::LoadConfig(const string & pathCar)
{
	Defaults();

	///  load  -----
	CONFIGFILE cf;
	if (!cf.Load(pathCar))
	{  LogO("Error: CarModel Can't load .car: "+pathCar);  return;  }


	//  vehicle type
	vType = V_Car;
	string drive;
	cf.GetParam("drive", drive);

	if (drive == "hover")  //>
		vType = V_Spaceship;
	else if (drive == "sphere")
		vType = V_Sphere;


	//  wheel count
	int nw = 0;
	cf.GetParam("wheels", nw);
	if (nw >= 2 && nw <= MAX_WHEELS)
		SetNumWheels(nw);


	//-  custom interior model offset
	cf.GetParam("model_ofs.interior-x", interiorOffset[0]);
	cf.GetParam("model_ofs.interior-y", interiorOffset[1]);
	cf.GetParam("model_ofs.interior-z", interiorOffset[2]);
	cf.GetParam("model_ofs.rot_fix", bRotFix);

	//~  boost offset
	cf.GetParam("model_ofs.boost-x", boostOffset[0]);
	cf.GetParam("model_ofs.boost-y", boostOffset[1]);
	cf.GetParam("model_ofs.boost-z", boostOffset[2]);
	cf.GetParam("model_ofs.boost-size-z", boostSizeZ);
	cf.GetParam("model_ofs.boost-name", sBoostParName);
	
	//  thruster  spaceship hover  max 4 pairs
	int i;
	for (i=0; i < PAR_THRUST; ++i)
	{
		string s = "model_ofs.thrust";
		if (i > 0)  s += toStr(i);
		cf.GetParam(s+"-x", thrusterOfs[i][0]);
		cf.GetParam(s+"-y", thrusterOfs[i][1]);
		cf.GetParam(s+"-z", thrusterOfs[i][2]);
		cf.GetParam(s+"-size-z", thrusterSizeZ[i]);
		cf.GetParam(s+"-name", sThrusterPar[i]);
	}
	

	//~  brake flares
	float pos[3];  bool ok=true;  i=0;
	while (ok)
	{	ok = cf.GetParam("flares.brake-pos"+toStr(i), pos);  ++i;
		if (ok)  brakePos.push_back(bRotFix ? Vector3(-pos[0],pos[2],pos[1]) : Vector3(-pos[1],-pos[2],pos[0]));
	}
	cf.GetParam("flares.brake-color", pos);
	brakeClr = ColourValue(pos[0],pos[1],pos[2]);
	cf.GetParam("flares.brake-size", brakeSize);
	
	
	//-  custom exhaust pos for boost particles
	if (cf.GetParam("model_ofs.exhaust-x", exhaustPos[0]))
	{
		manualExhaustPos = true;
		cf.GetParam("model_ofs.exhaust-y", exhaustPos[1]);
		cf.GetParam("model_ofs.exhaust-z", exhaustPos[2]);
	}else
		manualExhaustPos = false;
	if (!cf.GetParam("model_ofs.exhaust-mirror-second", has2exhausts))
		has2exhausts = false;


	//- load cameras pos
	cf.GetParamE("driver.view-position", pos);
	driver_view[0]=pos[1]; driver_view[1]=-pos[0]; driver_view[2]=pos[2];
	
	cf.GetParamE("driver.hood-position", pos);
	hood_view[0]=pos[1]; hood_view[1]=-pos[0]; hood_view[2]=pos[2];

	if (cf.GetParam("driver.ground-position", pos))
	{	ground_view[0]=pos[1]; ground_view[1]=-pos[0]; ground_view[2]=pos[2];  }
	else
	{	ground_view[0]=0.f; ground_view[1]=1.6; ground_view[2]=0.4f;  }

	cf.GetParam("driver.dist", camDist);


	//  tire params
	float val;
	bool both = cf.GetParam("tire-both.radius", val);

	int axles = std::max(2, numWheels/2);
	for (i=0; i < axles; ++i)
	{
		WHEEL_POSITION wl, wr;  string pos;
		CARDYNAMICS::GetWPosStr(i, numWheels, wl, wr, pos);
		if (both)  pos = "both";
		
		float radius;
		cf.GetParamE("tire-"+pos+".radius", radius);
		whRadius[wl] = radius;  whRadius[wr] = radius;
		
		float width = 0.2f;
		cf.GetParam("tire-"+pos+".width-trail", width);
		whWidth[wl] = width;  whWidth[wr] = width;
	}
	
	//  wheel pos
	//  for track's ghost or garage view
	int version = 2;
	cf.GetParam("version", version);
	for (i = 0; i < numWheels; ++i)
	{
		string sPos = sCfgWh[i];
		float pos[3];  MATHVECTOR<float,3> vec;

		cf.GetParamE("wheel-"+sPos+".position", pos);
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		vec.Set(pos[0],pos[1], pos[2]);
		
		whPos[i] = vec;
	}
	//  steer angle
	maxangle = 26.f;
	cf.GetParamE("steering.max-angle", maxangle);
	maxangle *= pGame->GetSteerRange();
}
