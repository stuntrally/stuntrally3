#include "enums.h"
#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "CScene.h"
#include "SceneClasses.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include <OgreTimer.h>
#include "HudRenderable.h"
#include "Terra.h"
#include <OgreTextureGpuManager.h>
#include <OgreHlmsUnlit.h>
#include <OgrePixelFormatGpu.h>
#include <OgreStagingTexture.h>
#include <OgreHlmsPbsPrerequisites.h>
#include <MyGUI.h>
using namespace Ogre;


//  color factors for edit mode D,S,E,F
const float App::brClr[4][3] = {
	{0.3, 0.8, 0.1}, {0.2, 0.8, 0.6}, {0.6, 0.9, 0.6}, {0.4, 0.7, 1.0} };


static float GetAngle(float x, float y)
{
	if (x == 0.f && y == 0.f)
		return 0.f;

	if (y == 0.f)
		return (x < 0.f) ? PI_d : 0.f;
	else
		return (y < 0.f) ? atan2f(-y, x) : (2*PI_d - atan2f(y, x));
}


//  🖼️ update brush preview texture
//--------------------------------------------------------------------------------------------------------------------------
void App::UpdBrushPrv(bool first)
{
	if (!first && (edMode >= ED_Road /*|| bMoveCam/*|| !bEdit()*/))  return;
	if (!pSet->brush_prv || !brPrvTex)  return;

	// if (!first || !brPrvTex)  return;
	auto* mgr =	mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();

	StagingTexture* tex = mgr->getStagingTexture(
		BrPrvSize, BrPrvSize, 1u, 1u, PFG_RGBA8_UNORM );
	tex->startMapRegion();
	TextureBox texBox = tex->mapRegion(
		BrPrvSize, BrPrvSize, 1u, 1u, PFG_RGBA8_UNORM );

	uint8* data = new uint8[BrPrvSize * BrPrvSize * 4];
	updBrushData(data);

	//  upload
	texBox.copyFrom( data, BrPrvSize, BrPrvSize, BrPrvSize * 4 );

	tex->stopMapRegion();
	tex->upload( texBox, brPrvTex, 0, 0, 0 );
	mgr->removeStagingTexture( tex );  tex = 0;
	delete[] data;
}

//  fill brush preview data
//--------------------------------------------------------------------------------------------------------------------------
void App::updBrushData(uint8* data)
{
	uint8* p = data;

	const float fB = brClr[edMode][0]*255.f, fG = brClr[edMode][1]*255.f, fR = brClr[edMode][2]*255.f;
	const float gp = 2.f;  // 2.4 gamma srgb fix-

	const float s = BrPrvSize * 0.5f, s1 = 1.f/s,
		fP = curBr().power, fQ = curBr().freq*5.f, nof = curBr().offset;
	const int oct = curBr().octaves;  const float PiN = PI_d/oct;

	switch (curBr().shape)
	{
	case BRS_Noise2:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * (1.0-pow( fabs(CScene::Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*d)) * (1.5f-fP*0.1);
			c = std::max(0.f, pow(c, gp));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			// *p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
			*p++ = bB;  *p++ = bG;  *p++ = bR;  *p++ = bG > 2 ? 255 : 0;
		}	break;

	case BRS_Noise:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( fabs(CScene::Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*0.5f) * 0.9f;
			c = std::max(0.f, pow(c, gp));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bB;  *p++ = bG;  *p++ = bR;  *p++ = bG > 2 ? 255 : 0;
		}	break;

	case BRS_Sinus:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = powf( sinf(d * PI_d*0.5f), fP);
			c = std::max(0.f, pow(c, gp));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bB;  *p++ = bG;  *p++ = bR;  *p++ = bG > 2 ? 255 : 0;
		}	break;

	case BRS_Ngon:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			float k = GetAngle(fx,fy);  // 0..2Pi

    		float c = std::max(0.f, std::min(1.f,
    			fQ * powf( fabs(d / (-1.f+nof + cosf(PiN) / cosf( fmodf(k, 2*PiN) - PiN ) )),fP) ));
			c = std::max(0.f, pow(c, gp));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bB;  *p++ = bG;  *p++ = bR;  *p++ = bG > 2 ? 255 : 0;
		}	break;

	case BRS_Triangle:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = powf( fabs(d), fP);
			c = std::max(0.f, pow(c, gp));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bB;  *p++ = bG;  *p++ = bR;  *p++ = bG > 2 ? 255 : 0;
		}	break;
	default:  break;
	}
}

//  fill brush data (shape), after size change
//--------------------------------------------------------------------------------------------------------------------------
void App::updBrush()
{
	//  ranges
	if (curBr().size < 2)  curBr().size = 2;
	if (curBr().size > BrushMaxSize)  curBr().size = BrushMaxSize;
	if (curBr().freq < 0.01)  curBr().freq = 0.01;
	if (curBr().power < 0.02)  curBr().power = 0.02;

	int size = (int)curBr().size, a = 0;
	float s = size * 0.5f, s1 = 1.f/s,
		fP = curBr().power, fQ = curBr().freq*5.f, ofs = curBr().offset;
	int oct = curBr().octaves;  const float PiN = PI_d/oct;

	switch (curBr().shape)
	{
	case BRS_Noise2:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * (1.0-pow( fabs(CScene::Noise(x*s1+ofs,y*s1+ofs, fQ, oct, 0.5f)), fP*d)) * (1.5f-fP*0.1);
			c = std::max(0.f, c);
			
			mBrushData[a] = std::min(1.f, c );
		}	}	break;


	case BRS_Noise:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( fabs(CScene::Noise(x*s1+ofs,y*s1+ofs, fQ, oct, 0.5f)), fP*0.5f);

			mBrushData[a] = std::max(-1.f, std::min(1.f, c ));
		}	}	break;

	case BRS_Sinus:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			
			float c = powf( sinf(d * PI_d*0.5f), fP);
			mBrushData[a] = c;
		}	}	break;

	case BRS_Ngon:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			float k = GetAngle(fx,fy);  // 0..2Pi

			float c = std::max(0.f, std::min(1.f,
				fQ * powf( fabs(d / (-1.f+ofs + cosf(PiN) / cosf( fmodf(k, 2*PiN) - PiN ) )),fP) ));
			mBrushData[a] = c;
		}	}	break;

	case BRS_Triangle:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			
			float c = powf( fabs(d), fP);
			mBrushData[a] = c;
		}	}	break;
	default:  break;
	}
	
	//  filter brush kernel  ------
	auto& mBrFilt = br[ED_Filter].filter;
	if (mBrFilt != mBrFiltOld)
	{	mBrFilt = std::max(0.f, std::min(8.f, mBrFilt));
		mBrFiltOld = mBrFilt;
	
		delete[] pBrFmask;  pBrFmask = 0;
		const float fl = mBrFilt;
		const int f = ceil(fl);  float fd = 1.f + fl - floor(fl);
		int m,i,j;

		//  gauss kernel for smoothing
		const int mm = (f*2+1)*(f*2+1);
		pBrFmask = new float[mm];  m = 0;

		float fm = 0.f;  //sum
		for (j = -f; j <= f; ++j)
		{
			float fj = float(j)/f;
			for (i = -f; i <= f; ++i, ++m)
			{
				float fi = float(i)/f;
				float u = std::max(0.f, fd - sqrtf(fi*fi+fj*fj) );
				pBrFmask[m] = u;  fm += u;
			}
		}
		fm = 1.f / fm;  //avg
		for (m = 0; m < mm; ++m)
			pBrFmask[m] *= fm;
	}
	
	UpdBrushPrv();  // upd skip..
}


///  🔄🔁 Terrain  generate
///--------------------------------------------------------------------------------------------------------------------------
void CGui::btnTerGenerate(WP wp)
{
	const std::string& n = wp->getName();
	bool add = false, sub = false;
	if (n == "TerrainGenAdd")  add = true;  else
	if (n == "TerrainGenSub")  sub = true;/*else
	if (n == "TerrainGenMul")  mul = true; ok */

	float* hfData = &td().hfHeight[0];
	const int sx = td().iVertsX;
	const float s = sx * 0.5f, s1 = 1.f/s;
	const float ox = pSet->gen_ofsx, oy = pSet->gen_ofsy;

	//)  road test
	bool bRoad = pSet->gen_roadsm > 0.1f;
	float rdPow = pSet->gen_roadsm;  //-
	int r = 0;
	/*Image2 imgRoad;  // fixme outside road
	if (bRoad)
	{
		try {	imgRoad.load(String("roadDensity.png"),"General");  }
		catch(...) {	}
		r = imgRoad.getWidth();
	}*/

	Ogre::Timer ti;

	//  generate noise terrain hmap
	int a,x,y;  float c;

	for (y=0; y < sx; ++y)  {  a = y * sx;
	for (x=0; x < sx; ++x,++a)
	{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1

		c = CScene::Noise(y*s1-oy, x*s1+ox, pSet->gen_freq, pSet->gen_oct, pSet->gen_persist) * 0.8f;
		c = c >= 0.f ? powf(c, pSet->gen_pow) : -powf(-c, pSet->gen_pow);

		//  check if on road - uses roadDensity.png
		//  todo: smooth depends on -smooth grass dens par, own val?
		/*if (bRoad)
		{
			int mx = ( fx+1.f)*0.5f*r, my = (-fy+1.f)*0.5f*r;
					
			float cr = 0.f;//; imgRoad.getColourAt(
				// std::max(0,std::min(r-1, mx)), std::max(0,std::min(r-1, my)), 0).r;

			//c = c + std::max(0.f, std::min(1.f, 2*c-cr)) * pow(cr, rdPow);
			c *= pow(cr, rdPow);
		}*/
		// fixme: ter gen ang pars
		//c *= app->linRange(hfAng[a],  pSet->gen_terMinA,pSet->gen_terMaxA, pSet->gen_terSmA);
		c *= CScene::linRange(hfData[a], pSet->gen_terMinH,pSet->gen_terMaxH, pSet->gen_terSmH);

		hfData[a] = add ? (hfData[a] + c * pSet->gen_scale + pSet->gen_ofsh) : (
					sub ? (hfData[a] - c * pSet->gen_scale - pSet->gen_ofsh) :
						  (hfData[a] * c * pSet->gen_mul) );
	}	}

	LogO(String(":::* Time Ter Gen: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	int size = sx * sx * sizeof(float);
	saveNewHmap(hfData, size);

	LogO(String(":::* Time Ter Gen save: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");

	app->bNewHmap = true;	app->UpdateTrack();
}

//  🖼️ update terrain generator preview texture
//--------------------------------------------------------------------------------------------------------------------------
void App::UpdTerGenPrv(bool first)
{
	// if (!first || !terPrvTex)  return;
	auto* mgr =	mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();

	StagingTexture* tex = mgr->getStagingTexture(
		TerPrvSize, TerPrvSize, 1u, 1u, PFG_RGBA8_UNORM );
	tex->startMapRegion();
	TextureBox texBox = tex->mapRegion(
		TerPrvSize, TerPrvSize, 1u, 1u, PFG_RGBA8_UNORM );

	uint8* data = new uint8[TerPrvSize * TerPrvSize * 4], *p = data;

	const static float fB[2] = { 90.f, 90.f}, fG[2] = {255.f,160.f}, fR[2] = { 90.f,255.f};

	const float s = TerPrvSize * 0.5f, s1 = 1.f/s;
	const float ox = pSet->gen_ofsx, oy = pSet->gen_ofsy;

	for (int y = 0; y < TerPrvSize; ++y)
	for (int x = 0; x < TerPrvSize; ++x)
	{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1

		float c = CScene::Noise(x*s1-oy, y*s1+ox,
			pSet->gen_freq, pSet->gen_oct, pSet->gen_persist) * 0.8f;  // par fit
		bool b = c >= 0.f;
		c = b ? powf(c, pSet->gen_pow) : -powf(-c, pSet->gen_pow);

		int i = b ? 0 : 1;  c = b ? c : -c;
		//c *= pSet->gen_scale;  //no
		
		uint8 bR = c * fR[i], bG = c * fG[i], bB = c * fB[i];
		// *p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = 255;//bG > 32 ? 255 : 0;
		*p++ = bB;  *p++ = bG;  *p++ = bR;  *p++ = 255;
	}

	//  upload
	texBox.copyFrom( data, TerPrvSize, TerPrvSize, TerPrvSize * 4 );

	tex->stopMapRegion();
	tex->upload( texBox, terPrvTex, 0, 0, 0 );
	mgr->removeStagingTexture( tex );  tex = 0;
	delete[] data;
}



///  get edit rect
//-----------------------------------------------------------------------------------------------
bool App::GetEditRect(Vector3& pos, Rect& rcBrush, Rect& rcMap, int size,  int& cx, int& cy)
{
	float tws = scn->td->fTerWorldSize;
	int t = scn->ter->getSize(); //scn->sc->td.iTerSize;

	//  world float to map int
	int mapX = (pos.x + 0.5*tws)/tws*t, mapY = (pos.z + 0.5*tws)/tws*t;
	mapX = std::max(0,std::min(t-1, mapX)), mapY = std::max(0,std::min(t-1, mapY));

	int brS = (int)curBr().size;
	float hBr = brS * 0.5f;
	rcMap = Rect(mapX-hBr, mapY-hBr, mapX+hBr, mapY+hBr);
	rcBrush = Rect(0,0, brS,brS);
	cx = 0;  cy = 0;

	if (rcMap.left < 0)  // trim
	{	rcBrush.left += 0 - rcMap.left;  cx += -rcMap.left;
		rcMap.left = 0;
	}
	if (rcMap.top < 0)
	{	rcBrush.top  += 0 - rcMap.top;   cy += -rcMap.top;
		rcMap.top = 0;
	}
	if (rcMap.right > size)
	{	rcBrush.right  -= rcMap.right - size;
		rcMap.right = size;
	}
	if (rcMap.bottom > size)
	{	rcBrush.bottom -= rcMap.bottom - size;
		rcMap.bottom = size;
	}

	if (rcMap.right - rcMap.left < 1 ||
		rcMap.bottom - rcMap.top < 1)
		return false;  // no area
	
	/*sprintf(sBrushTest,
		" ---br rect--- \n"
		"size: %3d %6.3f \n"
		"pos %3d %3d  c: %3d %3d \n"
		"rect:  %3d %3d  %3d %3d \n"
		"map: %3d %3d  %3d %3d \n"
		"br: %3d %3d  %3d %3d \n"
		,brS, mBrSize[curBr]
		,mapX, mapY, cx, cy
		 ,rcMap.right-rcMap.left, rcMap.bottom-rcMap.top
		 ,rcBrush.right-rcBrush.left, rcBrush.bottom-rcBrush.top
		,rcMap.left, rcMap.top, rcMap.right, rcMap.bottom
		,rcBrush.left, rcBrush.top, rcBrush.right, rcBrush.bottom);
	LogO(String(sBrushTest));/**/
	return true;
}


///  ^v Deform
//-----------------------------------------------------------------------------------------------
void App::Deform(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy, size = scn->ter->getSize();
	if (!GetEditRect(pos, rcBrush, rcMap, size, cx,cy))
		return;
	// LogO(iToStr(rcMap.top)+" "+iToStr(rcMap.bottom)+" "+iToStr(rcMap.left)+" "+iToStr(rcMap.right));
	
	auto& fHmap = scn->ter->getHeightData();
	
	float its = curBr().intens * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * size + rcMap.left;
		brPos = jj * BrushMaxSize + cx;
		//brPos = std::max(0, std::min(BrushMaxSize*BrushMaxSize-1, brPos ));

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			///  pos float -> brush data compute here (for small size brushes) ..
			fHmap[mapPos] += mBrushData[brPos] * its;  // deform
			++mapPos;  ++brPos;
		}
	}
	scn->ter->dirtyRect(rcMap);
	scn->UpdBlendmap();
	// bTerUpd = true;
}


///  -_ set Height
//-----------------------------------------------------------------------------------------------
void App::Height(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy, size = scn->ter->getSize();
	if (!GetEditRect(pos, rcBrush, rcMap, size, cx,cy))
		return;
	
	auto& fHmap = scn->ter->getHeightData();
		
	const float its = curBr().intens * dtime * brMul, h = curBr().height;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * size + rcMap.left;
		brPos = jj * BrushMaxSize + cx;

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			float d = h - fHmap[mapPos];
			d = d > 2.f ? 2.f : d < -2.f ? -2.f : d;  // par speed-
			fHmap[mapPos] += d * mBrushData[brPos] * its;
			++mapPos;  ++brPos;
		}
	}
	scn->ter->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}


///  ~- Smooth (Flatten)
//-----------------------------------------------------------------------------------------------
void App::Smooth(Vector3 &pos, float dtime)
{
	float avg = 0.0f;
	int sample_count = 0;
	CalcSmoothFactor(pos, avg, sample_count);
	
	if (sample_count)
		SmoothTer(pos, avg / (float)sample_count, dtime);
}

void App::CalcSmoothFactor(Vector3 &pos, float& avg, int& sample_count)
{
	Rect rcBrush, rcMap;  int cx,cy, size = scn->ter->getSize();
	if (!GetEditRect(pos, rcBrush, rcMap, size, cx,cy))
		return;
	
	auto& fHmap = scn->ter->getHeightData();
	int mapPos;

	avg = 0.0f;  sample_count = 0;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j)
	{
		mapPos = j * size + rcMap.left;
		for (int i = rcMap.left;i < rcMap.right; ++i)
		{
			avg += fHmap[mapPos];  ++mapPos;
		}
	}
	sample_count = (rcMap.right - rcMap.left) * (rcMap.bottom - rcMap.top);
}

//-----------------------------------------------------------------------------------------------
void App::SmoothTer(Vector3 &pos, float avg, float dtime)
{
	Rect rcBrush, rcMap;  int cx,cy, size = scn->ter->getSize();
	if (!GetEditRect(pos, rcBrush, rcMap, size, cx,cy))
		return;
	
	auto& fHmap = scn->ter->getHeightData();
	float mRatio = 1.f, brushPos;
	int mapPos;
	float mFactor = curBr().intens * dtime * 0.1f;

	for (int j = rcMap.top; j < rcMap.bottom; ++j)
	{
		brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BrushMaxSize;
		brushPos += rcBrush.left;
		//**/brushPos += cy * BrushMaxSize + cx;
		mapPos = j * size + rcMap.left;

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			float val = avg - fHmap[mapPos];
			val = val * std::min(mBrushData[(int)brushPos] * mFactor, 1.0f);
			fHmap[mapPos] += val;
			++mapPos;
			brushPos += mRatio;
		}
	}
	scn->ter->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}


///  ^v \ Filter - low pass, removes noise
//-----------------------------------------------------------------------------------------------
void App::Filter(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy, size = scn->ter->getSize();
	if (!GetEditRect(pos, rcBrush, rcMap, size, cx,cy))
		return;
	
	auto& fHmap = scn->ter->getHeightData();
	
	float its = curBr().intens * dtime * std::min(1.f,brMul);  //mul >1 errors
	int mapPos, brPos, jj = cy,
		ter = size, ter2 = ter*ter, ter1 = ter+1;

	const int f = ceil(curBr().filter);
	int x,y,m,yy,i,j;
	
	for (j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * ter + rcMap.left;
		brPos = jj * BrushMaxSize + cx;

		for (i = rcMap.left; i < rcMap.right; ++i)
		if (mapPos -f*ter1 >= 0 && mapPos +f*ter1 < ter2)  // ter borders
		{
			//  sum in kernel
			float s = 0.f;  m = 0;
			for (y = -f; y <= f; ++y) {  yy = y*ter-f;
			for (x = -f; x <= f; ++x, ++m, ++yy)
				s += fHmap[mapPos + yy] * pBrFmask[m];  }
				
			fHmap[mapPos] += (s-fHmap[mapPos]) * mBrushData[brPos] * its;  // filter
			++mapPos;  ++brPos;
		}
	}
	scn->ter->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}



//  🖼️ previews  for brush and noise ter gen
//--------------------------------------------------------------------------------------------------------------------------
void App::CreatePreviews()
{
	LogO("C--- Create prv tex");
	auto *mgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	
	//  brush tex ----
	brPrvTex = mgr->createTexture("BrushTex",
		GpuPageOutStrategy::SaveToSystemRam,
		TextureFlags::ManualTexture, TextureTypes::Type2D );

	brPrvTex->setResolution( BrPrvSize, BrPrvSize );
	brPrvTex->scheduleTransitionTo( GpuResidency::OnStorage );
	brPrvTex->setPixelFormat( PFG_RGBA8_UNORM );
	brPrvTex->scheduleTransitionTo( GpuResidency::Resident );
	// "BrushPrvTex", rgDef, TEX_TYPE_2D,
	// BrPrvSize,BrPrvSize,0, PF_BYTE_RGBA, TU_DYNAMIC);
	
	UpdBrushPrv(true);


	//  ter gen tex ----
	terPrvTex = mgr->createTexture("TerGenTex",
		GpuPageOutStrategy::SaveToSystemRam,
		TextureFlags::ManualTexture, TextureTypes::Type2D );

	terPrvTex->setResolution( TerPrvSize, TerPrvSize );
	terPrvTex->scheduleTransitionTo( GpuResidency::OnStorage );
	terPrvTex->setPixelFormat( PFG_RGBA8_UNORM );
	terPrvTex->scheduleTransitionTo( GpuResidency::Resident );

	UpdTerGenPrv(true);


	//  rect prv ----
	for (int i=0; i < 2; ++i)
	{
		String sMtr = i ? "edTerGen" : "edBrush";
		const auto hlms = dynamic_cast<HlmsUnlit*>(mRoot->getHlmsManager()->getHlms(HLMS_UNLIT));
		HlmsUnlitDatablock* db = dynamic_cast<HlmsUnlitDatablock*>(hlms->getDatablock(sMtr));
		db->setTexture(PBSM_DIFFUSE, i ? terPrvTex : brPrvTex);

		auto& hr = i ? hrTerGen : hrBrush;
		hr = new HudRenderable(sMtr, mSceneMgr,
			OT_TRIANGLE_LIST, true,false, RV_Hud,RQG_Hud1, 1);
		
		hr->begin();
		auto s = pSet->size_minimap * (i ? 1.4f : 0.8f);  //par
		Real x1 = -1.f, x2 = x1 + s / asp, y1 = -0.8f, y2 = y1 + s;
		hr->position(x1,y1, 0.f);  hr->texUV(0.f, 1.f);
		hr->position(x2,y1, 0.f);  hr->texUV(1.f, 1.f);
		hr->position(x1,y2, 0.f);  hr->texUV(0.f, 0.f);
		hr->position(x2,y2, 0.f);  hr->texUV(1.f, 0.f);
		hr->end();

		auto& nd = i ? ndTerGen : ndBrush;
		nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		nd->attachObject(hr);
		nd->setVisible(false);
	}
}
