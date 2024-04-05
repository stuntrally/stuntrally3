#pragma once

//  .car types  --------
enum VehicleType
{
	V_Car,  // 2..8 wheeled
	V_Spaceship,  // hovering
	V_Sphere,  // rolling, bouncy-

	V_Drone,  // hovering sphere sci-fi
	V_Hovercar,  // hovering car sci-fi
	V_Hovercraft  // racing hovercraft
};


namespace SURFACE  {
enum CARSURFACETYPE
{
	NONE, ASPHALT, GRASS, GRAVEL, CONCRETE, SAND, COBBLES
};  }


//  inputs  --------
namespace CARINPUT  {
enum CARINPUT
{
	//actual car inputs that the car uses
	THROTTLE, 	BRAKE,  BOOST, FLIP,
  	HANDBRAKE, 	CLUTCH, //-
  	STEER_LEFT,	STEER_RIGHT,
 	SHIFT_UP, 	SHIFT_DOWN,
 	PREV_CAM, NEXT_CAM,
 	LAST_CHK, REWIND,
 	//ABS_TOGGLE, TCS_TOGGLE, START_ENGINE,
	LIGHTS,
	ALL
};  }


//  wheels  ----
enum WHEEL_POSITION
{
	FRONT_LEFT,	FRONT_RIGHT,
	REAR_LEFT,	REAR_RIGHT,
	REAR2_LEFT, REAR2_RIGHT,
	REAR3_LEFT, REAR3_RIGHT
};
const int MAX_WHEELS = 8;
const int MAX_CARS = 16;
const int CarPosCnt = 8;  // size of poses queue

const static char sCfgWh[MAX_WHEELS][4] = {
	"FL","FR","RL","RR",  // .car config wheel names
	"RL2","RR2","RL3","RR3"};  // for 6, 8


//  other  ----
const int PAR_BOOST = 2, PAR_THRUST = 4;  // max particles for boost and spc thrusters

enum EPerfTest {PT_StartWait, PT_Accel, PT_Brake};

const int Ncrashsounds = 12, Nwatersounds = 3;
