#pragma once
#include <OgreRenderQueue.h>

#define rgDef  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME


//  Render Queue Groups used  //search for setRenderQueueGroup
//------------------------------------------------------------------------
/*	By default, movables will be assigned the following render queue ID:\n
	Decal:              0 \n
	Light:              0-5 (depends on light type, cannot be changed) \n
	Item:               10 \n
	Rectangle2D:        10 \n
	v1::Entity:         110 \n
	v1::Rectangle2D:    110 \n
	ParticleSystem:     110 \n
	[.. more unlisted ..]
@remarks
	By default, the render queues have the following mode set: \n
	0-99:    FAST \n
	100-199: V1_FAST \n
	200-224: FAST \n
	225-255: V1_FAST */
	
const Ogre::uint8
	RENDER_QUEUE_MAIN = 50,
	RENDER_QUEUE_7 = 70,
	RENDER_QUEUE_8 = 80,
	RENDER_QUEUE_OVERLAY = 100,
	RQG_Sky = 5, //RENDER_QUEUE_SKIES_EARLY,	// 5
	
	RQG_BatchOpaque  = RENDER_QUEUE_MAIN,	// 50  paged geom
	RQG_Road         = RENDER_QUEUE_7,

	RQG_RoadBlend    = RENDER_QUEUE_7+1,
	RQG_BatchAlpha   = RENDER_QUEUE_7+4,  // veget transparent
	RQG_Fluid        = RENDER_QUEUE_7+5,
	RQG_RoadMarkers  = RENDER_QUEUE_7+6,

	RQG_CarGlass     = RENDER_QUEUE_7+7,
	RQG_CarTrails    = RENDER_QUEUE_8,    // trails after glass
	RQG_PipeGlass    = RENDER_QUEUE_8+2,  // glass pipe road`
	RQG_CarParticles = RENDER_QUEUE_8+3,  // particles after trails

	RQG_Weather      = RENDER_QUEUE_8+5,
	RQG_CarGhost     = RENDER_QUEUE_8+7,

	RQG_Hud1         = RENDER_QUEUE_OVERLAY-5,	// 95
	RQG_Hud2         = RENDER_QUEUE_OVERLAY-2,
	RQG_Hud3         = RENDER_QUEUE_OVERLAY-1;	// 99


//  Visibility Flags used  //search for setVisibility
//------------------------------------------------------------------------
const Ogre::uint32
	RV_Road = 1,	// road only, for road textures
	RV_Hud = 2,		// hud and markers
	RV_Terrain = 4,	// terrain and fluids, for terrain texture
	RV_Vegetation = 8,  // vegetation, paged geom
	RV_VegetGrass = 64, // grass, paged geom
	RV_Objects = 256,  // all objects (static meshes and props)
	RV_Sky = 32,	// sky, editor only

	RV_Car = 128,		// car,tires in game, (hide in reflection render)
	RV_Particles = RV_Car,
	RV_CarGlass = 16,	// car glass in game, (hide for in car camera)
	RV_MaskReflect = RV_Sky + RV_Road + RV_Terrain + RV_Vegetation + RV_Objects,  // hide 2: hud, car,glass,tires
	
	RV_WaterReflect = RV_Terrain + RV_Vegetation + RV_Road /*+ RV_Objects /*+ RV_Car*/,
	RV_WaterRefract = RV_Terrain + RV_Vegetation + RV_Road + RV_Objects + RV_Car,

	RV_MaskAll = 511,
	RV_MaskPrvCam = 512;
