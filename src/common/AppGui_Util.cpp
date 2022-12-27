#include "pch.h"
#include "AppGui.h"
#include "settings.h"

#include <OgreRoot.h>
#include <OgreItem.h>
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsUnlitDatablock.h>
#include <OgreHlmsManager.h>
// #include <OgreGpuProgramManager.h>
// #include <OgreTextureGpuManager.h>
// #include <OgrePixelFormatGpuUtils.h>
// #include <Vao/OgreVaoManager.h>
using namespace Ogre;


//  ⛓️ util  wireframe
//----------------------------------------------------------------
void AppGui::SetWireframe()
{
	bool b = bWireframe;
	
	SetWireframe( HLMS_PBS, b );  // 3d all
	// SetWireframe( HLMS_UNLIT, b );  // 2d particles, Gui
	SetWireframe( HLMS_USER3, b );  // terrain
}

void AppGui::SetWireframe(Ogre::HlmsTypes type, bool wire)
{
	HlmsMacroblock mb;
	mb.mPolygonMode = wire ? PM_WIREFRAME : PM_SOLID;

	Hlms *hlms = mRoot->getHlmsManager()->getHlms( type );

	const auto &dbs = hlms->getDatablockMap();
	for (auto it = dbs.begin(); it != dbs.end(); ++it)
	{
		// String name = itor->second.name;
		// if( !StringUtil::endsWith(name, "_TrueTypeFont", false))  // Gui font
		auto db = it->second.datablock;
		if (db)  db->setMacroblock( mb );
	}
}


//  ⛓️ util  wrap texture
//-----------------------------------------------------------------------------------
void AppGui::SetTexWrap(Ogre::HlmsTypes type, Ogre::String name, bool wrap)
{
	HlmsSamplerblock sb;
	sb.mMinFilter = FO_ANISOTROPIC;  sb.mMagFilter = FO_ANISOTROPIC;
	sb.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sb.mMaxAnisotropy = pSet->anisotropy;
	auto w = wrap ? TAM_WRAP : TAM_CLAMP;
	sb.mU = w;  sb.mV = w;  sb.mW = w;

	Hlms *hlms = mRoot->getHlmsManager()->getHlms( type );
	if (type == HLMS_PBS)
	{	HlmsPbsDatablock *db = static_cast<HlmsPbsDatablock*>(hlms->getDatablock( name ));
		if (db)  db->setSamplerblock( PBSM_DIFFUSE, sb );
	}
	else if (type == HLMS_UNLIT)
	{	HlmsUnlitDatablock *db = static_cast<HlmsUnlitDatablock*>(hlms->getDatablock( name ));
		if (db)  db->setSamplerblock( PBSM_DIFFUSE, sb );
	}
}

void AppGui::SetTexWrap(Ogre::Item* it, bool wrap)
{
	HlmsSamplerblock sb;
	sb.mMinFilter = FO_ANISOTROPIC;  sb.mMagFilter = FO_ANISOTROPIC;
	sb.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sb.mMaxAnisotropy = pSet->anisotropy;
	auto w = wrap ? TAM_WRAP : TAM_CLAMP;
	sb.mU = w;  sb.mV = w;  sb.mW = w;

	assert( dynamic_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() ) );
	HlmsPbsDatablock *db =
		static_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() );
	
	for (int n=0; n < NUM_PBSM_SOURCES; ++n)  // all
		db->setSamplerblock( PBSM_DIFFUSE + n, sb );
}
