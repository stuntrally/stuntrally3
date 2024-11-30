#pragma once

enum ED_MODE
{
	ED_Deform=0, ED_Smooth, ED_Height, ED_Filter, //ED_Paint,
	ED_Road, ED_Start, ED_PrvCam,
	ED_Fluids, ED_Objects, ED_Particles,
	ED_Collects, ED_Fields,
	ED_ALL,  ED_TerEditMax = ED_Filter+1
};

enum WND_Types  // pSet->inMenu
{
	WND_Track=0,
	WND_Edit, WND_EdObj,
	WND_Help, WND_Options,
	WND_Materials,
	WND_ALL,
	ciMainBtns = WND_Materials  // last
};

enum TABS_Track
{
	TabTrk_Back=0,
	TabTrk_Track,
	TabTrk_Tools,
	TabTrk_Game,
	TabTrk_Warnings,
	TabTrk_Export,
	TabsTrk_ALL
};

enum TABS_Edit
{
	TAB_Back=0, TAB_Sun,
	TAB_Terrain, TAB_Layers,
	TAB_Grass, TAB_Veget,
	TAB_Road, TAB_Surface,
	TabsEdit_ALL
};

enum TABS_EdObj
{
	TABo_Objects=0,
	TABo_Fluids, TABo_Particles,
	TabsEdObj_ALL
};
