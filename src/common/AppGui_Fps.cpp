#include "pch.h"
#include "AppGui.h"
#include "Def_Str.h"

#include <OgreRoot.h>
#include <OgreFrameStats.h>
#include <OgreTextureGpuManager.h>
#include <Vao/OgreVaoManager.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;
using namespace MyGUI;


//  🌈 Fps, stat colors
//-----------------------------------------------------------------------------------
const int iClrVal = 8;
const String sClrVal[iClrVal] =  // long
	{"#A0C0FF", "#20A0FF", "#10FFFF", "#10FF10", "#FFFF20", "#FFC010", "#FF8010", "#FF1010"};

const String clrVal(int i)
{	return sClrVal[std::max(0, std::min(iClrVal-1, i))];  }

String cvsF(float v, float grn, float red, int precision=2, int width=4)
{
	int c = 4.f * (v - grn) / (red - grn) + 3.f /*grn id*/;
	return clrVal(c) + fToStr(v, precision, width);
}
String cvsI(int v, int grn, int red, int width=4)
{
	int c = 4.f * (v - grn) / (red - grn) + 3.f /*grn id*/;
	return clrVal(c) + iToStr(v, width);
}


//  💫📈 upd Fps stats
//-----------------------------------------------------------------------------------
void AppGui::UpdFpsText()
{
	const RenderSystem *rs = mRoot->getRenderSystem();
	const RenderingMetrics& rm = rs->getMetrics();
	const FrameStats *st = mRoot->getFrameStats();

	const float fps = st->getAvgFps(),  //st->getFps(),
		tris = rm.mFaceCount/1000000.f, mem = GetGPUmem();
	const int draw = rm.mDrawCount,
		inst = rm.mInstanceCount, vgt = 0, //scn->cntAll,
		gui = MyGUI::Ogre2RenderManager::getInstance().getBatchCount();

	String txt;
	txt += cvsF( fps,  59.f, 30.f, fps >= 100.f ? 0 : 1,4) + "  ";
	txt += cvsF( tris, 1.f, 3.f, 2,4) + "m ";  //txt += "v " + toStr( rm.mVertexCount/1000 ) + "  ";
	txt += cvsI( draw, 50, 180, 3) + "\n";

	txt += "#A0C0A0 Vgt ";
	txt += cvsI( inst, 1000, 8000, 5) + " ";  //txt += "b " + toStr( rm.mBatchCount, 0);
	txt += cvsI( vgt, 3000, 15000, 5) + "\n";

	txt += "#B0B0B0 Gui " + cvsI( gui, 5, 30, 2) + " ";
	txt += cvsF( mem,  500.f, 1300.f, 0,4) + "M\n";

	//  test colors
	/*for (int i = 15; i < 60; i+=5)
		txt += cvsF( i, 30.f, 50.f, 0,2) + " ";
	txt += "\n";/**/

	txFps->setCaption(txt);
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
		6,3, 226,78, Align::Default, "ToolTip");
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
