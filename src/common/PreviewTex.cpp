#include "pch.h"
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
	// LogO("PreviewTex: "+path);
	if (!prvTex)
		return false;
	// prvTex->waitForData();  
	if (prvTex->getResidencyStatus() != GpuResidency::Resident)
		return false;

	if (!PATHS::FileExists(path))
	{
		LogO("PreviewTex: doesn't exist: "+path);
		Clear(b,g,r,a);
		return false;
	}
	// LogO("PreviewTex: "+sName+" load: "+path);

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
		#if 1
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
					// r = 255.f * PixelFormatGpuUtils::fromSRGB(r / 255.f);
					// g = 255.f * PixelFormatGpuUtils::fromSRGB(g / 255.f);
					// b = 255.f * PixelFormatGpuUtils::fromSRGB(b / 255.f);
					// powf( ( x + 0.055f ) / 1.055f, 2.4f ); ^
					r = 255.f * powf(r / 255.f, 1.9f);  //** par!
					g = 255.f * powf(g / 255.f, 1.9f);
					b = 255.f * powf(b / 255.f, 1.9f);
					buf[p] = r;  buf[p+1] = g;  buf[p+2] = b;
					p += 4;  // a
				}
			}
		#endif
			// LogO(String("::: Time prv sRGB: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  // 44 ms  6 Fps
				
			//LogO("PRV: "+toStr(img.getWidth())+" "+toStr(img.getHeight())+"  "+path);

			///----
			auto pf = PFG_RGBA8_UNORM_SRGB;
			// auto pf = PFG_RGBA8_UNORM;
			const TextureBox& imgData = img.getData(0);

			StagingTexture* tex = mgr->getStagingTexture(
				xSize, ySize, 1u, 1u, pf );
			tex->startMapRegion();
			TextureBox texBox = tex->mapRegion(
				xSize, ySize, 1u, 1u, pf );

			tex->stopMapRegion();
			if (imgData.width == texBox.width && texBox.data)
			try
			{
				texBox.copyFrom( imgData );  // upload  // todo debug crash?
				tex->upload( texBox, prvTex, 0, 0, 0 );
			}
			catch (Exception e)
			{
				LogO(String("PreviewTex: can't upload")/*+e.what()*/);
			}
			mgr->removeStagingTexture( tex );  tex = 0;

			prvTex->scheduleTransitionTo( GpuResidency::Resident );

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
		LogO(String("PreviewTex: can't upload")/*+e.what()*/);
	}
	
	mgr->removeStagingTexture( tex );  tex = 0;
	delete[] data;
}
