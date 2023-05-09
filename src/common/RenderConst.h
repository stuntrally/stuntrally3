#pragma once
#include <OgrePrerequisites.h>

#define rgDef  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME


//  Render Queue Groups used  //search for setRenderQueueGroup
//------------------------------------------------------------------------
/*	Default, movables queue ID:
	Decal            0
	Light            0-5
	Item             10
	Rectangle2D      10
	v1::Entity       110
	v1::Rectangle2D  110
	ParticleSystem   110
	..
	Default, queues mode:
	0-99     FAST
	100-199  V1_FAST
	200-224  FAST
	225-255  V1_FAST
*/
const Ogre::uint8
	RQ_7        = 70,
	RQ_GlassV2  = 80,
	RQ_GlassV1  = 105,
	RQ_OVERLAY  = 200,

	RQG_Sky          = 5,
	RQG_Terrain      = 11,
	
	//RQG_BatchOpaque  = 50,
	RQG_Road         = RQ_7,

	RQG_RoadBlend    = RQ_7 +1,
	RQG_BatchAlpha   = RQ_7 +4,  // veget transparent
	RQG_Fluid        = RQ_7 +5,
	RQG_RoadMarkers  = RQ_7 +6,  // ed

	RQG_CarGlass     = RQ_GlassV2,  // v2
	RQG_CarTrails    = RQ_GlassV1,      // v1 trails after glass
	RQG_PipeGlass    = RQ_GlassV2 +2,   // v2 glass pipe road`
	RQG_CarParticles = RQ_GlassV1 +3,   // v1 particles after trails

	RQG_Weather      = RQ_GlassV1 +5,   // v1 particles rain etc
	RQG_Ghost        = RQ_GlassV2 +7,   // markers, debug

	RQG_Hud1         = RQ_OVERLAY +1,
	RQG_Hud2         = RQ_OVERLAY +2,
	RQG_Hud3         = RQ_OVERLAY +3;


//  Visibility Flags used   search for setVisibility
//------------------------------------------------------------------------
const Ogre::uint32
	RV_Road         = 1,   // road only, for road textures
	RV_Hud          = 2,   // hud  and markers
	RV_Terrain      = 4,   // terrain  and  fluids  for ed terrain.jpg texture
	RV_Vegetation   = 8,   // vegetation
	RV_VegetGrass   = 64,  // grass
	RV_Objects      = 256, // all objects  (static meshes, buildings and dynamic props)
	RV_Sky          = 32,  // sky, editor only
  
	RV_Car          = 128, // car,interior, tires  in game, (hide in reflection render)
	RV_Particles    = RV_Car,
	RV_CarGlass     = 16,  // car glass in game, (hide for in car camera)

	//  vis masks
	RV_MaskReflect  = RV_Sky + RV_Road + RV_Terrain + RV_Vegetation + RV_Objects,  // hide 2: hud, car,glass,tires
	//  most as  visibility_mask  in SR3.compositor
	RV_WaterReflect = RV_Terrain + RV_Vegetation + RV_Road /*+ RV_Objects /*+ RV_Car*/,
	RV_WaterRefract = RV_Terrain + RV_Vegetation + RV_Road + RV_Objects + RV_Car,

	RV_MaskAll      = 511,
	RV_MaskPrvCam   = 512,
	
	RV_EdRoadDens    = 1024,
	RV_EdRoadPreview = 2048;
