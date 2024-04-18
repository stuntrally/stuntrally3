#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "paths.h"
#include "App.h"

#include "CGui.h"
#include "settings.h"

#include <OgreWindow.h>
#include <OgreException.h>
#include <OgreString.h>
#include <OgreHlmsUnlit.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_Gui.h>
#include <MyGUI_Window.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_FactoryManager.h>
#include <MyGUI_ResourceTrueTypeFont.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


CGuiCom::CGuiCom(App* app1)
	:app(app1)
{
	pSet = app1->pSet;
	sc = app1->scn->sc;
	//mGui = app1->mGui;  set in GuiInit

	pathTrk[0] = PATHS::Tracks() + "/";
	pathTrk[1] = PATHS::TracksUser() + "/";
	
	int t,i;
	for (t=0; t < 3; ++t)
	{	
		imgPrv[t] = 0;  imgMini[t] = 0;  imgTer[t] = 0;
		imgMiniPos[t] = 0;  imgMiniRot[t] = 0;
	}
	for (t=0; t < 2; ++t)
	{
		for (i=0; i < StTrk; ++i)     stTrk[t][i] = 0;
		for (i=0; i < ImStTrk; ++i) imStTrk[t][i] = 0;
		for (i=0; i < InfTrk; ++i){  infTrk[t][i] = 0;  imInfTrk[t][i] = 0;  }
	}
	
	//  🏝️ short scenery names  for user tracks
	//  and presets.xml  colors from  sc=".."
	scnN["Jng"] = "Jungle";        scnN["JngD"]= "JungleDark";      scnN["Mos"] = "Moss";          
	scnN["For"] = "Forest";        scnN["ForM"]= "ForestMntn";      scnN["ForY"]= "ForestYellow";
	scnN["Fin"] = "Finland";       scnN["Mud"] = "Mud";             scnN["DesM"]= "DesertMud";

	scnN["Sav"] = "Savanna";       scnN["SavD"]= "SavannaDry";      scnN["Stn"] = "Stone";
	scnN["Grc"] = "Greece";        scnN["GrcW"]= "GreeceWhite";     scnN["GrcR"] = "GreeceRocky";  

	scnN["Atm"] = "Autumn";        scnN["AtmD"]= "AutumnDark";
	scnN["Wnt"] = "Winter";        scnN["Wet"] = "WinterWet";

    scnN["Des"] = "Desert";        scnN["Isl"] = "Island";          scnN["IslD"]= "IslandDark";
	scnN["Aus"] = "Australia";     scnN["Can"] = "Canyon";        
	scnN["Vlc"] = "Volcanic";      scnN["VlcD"]= "VolcanicDark";

	scnN["Tox"] = "Toxic";         scnN["Aln"] = "Alien";
	scnN["Uni"] = "Unidentified";  scnN["Mrs"] = "Mars";            scnN["Cry"] = "Crystals";

	scnN["Sur"] = "Surreal";       scnN["Mar"] = "Marble";          scnN["Spr"] = "Spring";
	scnN["Spc"] = "Space";         scnN["SuSp"]= "SurrealSpace";
	scnN["Ano"] = "Anomaly";       scnN["Apo"] = "PostApo";

	scnN["Oth"] = "Other";         scnN["Blk"] = "BlackDesert";     scnN["Asp"] = "Asphalt";

	//  🏝️ scenery 🌈 colors  for track names  *  *  * * * * ** ** ** *** *** ****
	scnClr["Jungle"]       = "#50FF50";  scnClr["JungleDark"]   = "#40C040";  scnClr["Moss"]         = "#70F0B0";
	scnClr["Forest"]       = "#A0C000";  scnClr["ForestMntn"]   = "#A0C080";  scnClr["ForestYellow"] = "#C0C000";
	scnClr["Finland"]      = "#A0E080";  scnClr["Mud"]          = "#A0A000";  scnClr["DesertMud"]    = "#B0B000";

	scnClr["Savanna"]      = "#C0F080";  scnClr["SavannaDry"]   = "#C0D090";  scnClr["Stone"]        = "#A0A0A0";
	scnClr["Greece"]       = "#B0FF00";  scnClr["GreeceWhite"]  = "#C0C0A0";  scnClr["GreeceRocky"]  = "#B0A8A0";

	scnClr["Autumn"]       = "#FFA020";  scnClr["AutumnDark"]   = "#908070";
	scnClr["Winter"]       = "#D0D8D8";  scnClr["WinterWet"]    = "#90D898";

	scnClr["Desert"]       = "#F0F000";  scnClr["Island"]       = "#FFFF80";  scnClr["IslandDark"]   = "#909080";
	scnClr["Australia"]    = "#FF9070";  scnClr["Canyon"]       = "#E0B090";
	scnClr["Volcanic"]     = "#908030";  scnClr["VolcanicDark"] = "#706030";

	scnClr["Toxic"]        = "#60A030";  scnClr["Alien"]        = "#D0FFA0";
	scnClr["Unidentified"] = "#8080D0";  scnClr["Mars"]         = "#A04840";  scnClr["Crystals"]     = "#4090F0";

	scnClr["Surreal"]      = "#F080B0";  scnClr["Marble"]       = "#E0A0C0";  scnClr["Spring"]       = "#A0F000";
	scnClr["Space"]        = "#A0B8D0";  scnClr["SurrealSpace"] = "#80B0FF";
	scnClr["Anomaly"]      = "#FF50D0";  scnClr["PostApo"]      = "#283818";

	scnClr["Other"]        = "#C0D0E0";  scnClr["BlackDesert"]  = "#202020";  scnClr["Asphalt"]      = "#B0E0E0";
}

int TrkL::idSort = 0;

TrkL::TrkL() :
	ti(0)
{	}


//  🆕 Init all
void CGuiCom::GuiInitAll()
{
	GuiInitTooltip();
	GuiInitLang();

	GuiInitGraphics();
	InitGuiScreenRes();
	ImgPrvInit();
}

//  Util
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiCenterMouse()
{	
	// int xm = app->mWindow->getWidth()/2, ym = app->mWindow->getHeight()/2;

	// app->mInputWrapper->warpMouse(xm, ym);
	// InputManager::getInstance().injectMouseMove(xm, ym, 0);
}

void CGuiCom::btnQuit(WP)
{
	app->Quit();
}

//  unfocus lists (would have double up/dn key input)
void CGuiCom::UnfocusLists()
{
	WP w = InputManager::getInstance().getKeyFocusWidget();
	while (w)
	{
		//LogO(w->getTypeName() +" "+ w->getName());

		#ifdef SR_EDITOR
		if (w == (WP)trkList  || w == (WP)app->gui->liSky || w == (WP)app->gui->liTex ||
			w == (WP)app->gui->liGrs || w == (WP)app->gui->liVeg || w == (WP)app->gui->liRd)
		#else
		if (w == (WP)trkList  || (app && app->gui && (
			w == (WP)app->gui->carList  || w == (WP)app->gui->liChalls ||
			w == (WP)app->gui->liChamps || w == (WP)app->gui->liStages || w == (WP)app->gui->rplList)) )
		#endif
		{
			InputManager::getInstance().resetKeyFocusWidget();
			return;
		}
		w = w->getParent();
	}
}

TabPtr CGuiCom::FindSubTab(WP tab)
{
	TabPtr  sub = 0;  // 0 for not found
	size_t s = tab->getChildCount();
	for (size_t n = 0; n < s; ++n)
	{
		WP wp = tab->getChildAt(n);
		if (StringUtil::startsWith(wp->getName(), "SubTab"))
			sub = (TabPtr)wp;
	}
	return sub;
}


//  🖼️ fullscr previews
//-----------------------------------------------------------------------------------
void CGuiCom::ImgPrvClk(WP)
{
	imgPrv[2]->setVisible(true);
}
void CGuiCom::ImgTerClk(WP)
{
	imgTer[2]->setVisible(true);
	imgMini[2]->setVisible(true);
}
void CGuiCom::ImgPrvClose(WP)
{
	imgPrv[2]->setVisible(false);
	imgTer[2]->setVisible(false);
	imgMini[2]->setVisible(false);
}

void CGuiCom::ImgPrvInit()
{
	std::vector<int> ii =
	#ifdef SR_EDITOR
		{0,2};
	#else
		{0,1,2};
	#endif
	for (int i : ii)
	{	string s = toStr(i);
		if (i < 2)
		{	ImgBC(imgPrv[i], "TrackImg" +s, ImgPrvClk);
			ImgBC(imgTer[i], "TrkTerImg"+s, ImgTerClk);
			ImgBC(imgMini[i],"TrackMap" +s, ImgTerClk);
		}else{
			ImgBC(imgPrv[i], "TrackImg" +s, ImgPrvClose);
			ImgBC(imgTer[i], "TrkTerImg"+s, ImgPrvClose);
			ImgBC(imgMini[i],"TrackMap" +s, ImgPrvClose);
		}
		initMiniPos(i);
	}
	for (int i : ii)
	{	imgPrv[i]->setImageTexture("PrvView");
  		imgTer[i]->setImageTexture("PrvTer");
  		imgMini[i]->setImageTexture("PrvRoad");
	}
}

void CGuiCom::initMiniPos(int i)
{
	imgMiniPos[i] = fImg("TrackPos" + toStr(i));
	imgMiniRot[i] = imgMiniPos[i]->getSubWidgetMain()->castType<RotatingSkin>();
	IntSize si = imgMiniPos[i]->getSize();
	imgMiniRot[i]->setCenter(IntPoint(si.width*0.9f, si.height*0.9f));  //0.7
}


//  🗜️ Resize MyGUI
//-----------------------------------------------------------------------------------

void CGuiCom::SizeGUI()
{
	#ifndef SR_EDITOR
	app->baseSizeGui();
	#endif
	
	//  call recursive method for all root widgets
	for (auto it = app->vwGui.begin(); it != app->vwGui.end(); ++it)
		doSizeGUI((*it)->getEnumerator());

#if 0  ///  🧰 _Tool_  test Gui text sizes
	for (auto it = app->vwGui.begin(); it != app->vwGui.end(); ++it)
		doTestGUI((*it)->getEnumerator());
#endif
}

void CGuiCom::doTestGUI(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WP wp = widgets.current();
		string relativeTo = wp->getUserString("RelativeTo");
		// if (relativeTo != "")
		{
			// WP p = wp->getParent();
			// if (p && (
			const auto& tp = wp->getTypeName();
			if (
				// (wp->getLayer() && wp->getLayer()->getName() != "ToolTip") &&  //-
				(
				tp == "TextBox" ||
				tp == "Button" || // && skin checkbox..
				tp == "EditBox"  // && skin empty..
				// wp->getSkin() == ""  //-
			))
			{
				LogO(wp->getTypeName()+" "+wp->getName());
				// if (wp->getLayer())  LogO(wp->getLayer()->getName());  //-
				
				Img img = wp->createWidget<ImageBox>("ImageBox",
					0,0, wp->getSize().width, wp->getSize().height,  // same size
					Align::Default, "ToolTip");
				img->setImageTexture("white.png");
				img->setAlpha(0.3f);  //par
				// getClrDiff(int)
				img->setNeedKeyFocus(0);  img->setNeedMouseFocus(0);
			/*
				Txt txt = wp->createWidget<TextBox>("TextBox",
					0,0, wp->getSize().width*2, 2*wp->getSize().height,
					Align::Default, "ToolTip");
				txt->setTextColour(Colour::White);
				txt->setFontName("font.small");
				txt->setFontHeight(6);  //par
				// img->setNeedKeyFocus(0);  img->setNeedMouseFocus(0);
			*/
			}
		}
		doTestGUI(wp->getEnumerator());
	}
}

void CGuiCom::doSizeGUI(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WP wp = widgets.current();
		string relativeTo = wp->getUserString("RelativeTo");

		if (relativeTo != "")
		{
			//  position & size relative to the widget specified in "RelativeTo" property (or full screen)
			IntSize relSize;
			if (relativeTo == "Screen")
				relSize = IntSize(app->mWindow->getWidth(), app->mWindow->getHeight());
			else
			{	WP window = fWP(relativeTo);
				relSize = window->getSize();
			}
			//  retrieve original size & pos
			IntPoint origPos;  IntSize origSize;
			origPos.left = s2i(wp->getUserString("origPosX"));
			origPos.top  = s2i(wp->getUserString("origPosY"));
			origSize.width  = s2i(wp->getUserString("origSizeX"));
			origSize.height = s2i(wp->getUserString("origSizeY"));
			
			//  calc & apply new size & pos
			float sx = relSize.width / 800.f, sy = relSize.height / 600.f;
			wp->setPosition(IntPoint( int(origPos.left * sx), int(origPos.top * sy) ));
			wp->setSize(IntSize(    int(origSize.width * sx), int(origSize.height * sy) ));
		}
		
		doSizeGUI(wp->getEnumerator());
	}
}

void CGuiCom::setOrigPos(WP wp, const char* relToWnd)
{
	if (!wp)  return;
	wp->setUserString("origPosX", toStr(wp->getPosition().left));
	wp->setUserString("origPosY", toStr(wp->getPosition().top));
	wp->setUserString("origSizeX", toStr(wp->getSize().width));
	wp->setUserString("origSizeY", toStr(wp->getSize().height));
	if (relToWnd)
		wp->setUserString("RelativeTo", relToWnd);
}


///  ❔ Tooltips
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitTooltip()
{
	mToolTip = fWP("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();

	for (VectorWidgetPtr::iterator it = app->vwGui.begin(); it != app->vwGui.end(); ++it)
		setToolTips((*it)->getEnumerator());
}

void CGuiCom::setToolTips(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WP wp = widgets.current();
		wp->setAlign(Align::Default);
				
		IntPoint origPos = wp->getPosition();
		IntSize origSize = wp->getSize();
		
		wp->setUserString("origPosX", toStr(origPos.left));
		wp->setUserString("origPosY", toStr(origPos.top));
		wp->setUserString("origSizeX", toStr(origSize.width));
		wp->setUserString("origSizeY", toStr(origSize.height));

		//  find parent window
		WP p = wp->getParent();
		while (p)
		{
			if (p->getTypeName() == "Window")
			{
				if (p->getUserString("NotSized").empty())
					wp->setUserString("RelativeTo", p->getName());
				break;
			}
			p = p->getParent();
		}
		
		bool tip = wp->isUserString("tip");
		if (tip)  // if has tooltip string
		{
			// needed for translation
			wp->setUserString("tip", TR(wp->getUserString("tip")));
			wp->setNeedToolTip(true);
			wp->eventToolTip += newDelegate(this, &CGuiCom::notifyToolTip);
		}
		//LogO(wp->getName() + (tip ? "  *" : ""));
		setToolTips(wp->getEnumerator());
	}
}

void CGuiCom::notifyToolTip(WP wp, const ToolTipInfo &info)
{
	if (!mToolTip)  return;

	#ifndef SR_EDITOR
	if (!app->isFocGui)
	#else
	if (!app->bGuiFocus)
	#endif
	{	mToolTip->setVisible(false);
		return;
	}
	if (info.type == ToolTipInfo::Show)
	{
		mToolTip->setSize(320, 128);  // start size for wrap
		mToolTipTxt->setSize(320, 128);

		String s = TR(wp->getUserString("tip"));
		mToolTipTxt->setCaption(s);
		const IntSize &si = mToolTipTxt->getTextSize();

		mToolTip->setSize(si.width +8, si.height +8);
		mToolTipTxt->setSize(si.width, si.height);
		mToolTip->setVisible(true);
		boundedMove(mToolTip, info.point);
	}
	else if (info.type == ToolTipInfo::Hide)
		mToolTip->setVisible(false);
}

//  Move a widget to a point while making it stay in the viewport.
void CGuiCom::boundedMove(Widget* moving, const IntPoint& point)
{
	const IntPoint offset(20, 20);  // mouse cursor
	IntPoint p = point + offset;

	const IntSize& size = moving->getSize();
	
	int w = app->mWindow->getWidth();
	int h = app->mWindow->getHeight();
	
	if (p.left + size.width > w)  p.left = w - size.width;
	if (p.top + size.height > h)  p.top = h - size.height;
			
	moving->setPosition(p);
}


//  🗺️ Languages combo
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitLang()
{
	if (languages.empty())
	{	//  add new in core_language.xml too
		auto AddLang = [&](string s){  languages[s] = TR("#{LANG_"+s+"}");  };
		AddLang("en");
		AddLang("de");
		AddLang("fr");
		AddLang("pl");
		AddLang("ru");
		AddLang("fi");
		AddLang("pt_BR");
		AddLang("it");
		AddLang("ro");
		AddLang("sk");
		AddLang("cs");
		AddLang("es");
		AddLang("nl_NL");
		AddLang("hu_HU");
		AddLang("ja");
	}
	Cmb cmb;
	CmbC(cmb, "Lang", comboLanguage);

	for (auto it = languages.cbegin(); it != languages.end(); ++it)
	{
		cmb->addItem(it->second);
		if (it->first == pSet->language)
			cmb->setIndexSelected(cmb->getItemCount()-1);
	}
}

void CGuiCom::comboLanguage(ComboBox* wp, size_t val)
{
	if (val == ITEM_NONE)  return;
	UString sel = wp->getItemNameAt(val);
	
	for (auto it = languages.cbegin(); it != languages.end(); ++it)
	{
		if (it->second == sel)
			pSet->language = it->first;
	}
	LanguageManager::getInstance().setCurrentLanguage(pSet->language);

	// CreateFonts();  //!- todo
	// todo: fix, without restart
	//  reinit gui
	// #ifndef SR_EDITOR
	// bGuiReinit = true;
	// #endif
}


///  🔠 create fonts
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::CreateFonts()
{
	LogO("C--# Create Fonts");
	MyGUI::ResourceManager& mgr = MyGUI::ResourceManager::getInstance();
	
	struct Font
	{
		string name;
		float size;
		int range;
	};
	const int cnt = 7;  //** fonts  --------
	const Font fonts[cnt] = {
		{"font.small",  0.985f* 30.f, 0 },  //  0 all letters Gui
		{"font.normal", 0.985f* 34.f, 0 },
		{"font.big",    0.985f* 39.f, 0 },
		{"hud.text",    42.f, 1 },  //  1 same for hud times
		// {"hud.times",   42.f, 1 },
		{"hud.replay",  32.f, 2 },  //  2 digits only
		{"DigGear",     96.f, 3 },  //  3 gear digits,R,N
		{"hud.fps",     32.f, 4 },  //  4 fps txt
	};
/*
	HlmsUnlit* hlms = (HlmsUnlit*)Root::getSingleton().getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );
	// HlmsUnlitDatablock* db = (HlmsUnlitDatablock*)(hlms->getDatablock(name));
	auto dbs = hlms->getDatablockMap();
	for (auto& db : dbs)
	if (StringUtil::endsWith(db.second.name, "_TrueTypeFont"))
		// LogO(string(db.first));
		LogO(db.second.name);
/**/
	bool jap = pSet->language == "ja", rus = pSet->language == "ru";

	for (int i=0; i < cnt; ++i)
	{
		//  del old
		const auto& fnt = fonts[i];
		const string name = fnt.name;
		if (mgr.isExist(name))
		{
			mgr.removeByName(name);
			// HlmsUnlit* hlms = (HlmsUnlit*)Root::getSingleton().getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );
			// HlmsUnlitDatablock* db = (HlmsUnlitDatablock*)(hlms->getDatablock(name));
			// hlms->destroyDatablock(name);
		}

		//  setup font				   // par
		float size = fnt.size;  // less for low screen res
		size *= max(0.55f, min(1.2f, (app->mWindow->getHeight()/*pSet->windowy*/ - 600.f) / 600.f));

		//  create  --------
		ResourceTrueTypeFont* font = (ResourceTrueTypeFont*)FactoryManager::getInstance().createObject("Resource", "ResourceTrueTypeFont");

		font->setResourceName(name);
		font->setSource( jap ? "NotoSansJP.ttf" :
			"DejaVuLGCSans.ttf");
		/*font->setSize(size);*/  font->setResolution(50);  font->setAntialias(false);
		font->setTabWidth(8);  font->setDistance(4);  font->setOffsetHeight(0);


		bool spc = 0;  //  meh  --
		auto setSpaceWidth = [&](int w)
		{
			//  loading from xml
			xml::Document doc;
			xml::ElementPtr root = doc.createRoot("ResourceTrueTypeFont"), e;
			// root->addAttribute("name", name);

			#define AddE(key, val)  e = root->createChild("Property");  e->addAttribute("key", key);  e->addAttribute("value", val)

			AddE("SpaceWidth", toStr(w));
			font->deserialization(root, Version(3,2,0));  // has initialise!
			spc = 1;
		};

		//  char ranges, unicode  --------
		switch (fnt.range)
		{
		case 0:  //  Gui
		case 1:  //  Hud Times
			font->setSize(
			#ifndef SR_EDITOR
				fnt.range == 1 ? size * pSet->font_times :
			#endif
				size * pSet->font_gui);
			font->addCodePointRange(33, 400);
			font->addCodePointRange(536, 539);  // romanian
			
			if (rus)
			{	font->addCodePointRange(1025, 1105);  // russian
				font->addCodePointRange(8470, 8470);
				font->removeCodePointRange(1026, 1039);
				// font->removeCodePointRange(1104);
				font->addCodePointRange(9728, 9738);
				// font->addCodePointRange(12448, 12543);
			}
			if (jap)  // slow start
			{	font->addCodePointRange(0x30A0, 0x30FF);  // japanese
				font->addCodePointRange(0x3041, 0x3096);
				font->addCodePointRange(0x3400, 0x4DB5);
				font->addCodePointRange(0x4E00, 0x9FCB);
				font->addCodePointRange(0xF900, 0xFA6A);
				font->addCodePointRange(0x2E80, 0x2FD5);
				// font->addCodePointRange(0xFF5F, 0xFF9F);  // half-
				// font->addCodePointRange(0x3000, 0x303F);  // symb
				// font->addCodePointRange(0x31F0, 0x31FF);
				// font->addCodePointRange(0x3220, 0x3243);
				// font->addCodePointRange(0x3280, 0x337F);  // misc
				// font->addCodePointRange(0xFF01, 0xFF5E);  // norm
			}
			break;

	#ifndef SR_EDITOR
		case 2:  //  Replay times
			font->setSize(size * pSet->font_gui);  //?
			font->addCodePointRange(37, 58);
			font->setOffsetHeight(-1);
			// font->mSpaceWidth = 4; //font->setSpaceWidth(4);  // cant
			setSpaceWidth(4);
			break;

		case 3:  //  Hud vel, gear
			font->setSize(size * pSet->font_hud);
			font->addCodePointRange(37, 58);
			font->addCodePointRange(78, 78);  font->addCodePointRange(82, 82);
			// font->addCodePoint(78);  font->addCodePoint(82);  // cant
			font->setAntialias(1);
			font->setOffsetHeight(-1);
			setSpaceWidth(42.f/96.f * size);  // ttf par
			break;
	#endif
		case 4:  //  Fps bar
			font->setSize(size);
			// font->setSize(size * pSet->font_gui);  //?
			font->addCodePointRange(33, 127);
			setSpaceWidth(14.f/32.f * size);  // ttf par
			break;
		}

		if (!spc)
			font->initialise();

		mgr.addResource(font);

		LogO("C--# Created Font: " + font->getResourceName() +"  size: "+fToStr(size));
	}

	// auto enu = mgr.getEnumerator();
	// while (enu)
	// 	LogO(enu.current().first);
	// enu.next();
}


///  Get Materials
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::GetMaterials(String filename, bool clear, String type)
{
	if (clear)
		vsMaterials.clear();
	
	DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(filename);
	if (stream)
	{	try
		{	while(!stream->eof())
			{
				std::string line = stream->getLine();
				StringUtil::trim(line);
 
				if (StringUtil::startsWith(line, type))
				{
					//LogO(line);
					Ogre::vector<String>::type vec = StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (auto it = vec.begin(); it < vec.end(); ++it)
					{
						if (skipFirst)
						{	skipFirst = false;
							continue;
						}
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			LogO("GetMaterials Exception! " + e.getFullDescription());
	}	}
	stream->close();
}

void CGuiCom::GetMaterialsMat(String filename, bool clear, String type)
{
	if (clear)
		vsMaterials.clear();
	
	std::ifstream stream(filename.c_str(), std::ifstream::in);
	if (!stream.fail())
	{	try
		{	while(!stream.eof())
			{
				char ch[256+2];
				stream.getline(ch,256);
				std::string line = ch;
				StringUtil::trim(line);
 
				if (StringUtil::startsWith(line, type))
				{
					//LogO(line);
					auto vec = StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (auto it : vec)
					{
						std::string match = it;
						StringUtil::trim(match);
						if (!match.empty())
						{
							if (skipFirst)
							{	skipFirst = false;  continue;	}

							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			LogO("GetMaterialsMat Exception! " + e.getFullDescription());
	}	}
	else
		LogO("GetMaterialsMat, can't open: " + filename);
	stream.close();
}

void CGuiCom::GetMaterialsJson(String filename, bool clear, String type, String matStart)
{
	if (clear)
		vsMaterials.clear();
	
	std::ifstream stream(filename.c_str(), std::ifstream::in);
	if (!stream.fail())
	{	try
		{
			bool typeStarted = false;
			while(!stream.eof())
			{
				char ch[256+2];
				stream.getline(ch,256);
				std::string line = ch;
 
				if (line.find(type) != string::npos)
					typeStarted = true;

				if (typeStarted && StringUtil::startsWith(line, matStart))
				{
					auto vec = StringUtil::split(line," \t:\"");
					bool skipFirst = false;
					for (auto it : vec)
					{
						std::string match = it;
						StringUtil::trim(match);
						if (!match.empty())
						{
							if (skipFirst)
							{	skipFirst = false;  continue;	}

							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			LogO("GetMaterialsJson Exception! " + e.getFullDescription());
	}	}
	else
		LogO("GetMaterialsJson, can't open: " + filename);
	stream.close();
}
