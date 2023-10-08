#pragma once
#include <OgrePrerequisites.h>

#define rgDef  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME


//  Render Queue Groups   used in setRenderQueueGroup etc
//------------------------------------------------------------------------
/*	Default, movables queue ID:
	Decal            0
	Light            0-5
	Item             10
	Rectangle2D      10
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
	RQG_Horizon1     = 12,  // horiz after terrain to skip most depth
	RQG_Horizon2     = 13,
	
	RQG_Road         = RQ_7,
	RQG_RoadBlend    = RQ_7 +1,

	RQG_BatchAlpha   = RQ_7 +4,  // veget transparent
	RQG_Fluid        = RQ_7 +5,  // water etc
	RQG_RoadMarkers  = RQ_7 +6,  // ed

	RQG_PipeGlass    = RQ_GlassV2 +2,   // v2 glass pipe road`
	RQG_CarGlass     = RQ_GlassV2 +4,   // v2
	RQG_Ghost        = RQ_GlassV2 +7,   // markers, debug

	RQG_CarTrails    = RQ_GlassV1,      // v1 trails after glass
	RQG_CarParticles = RQ_GlassV1 +3,   // v1 particles after trails

	RQG_Weather      = RQ_GlassV1 +5,   // v1 particles rain etc

	RQG_Hud1         = RQ_OVERLAY +1,
	RQG_Hud2         = RQ_OVERLAY +2,
	RQG_Hud3         = RQ_OVERLAY +3;


//  Visibility Flags  used in setVisibility
//------------------------------------------------------------------------
const Ogre::uint32
	RV_Hud          = 0x00002000,  // ⏱️⏲️ hud  and markers,  in game has own pass
	RV_Hud3D        = 0x00001000,  // 🎗️   trail

	RV_Road         = 0x00000001,  // 🛣️   road only, for road textures
	RV_Terrain      = 0x00000004,  // ⛰️   terrain  and  fluids 🌊  for ed terrain.jpg texture
	RV_Vegetation   = 0x00000008,  // 🌳🪨 vegetation
	RV_VegetGrass   = 0x00000010,  // 🌿   grass
	RV_Objects      = 0x00000020,  // 📦🏢 all objects  (static meshes, buildings and dynamic props)

	RV_SkyMain      = 0x00000100,  // ⛅   sky for main view
	RV_SkyPlanarRefl= 0x00000200,  // ⛅🪞 sky for fluids refl 🌊
	RV_SkyCubeRefl  = 0x00000400,  // ⛅🔮 sky for dynamic refl car,metals,etc
  
	RV_Car          = 0x00000080,  // 🚗⚫ car,interior, tires  in game  (hide in reflection render)
	RV_CarGlass     = 0x00000040,  // 🚗🪟 car glass in game  (hide for in car camera)
	RV_Particles    = RV_Car,

	//  vis masks
	RV_MaskGameAll  = RV_SkyMain + RV_Road + RV_Terrain + RV_Vegetation + RV_VegetGrass + RV_Objects 
					+ RV_Car + RV_CarGlass + RV_Hud3D,
	RV_MaskReflect  = RV_SkyCubeRefl + RV_Road + RV_Terrain + RV_Vegetation + RV_Objects,  // no hud, grass, car,glass,tires
	//  most as  visibility_mask  in SR3.compositor
	// RV_WaterReflect = RV_Terrain + RV_Vegetation + RV_Road /*+ RV_Objects /*+ RV_Car*/,
	// RV_WaterRefract = RV_Terrain + RV_Vegetation + RV_Road + RV_Objects + RV_Car,

	// RV_MaskAll      = 0x000001FF,
	// RV_MaskPrvCam   = 0x00000200,
	//  ed only  road RTT prv
	RV_EdRoadDens    = 0x00004000,  // 🛣️🌿 for grass,trees placing etc
	RV_EdRoadPreview = 0x00008000;  // 🛣️🌍 for minimap
