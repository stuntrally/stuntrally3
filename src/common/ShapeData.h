#pragma once

//  info  for shape user data (void*)
//------------------------------------------
const static int  // & 0xFF !
	SU_Road			= 0x100, //+mtrId
	SU_Pipe			= 0x200, //+mtrId
	SU_RoadWall		= 0x300,
	//SU_RoadColumn	= 0x400,  //=Wall
	SU_Terrain		= 0x500,
	SU_Vegetation	= 0x600,  // trees, rocks etc
	SU_Border		= 0x700,  // world border planes
	SU_ObjectStatic	= 0x800,
	SU_Fluid 		= 0x900,  //+surfId  solid fluids, ice etc
	//SU_ObjectDynamic= 0xA00;  //..
	SU_Collect		= 0xA00;  //+collect Id  collectible gems


//  info  for special collision objects:
//  car body, fluid areas, wheel fluid triggers, collectible item triggers
//-------------------------------------------------------------
enum EShapeType
{
	ST_Car=0, ST_Fluid, ST_Wheel, ST_Collect, ST_Other
};

class CARDYNAMICS;  class FluidBox;  class SCollect;
class ShapeData
{
public:
	EShapeType type;
	CARDYNAMICS* pCarDyn;
	FluidBox* pFluid;
	int whNum;
	SCollect* pCol;

	ShapeData( EShapeType type1, CARDYNAMICS* pCarDyn1=0,
		FluidBox* pFluid1=0, int whNum1=0, SCollect* pCol1=0)
		: type(type1), pCarDyn(pCarDyn1)
		, pFluid(pFluid1), whNum(whNum1), pCol(pCol1)
	{	}
};
