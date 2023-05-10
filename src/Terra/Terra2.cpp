//  Terra extra stuff by CryHam
#include "Terra.h"
#include "TerraShadowMapper.h"
#include "OgreHlmsTerra.h"

#include <OgreTextureGpu.h>
#include <OgreVector4.h>
#include <OgreImage2.h>

#include <Compositor/OgreCompositorChannel.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <OgreCamera.h>
#include <OgreDepthBuffer.h>
#include <OgreMaterialManager.h>
#include <OgrePixelFormatGpuUtils.h>
#include <OgreSceneManager.h>
#include <OgreStagingTexture.h>
#include <OgreTechnique.h>
#include <OgreTextureGpuManager.h>

#include "Def_Str.h"
#include "CScene.h"
#include "SceneXml.h"


namespace Ogre
{

	//  🆕 Create Heightmap once
	//-----------------------------------------------------------------------------------
	void Terra::createHmapTex(
		const std::vector<float>& hfHeight, int row)
	{
		destroyHeightmapTex();

		TextureGpuManager *mgr =
			mManager->getDestinationRenderSystem()->getTextureGpuManager();
		m_heightUnormScaled = m_fHeightMul;

		PixelFormatGpu fmt = PFG_R32_FLOAT;

		// Many Android GPUs don't support PFG_R16_UNORM so we scale it by hand,  // nope

		m_heightMapTex = mgr->createTexture(
			"HeightMapTex" + toStr( getId() ),
			GpuPageOutStrategy::SaveToSystemRam,
			TextureFlags::ManualTexture, TextureTypes::Type2D );
		m_heightMapTex->setResolution( m_iWidth, m_iHeight );
		m_heightMapTex->setPixelFormat( fmt );
		m_heightMapTex->scheduleTransitionTo( GpuResidency::Resident );


		//**  fill Hmap tex
		StagingTexture* tex = mgr->getStagingTexture(
			m_iWidth, m_iHeight, 1u, 1u, fmt );
		tex->startMapRegion();
		TextureBox texBox = tex->mapRegion(
			m_iWidth, m_iHeight, 1u, 1u, fmt );

		LogO("Terrain Hmap dim " + iToStr(m_iWidth)+" x "+ iToStr(m_iHeight)+"  "+ iToStr(row));
		
		// simple, exact

		{   int a = 0;  // SR  Hmap  fix 1025 to 1024  and flip Y !!
			if (bNormalized)
			{
				for (int y=0; y < m_iHeight; ++y)
				for (int x=0; x < m_iWidth; ++x)
				{
					float h = hfHeight[x + (m_iHeight-1-y) * row];
					m_heightMap[a++] = (h - fHMin) / fHRange;  //** ter norm scale
					// m_heightMap[a++] = 0.5f + 0.5f * sin(x*0.01) * cos(y*0.02);  // test
				}
			}else  // any floats
			{	for (int y=0; y < m_iHeight; ++y)
				for (int x=0; x < m_iWidth; ++x)
					m_heightMap[a++] = hfHeight[x + (m_iHeight-1-y) * row];
			}
			//  copy
			texBox.copyFrom( &m_heightMap[0], m_iWidth, m_iHeight, m_iWidth * sizeof(float) );
		}

		//  upload
		tex->stopMapRegion();
		tex->upload( texBox, m_heightMapTex, 0, 0, 0 );

		mgr->removeStagingTexture( tex );  tex = 0;
	}

	void Terra::readBackHmap(  // flips back Y !!
		std::vector<float>& hfHeight, int row)
	{
		int a=0;
		if (bNormalized)
		{
			for (int y=0; y < m_iHeight; ++y)
			for (int x=0; x < m_iWidth; ++x)
			{
				float h = m_heightMap[x + (m_iHeight-1-y) * row];
				hfHeight[a++] = h * fHRange + fHMin;  //** ter norm scale
			}
		}else  // any floats
		{
			for (int y=0; y < m_iHeight; ++y)
			for (int x=0; x < m_iWidth; ++x)
				hfHeight[a++] = m_heightMap[x + (m_iHeight-1-y) * row];
		}
	}

	//  💫 Update Heightmap, full, rect ignored
	//-----------------------------------------------------------------------------------
	void Terra::dirtyRect(Rect rect)
	{
		TextureGpuManager *mgr =
			mManager->getDestinationRenderSystem()->getTextureGpuManager();
		PixelFormatGpu fmt = PFG_R32_FLOAT;

		StagingTexture* tex = mgr->getStagingTexture(
			m_iWidth, m_iHeight, 1u, 1u, fmt );
		tex->startMapRegion();
		TextureBox texBox = tex->mapRegion(
			m_iWidth, m_iHeight, 1u, 1u, fmt );

		//  copy
		texBox.copyFrom( &m_heightMap[0], m_iWidth, m_iHeight, m_iWidth * sizeof(float) );

		//  upload
		tex->stopMapRegion();
		tex->upload( texBox, m_heightMapTex, 0, 0, 0 );

		mgr->removeStagingTexture( tex );  tex = 0;

		normalmap.Update();
	}


	//  🆕🏔️ Create Blendmap  * * *
	//-----------------------------------------------------------------------------------
	Terra::Normalmap::Normalmap(Terra* terra)
		: pTerra(terra)
	{	}
	Terra::Blendmap::Blendmap(Terra* terra)
		: pTerra(terra)
	{	}

	void Terra::Blendmap::Create()
	{
		Destroy();

		TextureGpuManager *mgr = pTerra->mManager->getDestinationRenderSystem()->getTextureGpuManager();
		texture = mgr->createTexture(
			// "BlendMapTex_" + toStr( pTerra->getId() ),
			"BlendMapTex_" + toStr(pTerra->cnt),
			GpuPageOutStrategy::SaveToSystemRam,
			TextureFlags::RenderToTexture | TextureFlags::AllowAutomipmaps,
			TextureTypes::Type2DArray, "General" );
		
		texture->setResolution( pTerra->m_heightMapTex->getWidth(), pTerra->m_heightMapTex->getHeight() );
		texture->setNumMipmaps( PixelFormatGpuUtils::getMaxMipmapCount(
			texture->getWidth(), texture->getHeight() ) );

		texture->setPixelFormat( PFG_RGBA8_UNORM );
		texture->scheduleTransitionTo( GpuResidency::Resident );

		LogO("TER ble ws new:" + toStr(pTerra->cnt));
		MaterialPtr blendMat = MaterialManager::getSingleton().load(
			// "Terra/GpuBlendMapper",
			"Terra/GpuBlendMapper"+toStr(pTerra->cnt),
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).staticCast<Material>();
		// MaterialPtr blendMat2 = blendMat->clone("MtrBle" + toStr(pTerra->cnt));
		// pass = blendMat2->getTechnique(0)->getPass(0);  // no-
		pass = blendMat->getTechnique(0)->getPass(0);
		
		TextureUnitState *texUnit = pass->getTextureUnitState(0);
		texUnit->setTexture( pTerra->m_heightMapTex );
		texUnit = pass->getTextureUnitState(1);  // n
		texUnit->setTexture( pTerra->normalmap.texture );

		SetParams();

		camera = pTerra->mManager->createCamera( "CamTerraBlend" + toStr(pTerra->cnt) );

		// const IdString workspaceName = "Terra/GpuBlendMapperWorkspace";
		const IdString workspaceName = "Terra/GpuBlendMapperWorkspace"+toStr(pTerra->cnt);
		workspace = pTerra->m_compositorManager->addWorkspace(
			pTerra->mManager, texture/*finalTargetChannels*/, camera, workspaceName, false );

		workspace->_beginUpdate(true);  //+?
		workspace->_update();
		workspace->_endUpdate(true);

		#ifndef SR_EDITOR  // todo: game no upd, destroy rtt
		//	pTerra->m_compositorManager->removeWorkspace( workspace );
		//	pTerra->mManager->destroyCamera( camera );
		#endif

		//^^ todo:  for GetTerMtrIds  tire ter surf ..
		// texture->writeContentsToFile("blendmapRTT.png", 0, 1);  //** ter test blendmap
	}

	//  🏔️💫  Update Blendmap (render tex)
	//-----------------------------------------------------------------------------------
	void Terra::Blendmap::Update()
	{
		if (!workspace)  return;
		SetParams();
		workspace->_beginUpdate(true);
		workspace->_update();
		workspace->_endUpdate(true);
		SetParams();
	}

	//  🏔️💫📄  Update Blendmap Params  * * *
	//-----------------------------------------------------------------------------------
	void Terra::Blendmap::SetParams()
	{
		// Normalize vScale for better precision in the shader math
		const Vector3 vScale =
			Vector3( pTerra->m_xzRelativeSize.x, pTerra->m_heightUnormScaled,
					 pTerra->m_xzRelativeSize.y ).normalisedCopy();

		GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();
		psParams->setNamedConstant( "heightMapResolution",
			Vector4( static_cast<Real>( pTerra->m_iWidth ),
					 static_cast<Real>( pTerra->m_iHeight ), 1, 1 ) );
		psParams->setNamedConstant( "vScale", vScale );
		// psParams->setNamedConstant( "vLayers", Vector4(1.f, 1.f, 1.f, 0.f) );

		int i;
		float Hmin[4],Hmax[4],Hsmt[4], Amin[4],Amax[4],Asmt[4];
		float Nnext[4],Nprev[3],Nnext2[2], Nonly[4];
		float Nfreq[3],Noct[3],Npers[3],Npow[3];
		float Nfreq2[2],Noct2[2],Npers2[2],Npow2[2];
		//  zero
		for (i=0; i < 4; ++i)
		{	Hmin[i]=0.f; Hmax[i]=0.f; Hsmt[i]=0.f;  Amin[i]=0.f; Amax[i]=0.f; Asmt[i]=0.f;
			Nnext[i]=0.f;  Nonly[i]=0.f;  }
		for (i=0; i < 3; ++i)
		{	Nprev[i]=0.f;  Nfreq[i]=0.f; Noct[i]=0.f; Npers[i]=0.f; Npow[i]=0.f;  }
		for (i=0; i < 2; ++i)
		{	Nnext2[i]=0.f;  Nfreq2[i]=0.f; Noct2[i]=0.f; Npers2[i]=0.f; Npow2[i]=0.f;  }
		
		const auto& td = pTerra->sc->tds[pTerra->cnt];
		int nl = std::min(4, (int)td.layers.size());
		for (i=0; i < nl; ++i)
		{	//  range
			const TerLayer& l = td.layersAll[td.layers[i]];
			Hmin[i] = l.hMin;	Hmax[i] = l.hMax;	Hsmt[i] = l.hSm;
			Amin[i] = l.angMin;	Amax[i] = l.angMax;	Asmt[i] = l.angSm;
			//  noise
			Nonly[i] = !l.nOnly ? 1.f : 0.f;
			Nnext[i] = i < nl-1 ? l.noise : 0.f;  // dont +1 last
			if (i > 0)  Nprev[i-1] = l.nprev;  // dont -1 first
			if (i < 2)  Nnext2[i] = nl > 2 ? l.nnext2 : 0.f;
			//  n par +1,-1, +2
			if (i < nl-1){  Nfreq[i] = l.nFreq[0];  Noct[i] = l.nOct[0];  Npers[i] = l.nPers[0];  Npow[i] = l.nPow[0];  }
			if (i < nl-2){  Nfreq2[i]= l.nFreq[1];  Noct2[i]= l.nOct[1];  Npers2[i]= l.nPers[1];  Npow2[i]= l.nPow[1];  }
		}
		#define Set4(s,v)  psParams->setNamedConstant( s, Vector4(v[0], v[1], v[2], v[3]) )
		#define Set3(s,v)  psParams->setNamedConstant( s, Vector3(v[0], v[1], v[2]) )
		#define Set2(s,v)  psParams->setNamedConstant( s, Vector2(v[0], v[1]) )
		#define Set1(s,v)  psParams->setNamedConstant( s, v )

		Set4("Hmin", Hmin);  Set4("Hmax", Hmax);  Set4("Hsmt", Hsmt);
		Set4("Amin", Amin);  Set4("Amax", Amax);  Set4("Asmt", Asmt);  Set4("Nonly", Nonly);
		Set3("Nnext", Nnext);  Set3("Nprev", Nprev);  Set2("Nnext2", Nnext2);
		Set3("Nfreq", Nfreq);  Set3("Noct", Noct);  Set3("Npers", Npers);  Set3("Npow", Npow);
		Set2("Nfreq2", Nfreq2);  Set2("Noct2", Noct2);  Set2("Npers2", Npers2);  Set2("Npow2", Npow2);
		Set1("terrainWorldSize", td.fTerWorldSize);
		// psParams.reset();  //?
	}

	//  🏔️💥  Destroy Blendmap
	//-----------------------------------------------------------------------------------
	void Terra::Blendmap::Destroy()
	{
		if (workspace)
		{	pTerra->m_compositorManager->removeWorkspace( workspace );
			workspace = 0;
		}
		if (camera)
		{	pTerra->mManager->destroyCamera( camera );
			camera = 0;
		}
		pass = 0;

		if (texture)
		{	TextureGpuManager *mgr =
				pTerra->mManager->getDestinationRenderSystem()->getTextureGpuManager();
			mgr->destroyTexture( texture );
			texture = 0;
		}
	}


	//  ⛓️ new utils
	//-----------------------------------------------------------------------------------
	Real Terra::getAngle(Real x, Real z, Real s) const
	{
		Real y0 = 0.f;
		Vector3 vx(x-s, y0, z), vz(x, y0, z-s);
		Vector3 vX(x+s, y0, z), vZ(x, y0, z+s);
		vx.y = getHeight(vx.x, vx.z);  vX.y = getHeight(vX.x, vX.z);
		vz.y = getHeight(vz.x, vz.z);  vZ.y = getHeight(vZ.x, vZ.z);
		Vector3 v_x = vx-vX;  //v_x.normalise();
		Vector3 v_z = vz-vZ;  //v_z.normalise();
		Vector3 n = -v_x.crossProduct(v_z);  n.normalise();
		Real a = Math::ACos(n.y).valueDegrees();
		return a;
	}

	Real Terra::getHeight( Real x, Real z ) const
	{
		Vector3 pos(x, 0, z);
		getHeightAt(pos);
		return pos.y;
	}

}
