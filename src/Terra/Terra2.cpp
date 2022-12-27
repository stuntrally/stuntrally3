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

	//  🆕 Create Height
	//-----------------------------------------------------------------------------------
	void Terra::createHeightmapTexture(
		std::vector<float> hfHeight, int row)
	{
		destroyHeightmapTexture();

		TextureGpuManager *mgr =
			mManager->getDestinationRenderSystem()->getTextureGpuManager();
		m_heightUnormScaled = m_fHeightMul;

		PixelFormatGpu fmt = PFG_R32_FLOAT;

		// Many Android GPUs don't support PFG_R16_UNORM so we scale it by hand,  // nope

		m_heightMapTex = mgr->createTexture(
			"HeightMapTex" + StringConverter::toString( getId() ),
			GpuPageOutStrategy::SaveToSystemRam,
			TextureFlags::ManualTexture,
			TextureTypes::Type2D );
		m_heightMapTex->setResolution( m_iWidth, m_iHeight );
		m_heightMapTex->setPixelFormat( fmt );
		m_heightMapTex->scheduleTransitionTo( GpuResidency::Resident );


		//**  fill Hmap tex
		StagingTexture* tex = mgr->getStagingTexture(
			m_iWidth, m_iHeight, 1u, 1u, fmt );
		tex->startMapRegion();
		TextureBox texBox = tex->mapRegion(
			m_iWidth, m_iHeight, 1u, 1u, fmt );

		LogO("Ter Hmap dim " + iToStr(m_iWidth)+" x "+ iToStr(m_iHeight)+"  "+ iToStr(row));
		
		// simple, exact
		if (m_iHeight == row)
		{
			texBox.copyFrom( &hfHeight[0], m_iWidth, m_iHeight, row * m_iWidth * sizeof(float) );
		}else
		{   int a = 0;  // SR  Hmap  fix 1025 to 1024,  and flip
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

		mgr->removeStagingTexture( tex );
		tex = 0;
	}

	//  💫 Update Height
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

		mgr->removeStagingTexture( tex );
		tex = 0;
	}


	//**  🏔️ Blendmap  * * *
	//-----------------------------------------------------------------------------------
	void Terra::createBlendmap()
	{
		destroyBlendmap();

		TextureGpuManager *textureManager =
			mManager->getDestinationRenderSystem()->getTextureGpuManager();
		m_blendMapTex = textureManager->createTexture(
			"BlendMapTex_" + StringConverter::toString( getId() ), GpuPageOutStrategy::SaveToSystemRam,
			TextureFlags::RenderToTexture | TextureFlags::AllowAutomipmaps,
			TextureTypes::Type2DArray, "General" );
		
		m_blendMapTex->setResolution( m_heightMapTex->getWidth(), m_heightMapTex->getHeight() );
		m_blendMapTex->setNumMipmaps( PixelFormatGpuUtils::getMaxMipmapCount(
			m_blendMapTex->getWidth(), m_blendMapTex->getHeight() ) );

		m_blendMapTex->setPixelFormat( PFG_RGBA8_UNORM );
		m_blendMapTex->scheduleTransitionTo( GpuResidency::Resident );

		MaterialPtr blendMapperMat = MaterialManager::getSingleton().load(
			"Terra/GpuBlendMapper",
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).staticCast<Material>();
		Pass *pass = blendMapperMat->getTechnique(0)->getPass(0);
		TextureUnitState *texUnit = pass->getTextureUnitState(0);
		texUnit->setTexture( m_heightMapTex );
		texUnit = pass->getTextureUnitState(1);  // n
		texUnit->setTexture( m_normalMapTex );

		SetBlendmapParams(pass);

		Camera *dummyCamera = mManager->createCamera( "TerraDummyCamera2" );

		const IdString workspaceName = "Terra/GpuBlendMapperWorkspace";
		CompositorWorkspace *workspace = m_compositorManager->addWorkspace(
			mManager, m_blendMapTex/*finalTargetChannels*/, dummyCamera, workspaceName, false );
		workspace->_beginUpdate( true );
		workspace->_update();
		workspace->_endUpdate( true );

		m_compositorManager->removeWorkspace( workspace );
		mManager->destroyCamera( dummyCamera );

		// m_blendMapTex->writeContentsToFile("blendmapRTT.png", 0, 1);  //** ter test blendmap
	}


	//**  🏔️  Blendmap Params  * * *
	//-----------------------------------------------------------------------------------
	void Terra::SetBlendmapParams(Pass* pass)
	{
		// Normalize vScale for better precision in the shader math
		const Vector3 vScale =
			Vector3( m_xzRelativeSize.x, m_heightUnormScaled, m_xzRelativeSize.y ).normalisedCopy();

		GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();
		psParams->setNamedConstant( "heightMapResolution",
			Vector4( static_cast<Real>( m_iWidth ), static_cast<Real>( m_iHeight ), 1, 1 ) );
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
		
		int nl = std::min(4, (int)sc->td.layers.size());
		for (i=0; i < nl; ++i)
		{	//  range
			const TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
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
		Set1("terrainWorldSize", sc->td.fTerWorldSize);
	}

	//-----------------------------------------------------------------------------------
	void Terra::destroyBlendmap()
	{
		if( m_blendMapTex )
		{
			TextureGpuManager *textureManager =
					mManager->getDestinationRenderSystem()->getTextureGpuManager();
			textureManager->destroyTexture( m_blendMapTex );
			m_blendMapTex = 0;
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
