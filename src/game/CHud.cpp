#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "CGame.h"
#include "CHud.h"
// #include "CGui.h"
#include "SceneXml.h"
#include "CScene.h"
#include "settings.h"

#ifndef BT_NO_PROFILE
#include <LinearMath/btQuickprof.h>
#endif // BT_NO_PROFILE
#include <OgreManualObject2.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreWindow.h>
#include <OgreCamera.h>
#include <OgreOverlayElement.h>
#include <OgreTechnique.h>
// #include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace MyGUI;


//  ctor
CHud::CHud(App* ap1)
	:app(ap1)
{
	hud.resize(MAX_Players);
	
	pSet = ap1->pSet;
	// gui = ap1->gui;
}


//  HUD utils
//---------------------------------------------------------------------------------------------------------------

//  vel clr speed 🌈
MyGUI::Colour CHud::GetVelClr(float vel)
{
	const int idiv = 50, vmin = -150, vmax = 700;
	const float fdiv = idiv;
	
	int v = vel;
	int id = v > 0 ?  v / idiv :
				-abs(v) / idiv - 1;
	int mod = abs(v) % idiv;
	float f = v > 0 ?  // 0..1
		(v >= vmax ? 1.f :  mod / fdiv ) :
		(v <= vmin ? 0.f :  1.f - (mod / fdiv) );

	const static int velCnt = 17,
		ofs0 = -vmin / idiv;  // ofs for < 0
	const static Colour velClr[velCnt+1] = {
		Colour(0.5f, 0.4f, 1.0f),  //-150 vmin
		Colour(0.4f, 0.3f, 1.0f),  //-100
		Colour(0.2f, 0.3f, 1.0f),  //-50
		Colour(0.1f, 0.4f, 1.0f),  //  0
		Colour(0.3f, 0.6f, 1.0f),  // 50
		Colour(0.4f, 0.9f, 0.9f),  // 100
		Colour(0.2f, 1.0f, 0.5f),  // 150
		Colour(0.5f, 1.0f, 0.1f),  // 200
		Colour(0.9f, 0.9f, 0.1f),  // 250
		Colour(1.0f, 0.7f, 0.0f),  // 300
		Colour(1.0f, 0.5f, 0.0f),  // 350
		Colour(1.0f, 0.2f, 0.4f),  // 400
		Colour(1.0f, 0.3f, 0.7f),  // 450
		Colour(1.0f, 0.3f, 1.0f),  // 500
		Colour(1.0f, 0.5f, 1.0f),  // 550
		Colour(0.7f, 0.5f, 1.0f),  // 600
		Colour(0.5f, 0.5f, 1.0f),  // 650
		Colour(0.8f, 0.8f, 1.0f)}; // 700
	
	const int i1 = std::max(0, std::min(velCnt-1, id + ofs0)),
		i2 = i1 + 1;
	const Colour& c1 = velClr[i1], &c2 = velClr[i2];
	Colour c(  // linear interpolate
		c1.red   + f * (c2.red   - c1.red),
		c1.green + f * (c2.green - c1.green),
		c1.blue  + f * (c2.blue  - c1.blue), 1.f);
	return c;
}

void CHud::UpdMiniTer()
{
	/*MaterialPtr mm = MaterialManager::getSingleton().getByName("circle_minimap");
	Pass* pass = mm->getTechnique(0)->getPass(0);
	if (!pass)  return;
	try
	{	GpuProgramParametersSharedPtr par = pass->getFragmentProgramParameters();
		if (par->_findNamedConstantDefinition("showTerrain",false))
			par->setNamedConstant("showTerrain", pSet->mini_terrain ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("showBorder",false))
			par->setNamedConstant("showBorder", pSet->mini_border ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("square",false))
			par->setNamedConstant("square", pSet->mini_zoomed ? 0.f : 1.f);
	}
	catch(...){  }*/
}


Vector3 CHud::projectPoint(const Camera* cam, const Vector3& pos)
{
	Vector3 pos2D = cam->getProjectionMatrix() * (cam->getViewMatrix() * pos);

	//std::min(1.f, std::max(0.f,  ));  // leave on screen edges
	Real x =  pos2D.x * 0.5f + 0.5f;
	Real y = -pos2D.y * 0.5f + 0.5f;
	bool out = !cam->isVisible(pos);

	return Vector3(x * app->mWindow->getWidth(), y * app->mWindow->getHeight(), out ? -1.f : 1.f);
}

using namespace MyGUI;
TextBox* CHud::CreateNickText(int carId, String text)
{
	TextBox* txt = app->mGui->createWidget<TextBox>("TextBox",
		100,100, 360,32, Align::Center, "Back", "NickTxt"+toStr(carId));
	txt->setVisible(false);
	txt->setFontHeight(28);  //par 24..32
	txt->setTextShadow(true);  txt->setTextShadowColour(Colour::Black);
	txt->setCaption(text);
	return txt;
}

//  get color as text eg. #C0E0FF
String CHud::StrClr(ColourValue c)
{
	char hex[16];
	sprintf(hex, "#%02x%02x%02x", int(c.r * 255.f), int(c.g * 255.f), int(c.b * 255.f));
	return String(hex);
}


#ifndef BT_NO_PROFILE
///  Bullet profiling text
//--------------------------------------------------------------------------------------------------------------
void CHud::bltDumpRecursive(CProfileIterator* pit, int spacing, std::stringstream& os)
{
	pit->First();
	if (pit->Is_Done())
		return;

	float accumulated_time=0,parent_time = pit->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : pit->Get_Current_Parent_Total_Time();
	int i,j;
	int frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();
	for (i=0; i<spacing; ++i)  os << ".";
	os << "----------------------------------\n";
	for (i=0; i<spacing; ++i)  os << ".";
	std::string s = "Profiling: "+String(pit->Get_Current_Parent_Name())+" (total running time: "+fToStr(parent_time,3)+" ms) ---\n";
	os << s;
	//float totalTime = 0.f;

	int numChildren = 0;
	
	for (i = 0; !pit->Is_Done(); ++i,pit->Next())
	{
		++numChildren;
		float current_total_time = pit->Get_Current_Total_Time();
		accumulated_time += current_total_time;
		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;

		for (j=0;j<spacing;j++)	os << ".";
		double ms = (current_total_time / (double)frames_since_reset);
		s = toStr(i)+" -- "+pit->Get_Current_Name()+" ("+fToStr(fraction,2)+" %) :: "+fToStr(ms,3)+" ms / frame ("+toStr(pit->Get_Current_Total_Calls())+" calls)\n";
		os << s;
		//totalTime += current_total_time;
		//recurse into children
	}

	if (parent_time < accumulated_time)
	{
		os << "what's wrong\n";
	}
	for (i=0; i<spacing; ++i)  os << ".";
	double unaccounted=  parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f;
	s = "Unaccounted: ("+fToStr(unaccounted,3)+" %) :: "+fToStr(parent_time - accumulated_time,3)+" ms\n";
	os << s;
	
	for (i=0; i<numChildren; ++i)
	{
		pit->Enter_Child(i);
		bltDumpRecursive(pit, spacing+3, os);
		pit->Enter_Parent();
	}
}

void CHud::bltDumpAll(std::stringstream& os)
{
	CProfileIterator* profileIterator = 0;
	profileIterator = CProfileManager::Get_Iterator();

	bltDumpRecursive(profileIterator, 0, os);

	CProfileManager::Release_Iterator(profileIterator);
}
#endif // BT_NO_PROFILE
