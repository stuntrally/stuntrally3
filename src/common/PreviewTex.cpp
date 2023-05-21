#include "pch.h"
#include "RenderConst.h"
#include "PreviewTex.h"
#include "Def_Str.h"
#include "paths.h"
#include <fstream>
#include <OgreException.h>
#include <OgreResourceGroupManager.h>
#include <OgreTextureGpuManager.h>
#include <OgreImage2.h>
#include <OgreDataStream.h>
#include <OgreStagingTexture.h>
#include <OgrePixelFormatGpu.h>
using namespace Ogre;
using Ogre::uint8;

//#define CHANGE_SIZE  // fixme

TextureGpuManager *PreviewTex::mgr = 0;

//  1 set name (size unknown)  or
void PreviewTex::SetName(String texName)
{
	sName = texName;
}

//  1 create (known size)
bool PreviewTex::Create(int x, int y, String texName)
{
	if (!mgr)  return false;
#ifdef CHANGE_SIZE
	// LogO(texName+" "+toStr(x)+" "+toStr(y));
	if (prvTex)  // fixme
		Destroy();
#endif
	xSize = x;  ySize = y;
	sName = texName;
	// prvTex = TextureManager::getSingleton().createManual(
	// 	sName, rgDef, TEX_TYPE_2D,
	// 	x, y, 5, //par mipmaps
	// 	PF_BYTE_BGRA, TU_DEFAULT);  //TU_DYNAMIC_WRITE_ONLY_DISCARDABLE

	PixelFormatGpu fmt = PFG_RGBA8_UNORM;

	prvTex = mgr->createTexture( sName,
		GpuPageOutStrategy::SaveToSystemRam,
		TextureFlags::ManualTexture, TextureTypes::Type2D );
	prvTex->setResolution( xSize, ySize );
	prvTex->setPixelFormat( fmt );
	prvTex->scheduleTransitionTo( GpuResidency::Resident );

	//Clear();
	return true; //(bool)prvTex; //.isNull();
}

//  3 destroy
void PreviewTex::Destroy()
{
	if (!mgr)  return;
	if (prvTex)
	{	mgr->destroyTexture( prvTex );
		prvTex = 0;
	}
}


//  2 load image from path
bool PreviewTex::Load(String path, bool force,  uint8 b, uint8 g, uint8 r, uint8 a)
{
	if (!PATHS::FileExists(path))
	{
		LogO("PreviewTex: doesn't exist: "+path);
		Clear(b,g,r,a);
		return false;
	}

	if (curPath == path && !force)  // check if same
		return false;
	curPath = path;
	
	bool loaded = false;
	std::ifstream ifs(path.c_str(), std::ios::binary|std::ios::in);
	if (ifs.is_open())
	{
		String ext;
		String::size_type id = path.find_last_of('.');
		if (id != String::npos)
		{
			ext = path.substr(id+1);
			DataStreamPtr data(new FileStreamDataStream(path, &ifs, false));
			Image2 img;
			img.load(data, ext);

			auto w = img.getWidth(), h = img.getHeight();
		#ifdef CHANGE_SIZE
			if (!(prvTex && w == xSize && h == ySize))  // fixme.. white?
				Create(w, h, sName);
		#else
			if (!prvTex)
				Create(w, h, sName);
		#endif
			//**  convert sRGB, fixes bad white clr  // todo- in shader, slow
			// Ogre::Timer ti;
			auto fmt = img.getPixelFormat();  // LogO(toStr(fmt)+" fmt prv");
			if (fmt == PFG_RGBA8_UNORM)
			{
				uint8* buf = (uint8*)img.getRawBuffer();
				uint p = 0; //, a = img.getBytesPerRow(0);
				for (int y=0; y < ySize; ++y)
				for (int x=0; x < xSize; ++x)
				{
					uint8 r = buf[p], g = buf[p+1], b = buf[p+2];
					r = 255.f * PixelFormatGpuUtils::fromSRGB(r / 255.f);
					g = 255.f * PixelFormatGpuUtils::fromSRGB(g / 255.f);
					b = 255.f * PixelFormatGpuUtils::fromSRGB(b / 255.f);
					buf[p] = r;  buf[p+1] = g;  buf[p+2] = b;
					p += 4;  // a
				}
			}
			// LogO(String("::: Time prv sRGB: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  // 44 ms  6 Fps
				
			//LogO("PRV: "+toStr(img.getWidth())+" "+toStr(img.getHeight())+"  "+path);
			prvTex->scheduleTransitionTo( GpuResidency::Resident );

			///----
			auto pf = PFG_RGBA8_UNORM_SRGB;
			// auto pf = PFG_RGBA8_UNORM;
			const TextureBox& imgData = img.getData(0);

			StagingTexture* tex = mgr->getStagingTexture(
				xSize, ySize, 1u, 1u, pf );
			tex->startMapRegion();
			TextureBox texBox = tex->mapRegion(
				xSize, ySize, 1u, 1u, pf );

			texBox.copyFrom( imgData );  // upload

			tex->stopMapRegion();
			try
			{
				tex->upload( texBox, prvTex, 0, 0, 0 );
			}
			catch (Exception e)
			{
				LogO(String("PrvTex can't upload ")/*+e.what()*/);
			}
			mgr->removeStagingTexture( tex );  tex = 0;


			/*if (img.getWidth() == prvTex->getWidth() &&
				img.getHeight() == prvTex->getHeight())  // same dim
				prvTex->getBuffer()->blitFromMemory(img.getPixelBox());
			else
				Clear(b,g,r,a);*/

			//prvTex->setNumMipmaps(5);
			//prvTex->unload();  prvTex->loadImage(img);  //same

			loaded = true;
		}
		ifs.close();
	}else
		Clear(b,g,r,a);

	return loaded;
}

bool PreviewTex::Load(String path, bool force)
{
	return Load(path, force, 100, 60, 30, 60);
}


void PreviewTex::Clear(const uint8 b, const uint8 g, const uint8 r, const uint8 a)
{
	if (!prvTex)  return;

	StagingTexture* tex = mgr->getStagingTexture(
		xSize, ySize, 1u, 1u, PFG_RGBA8_UNORM );
	tex->startMapRegion();
	TextureBox texBox = tex->mapRegion(
		xSize, ySize, 1u, 1u, PFG_RGBA8_UNORM );

	uint8* data = new uint8[xSize * ySize * 4], *p = data;

	for (size_t y = 0; y < ySize; ++y)
	for (size_t x = 0; x < xSize; ++x)
	{
		*p++ = b;  *p++ = g;  *p++ = r;  *p++ = a;
	}

	//  upload
	texBox.copyFrom( data, xSize, ySize, xSize * 4 );

	tex->stopMapRegion();
	try
	{
		tex->upload( texBox, prvTex, 0, 0, 0 );
	}
	catch (Exception e)
	{
		LogO(String("PrvTex can't upload ")/*+e.what()*/);
	}
	
	mgr->removeStagingTexture( tex );  tex = 0;
	delete[] data;
}


//-------------------------------------------------------------------------------------------------------
//  utility for terrain textures
//  copies other texture's r channel to this texture's alpha
//  note: both must be same size
bool PreviewTex::LoadTer(String sRgb, String sAa, float defA)
{
	/*
	curPath = sRgb;
	bool loaded = false;
	std::ifstream ifR(sRgb.c_str(), std::ios::binary|std::ios::in);
	std::ifstream ifA(sAa.c_str(),  std::ios::binary|std::ios::in);
	String exR;  String::size_type idR = sRgb.find_last_of('.');
	String exA;  String::size_type idA = sAa.find_last_of('.');

	//  no alpha, use default const value
	if (ifR.is_open() && !ifA.is_open() &&
		idR != String::npos)
	{
		exR = sRgb.substr(idR+1);  exA = sAa.substr(idA+1);
		DataStreamPtr dataR(new FileStreamDataStream(sRgb, &ifR, false));
		Image imR; 	imR.load(dataR, exR);
	
		PixelBox pbR = imR.getPixelBox();
		//uchar* pR = static_cast<uchar*>(pbR.data);
		//int aR = pbR.getRowSkip() * PixelUtil::getNumElemBytes(pbR.format);

		xSize = imR.getWidth();  ySize = imR.getHeight();
		prvTex = TextureManager::getSingleton().createManual(
			sName, rgDef, TEX_TYPE_2D,
			xSize, ySize, 5,
			PF_BYTE_BGRA, TU_DEFAULT);

		//  fill texture  rgb,a
		HardwarePixelBufferSharedPtr pt = prvTex->getBuffer();
		pt->lock(HardwareBuffer::HBL_DISCARD);

		const PixelBox& pb = pt->getCurrentLock();
		uint8* pD = static_cast<uint8*>(pb.data);
		int aD = pb.getRowSkip() * PixelUtil::getNumElemBytes(pb.format);
		 
		float fA = defA * 255.f;
		size_t j,i;
		for (j = 0; j < ySize; ++j)
		{
			for (i = 0; i < xSize; ++i)   // B,G,R,A
			{	
				ColourValue cR = pbR.getColourAt(i,j,0);
				*pD++ = cR.b * 255.f;
				*pD++ = cR.g * 255.f;
				*pD++ = cR.r * 255.f;
				*pD++ = fA;
			}
			pD += aD;
		}
		pt->unlock();
	
		loaded = true;
	}
	else
	if (ifR.is_open() && ifA.is_open() &&
		idR != String::npos && idA != String::npos)
	{
		exR = sRgb.substr(idR+1);  exA = sAa.substr(idA+1);
		DataStreamPtr dataR(new FileStreamDataStream(sRgb, &ifR, false));
		DataStreamPtr dataA(new FileStreamDataStream(sAa,  &ifA, false));
		Image imR; 	imR.load(dataR, exR);
		Image imA; 	imA.load(dataA, exA);
	
		PixelBox pbR = imR.getPixelBox();
		PixelBox pbA = imA.getPixelBox();
		//uchar* pR = static_cast<uchar*>(pbR.data);
		//uchar* pA = static_cast<uchar*>(pbA.data);
		//int aR = pbR.getRowSkip() * PixelUtil::getNumElemBytes(pbR.format);
		//int aA = pbA.getRowSkip() * PixelUtil::getNumElemBytes(pbA.format);

		xSize = imR.getWidth();  ySize = imR.getHeight();
		prvTex = TextureManager::getSingleton().createManual(
			sName, rgDef, TEX_TYPE_2D,
			xSize, ySize, 5,
			PF_BYTE_BGRA, TU_DEFAULT);

		//  fill texture  rgb,a
		HardwarePixelBufferSharedPtr pt = prvTex->getBuffer();
		pt->lock(HardwareBuffer::HBL_DISCARD);

		const PixelBox& pb = pt->getCurrentLock();
		uint8* pD = static_cast<uint8*>(pb.data);
		int aD = pb.getRowSkip() * PixelUtil::getNumElemBytes(pb.format);
		 
		size_t j,i;
		for (j = 0; j < ySize; ++j)
		{
			for (i = 0; i < xSize; ++i)   // B,G,R,A
			{	
				ColourValue cR = pbR.getColourAt(i,j,0);
				ColourValue cA = pbA.getColourAt(i,j,0);
				*pD++ = cR.b * 255.f;
				*pD++ = cR.g * 255.f;
				*pD++ = cR.r * 255.f;
				*pD++ = cA.r * 255.f;
			}
			pD += aD;
		}
		pt->unlock();
	
		loaded = true;
	}else
		Clear();

	return loaded;*/
	return true;
}
