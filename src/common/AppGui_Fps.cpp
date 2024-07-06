#include "pch.h"
#include "AppGui.h"
#include "Def_Str.h"
#include "CScene.h"
#include "settings.h"

#include <OgreRoot.h>
#include <OgreFrameStats.h>
#include <OgreTextureGpuManager.h>
#include <Vao/OgreVaoManager.h>
#include <Compositor/OgreCompositorManager2.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;
using namespace MyGUI;


//  🌈 Fps, stat colors
//-----------------------------------------------------------------------------------
const int ciClrVal = 9, ciGrn = 3;
const String sClrVal[ciClrVal] =  // long
	{"#A0C0FF", "#20A0FF", "#10FFFF", "#10FF10",
	 "#FFFF10", "#FFC010", "#FF8010", "#FF1010", "#C00000", };

const String clrVal(int i)
{	return sClrVal[std::max(0, std::min(ciClrVal-1, i))];  }

String cvsF(float v, float grn, float red, int precision=2, int width=4)
{
	int c = 4.f * (v - grn) / (red - grn) + ciGrn;
	return clrVal(c) + fToStr(v, precision, width);
}
String cvsI(int v, int grn, int red, int width=4)
{
	int c = 4.f * (v - grn) / (red - grn) + ciGrn;
	return clrVal(c) + iToStr(v, width);
}


//  💫📈 upd Fps stats
//-----------------------------------------------------------------------------------
void AppGui::UpdFpsText(float dt)
{
	if (!pSet->fps_bar)
		return;

	timFps += dt;
	if (timFps < 0.2f)  // 5 fps upd
		return;
	timFps = 0.f;

	const auto *rs = mRoot->getRenderSystem();
	const RenderingMetrics& rm = rs->getMetrics();
	const FrameStats *st = mRoot->getFrameStats();
	const auto *cmp = mRoot->getCompositorManager2();

	const float fps = st->getAvgFps();  //st->getFps(),
	const float m = pSet->fps_mul;

	String txt;  const bool h = fps >= 100.f;
	txt += cvsF( fps,  59.f, 30.f, h ? 0 : 1, h ? 3 : 4) + "  ";
	
	if (pSet->fps_bar == 1)
	{	txFps->setCaption(txt);  return;  }
	
	const float
		tris = rm.mFaceCount/1000000.f,  //**  ? / 7 = 1 + 6 cube, wrong, meh
		mem = GetGPUmem();
	const int
		draw = rm.mDrawCount,
		inst = rm.mInstanceCount,
		vgt = scn->iVegetAll,
		gui = MyGUI::Ogre2RenderManager::getInstance().getBatchCount(),
		ws = cmp->getNumWorkspaces();

	if (pSet->fps_bar == 2)
	{
		txt += cvsF( tris, 4.f*m, 8.f*m, 2,4)+"  ";
		txt += cvsI( draw, 150*m, 1200*m, 3) + "  ";
		txFps->setCaption(txt);  return;
	}
	if (pSet->fps_bar == 3)
	{
		txt += cvsI( draw, 150*m, 1200*m, 3) + "  ";
		txt += cvsF( mem,  600.f*m, 1700.f*m, 0,4);  // + "M\n";
		txFps->setCaption(txt);  return;
	}

	txt += cvsF( tris, 4.f*m, 8.f*m, 2,4)+"  ";  // + "m ";  //txt += "v " + toStr( rm.mVertexCount/1000 ) + "  ";
	txt += cvsI( draw, 150*m, 1200*m, 3) + "\n";

	txt += "#A0C0A0 i ";
	txt += cvsI( inst, 2000*m, 16000*m, 5) + " ";  //txt += "b " + toStr( rm.mBatchCount, 0);
	txt += cvsI( vgt, 3000*m, 20000*m, 5) + "\n";

	txt += "#B0B0B0G " + cvsI( gui, 8*m, 45*m, 2);
	txt += "#9090F0 w " + cvsI( ws, 8, 16, 2) + " ";
	txt += cvsF( mem,  600.f*m, 1700.f*m, 0,4);  // + "M\n";

	//  test colors
	/*for (int i = 15; i < 60; i+=5)
		txt += cvsF( i, 30.f, 50.f, 0,2) + " ";
	txt += "\n";/**/

	txFps->setCaption(txt);

#if 0  // Log Fps - Test, no gui, etc
	static int i = 0;  ++i;
	if (i > 30)  // interval
	{	i = 0;
		txt = "  Fps " + fToStr( fps, 1,4) + 
			"  tri " + fToStr( tris, 2,4) +
			"m  drw " + iToStr( draw, 5 ) +
			"  inst " + iToStr( inst, 5 ) +
			"  vgt " + iToStr( vgt, 5 ) + 
			"  gui " + iToStr( gui, 2 ) +
			"  mem " + fToStr( mem, 0,4) + "M";
		LogO(txt);
	}
#endif
}


//  🆕 create Fps bar widgets
//------------------------------------------------
void AppGui::CreateFpsBar()
{
	Ogre::LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);

	// LogO("---- createScene");
	RenderSystem *rs = mRoot->getRenderSystem();
	rs->setMetricsRecordingEnabled( true );


	if (txFps)  return;
	bckFps = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 234,78, Align::Default, "Main");
	bckFps->setImageTexture("back_fps.png");
	bckFps->setAlpha(0.9f);  //bckFps->setVisible(false);

	txFps = mGui->createWidget<TextBox>("TextBox",
		6,0, 226,78, Align::Default, "Popup");
	txFps->setFontName("hud.fps");  //txFps->setAlpha(1.f);
	txFps->setTextShadow(true);  txFps->setTextShadowColour(Colour::Black);
}


//  ⛓️ util  GPU RAM mem
//-----------------------------------------------------------------------------------
float AppGui::GetGPUmem()
{
	RenderSystem *rs = mRoot->getRenderSystem();
	size_t freeB, capacityB;  // B = Bytes
	bool bIncludesTex;  // Textures
	VaoManager::MemoryStatsEntryVec memStats;
	VaoManager *vaoManager = rs->getVaoManager();
	vaoManager->getMemoryStats( memStats, capacityB, freeB, 0, bIncludesTex );

	TextureGpuManager *mgr = rs->getTextureGpuManager();
	size_t texBCpu, texBGpu, usedStagingTexB, availStagingTexB;
	mgr->getMemoryStats(
		texBCpu, texBGpu, usedStagingTexB, availStagingTexB );

	// Don't count texture memory twice if it's already included in VaoManager
	if( bIncludesTex )
		capacityB -= texBGpu + usedStagingTexB + availStagingTexB;

	const size_t BtoMB = 1024u * 1024u;
	/*char tmpBuffer[256];
	LwString text( Ogre::LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );
	text.clear();
	text.a( "\n\nGPU buffer pools (meshes, const, texture, indirect & uav buffers): ",
			( Ogre::uint32 )( ( capacityB - freeB ) / BtoMB ), "/",
			( Ogre::uint32 )( capacityB / BtoMB ), " MB" );
	outText += text.c_str();
	text.clear();
	text.a(
		"\nGPU StagingTexs. In use: ", ( Ogre::uint32 )( usedStagingTexB / BtoMB ),
		" MB. Available: ", ( Ogre::uint32 )( availStagingTexB / BtoMB ),
		" MB. Total:",
		( Ogre::uint32 )( ( usedStagingTexB + availStagingTexB ) / BtoMB ) );
	outText += text.c_str();*/

	const size_t totalB =
		capacityB + texBGpu + usedStagingTexB + availStagingTexB;

	/*text.clear();
	text.a( "\nGPU Textures:\t", ( Ogre::uint32 )( textureBGpu / BtoMB ), " MB" );
	text.a( "\nCPU Textures:\t", ( Ogre::uint32 )( textureBCpu / BtoMB ), " MB" );
	text.a( "\nTotal GPU:\t", ( Ogre::uint32 )( totalBNeeded / BtoMB ), " MB" );
	outText += text.c_str();*/

	return float( totalB / BtoMB ); // MB
	//return float( 1.f/1024.f * totalB / BtoMB);  // GB
}
