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
	SU_Collect		= 0xA00,  //+collect Id  collectible gems
	SU_Field		= 0xD00;  //+field Id  teleporters,accel,antigrav,etc


//  info  for special collision objects:
//-------------------------------------------------------------
enum EShapeType
{
	ST_Car=0,  //  car body
	ST_Fluid, ST_Wheel,  //  fluid area, wheel fluid trigger
	ST_Collect,  //  💎 collectible item trigger
	ST_Field,   //  🎆 field, one of FieldTypes
	ST_Damp,   //  🌿 veget bush, slowing
	ST_Other
};

class CARDYNAMICS;
class FluidBox;  class SCollect;  class SField;

class ShapeData
{
public:
	EShapeType type;
	CARDYNAMICS* pCarDyn;
	FluidBox* pFluid;  // 🌊
	int whNum;

	SCollect* pCol;  // 💎
	SField* pField;  // 🎆
	float fDamp;  // 🌿 veget bush

	ShapeData( EShapeType type1, CARDYNAMICS* pCarDyn1 =0,
		FluidBox* pFluid1 =0, int whNum1 =0,
		SCollect* pCol1 =0,
		SField* pField1 =0, float fDamp1 =0.f)
		
		: type(type1), pCarDyn(pCarDyn1)
		, pFluid(pFluid1), whNum(whNum1)
		, pCol(pCol1)
		, pField(pField1), fDamp(fDamp1)
	{	}
};
