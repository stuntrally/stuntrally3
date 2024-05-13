#include "pch.h"
#include "AppGui.h"
#include "settings.h"
#include "HlmsPbs2.h"

#include <OgreCommon.h>
#include <OgreRoot.h>
#include <OgreItem.h>
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsUnlitDatablock.h>
#include <OgreHlmsManager.h>
#include <SDL_events.h>
using namespace Ogre;
using namespace std;


//  ⛓️ util  🌐 Wireframe
//----------------------------------------------------------------
void AppGui::SetWireframe()
{
	bool b = bWireframe;
	
	SetWireframe( HLMS_PBS, b );  // 3d all
	// SetWireframe( HLMS_UNLIT, b );  // 2d particles, Gui
	SetWireframe( HLMS_USER3, b );  // terrain
}

void AppGui::SetWireframe(HlmsTypes type, bool wire)
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


//  ⛓️ util  wrap Texture filtering
//-----------------------------------------------------------------------------------
void AppGui::SetTexWrap(HlmsTypes type, String name, bool wrap)
{
	HlmsSamplerblock sb;
	InitTexFiltUV(&sb, wrap);

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

void AppGui::SetTexWrap(Item* it, bool wrap)
{
	SetAnisotropy(it);

return;  //! **  wrap in .json, no need-
	HlmsSamplerblock sb;
	InitTexFiltUV(&sb, wrap);

	assert( dynamic_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() ) );
	HlmsPbsDatablock *db = static_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() );
	
	for (int n=0; n < NUM_PBSM_SOURCES; ++n)  // all
		db->setSamplerblock( n, sb );
}

void AppGui::SetAnisotropy(Item* it)
{
	assert( dynamic_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() ) );
	HlmsPbsDatablock *db = static_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() );
	
	for (int n=0; n < NUM_PBSM_SOURCES; ++n)  // all
	{
		const HlmsSamplerblock *s = db->getSamplerblock( n );
		if (s)
		{
			HlmsSamplerblock sb( *s );
			InitTexFilt(&sb);
			db->setSamplerblock( n, sb );
		}
	}
}

void AppGui::SetAnisotropy()
{
	return;  //! ** drops road UV wrap on load!

	Hlms *hlms = mRoot->getHlmsManager()->getHlms( HLMS_PBS );
	const auto& dbs = hlms->getDatablockMap();
	for (auto it = dbs.begin(); it != dbs.end(); ++it)
	{
		auto* db = static_cast< HlmsPbsDatablock *>(it->second.datablock);
		// if (db)
		for (int n=0; n < NUM_PBSM_SOURCES; ++n)  // all
		{
			const HlmsSamplerblock *s = db->getSamplerblock( n );
			if (s)
			{
				HlmsSamplerblock sb( *s );
				InitTexFilt(&sb);
				db->setSamplerblock( n, sb );
			}
		}
	}
}

void AppGui::InitTexFilt(HlmsSamplerblock* sb)
{
	FilterOptions mia, mip;
	switch (pSet->g.tex_filt)
	{
	case 3:  mia = FO_ANISOTROPIC;  mip = FO_ANISOTROPIC;  break;  // full anisotropic
	case 2:  mia = FO_ANISOTROPIC;  mip = FO_LINEAR;  break;  // anisotropic
	case 1:  mia = FO_LINEAR;  mip = FO_LINEAR;  break;  // trilinear
	case 0:  mia = FO_LINEAR;  mip = FO_POINT;  break;  // bilinear
	}
	sb->mMinFilter = mia;  sb->mMagFilter = mia;
	sb->mMipFilter = mip;

	sb->mMaxAnisotropy = pSet->g.anisotropy;
}

void AppGui::InitTexFiltUV(HlmsSamplerblock* sb, bool wrap)
{
	InitTexFilt(sb);

	auto w = wrap ? TAM_WRAP : TAM_CLAMP;
	sb->mU = w;  sb->mV = w;  sb->mW = w;
}

//  ed selected
void AppGui::UpdSelectGlow(Renderable *rend, bool selected)
{
	rend->setCustomParameter(HlmsPbs2::selected_glow, Vector4(selected ? 1 : 0, 0,0,0));

	HlmsDatablock *datablock = rend->getDatablock();
	Hlms *hlms = datablock->getCreator();
	if( hlms->getType() == HLMS_PBS )
	{
		assert( dynamic_cast<HlmsPbs2*>( hlms ) );
		HlmsPbs2 *myHlmsPbs = static_cast<HlmsPbs2*>( hlms );
		uint32 hash, casterHash;
		myHlmsPbs->CalculateHashFor( rend, hash, casterHash );
		rend->_setHlmsHashes( hash, casterHash );
	}
}

//  fix rtt for vulkan
void AppGui::BarrierResolve(TextureGpu* tex)
{
	RenderSystem *rs = mSceneMgr->getDestinationRenderSystem();
	rs->endCopyEncoder();
	BarrierSolver &barrierSolver = rs->getBarrierSolver();
	ResourceTransitionArray resTrans;
	barrierSolver.resolveTransition( resTrans, tex, ResourceLayout::Texture,
									ResourceAccess::Read, 1u << GPT_FRAGMENT_PROGRAM );
	rs->executeResourceTransition( resTrans );
}


//  Gui Input utils
//-----------------------------------------------------------------------------------
std::vector<unsigned long> AppGui::utf8ToUnicode(const string& utf8)
{
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size())
	{
		unsigned long uni;  size_t todo;
		unsigned char ch = utf8[i++];

		     if (ch <= 0x7F){	uni = ch;	todo = 0;	}
		else if (ch <= 0xBF){	throw logic_error("not a UTF-8 string");	}
		else if (ch <= 0xDF){	uni = ch&0x1F;	todo = 1;	}
		else if (ch <= 0xEF){	uni = ch&0x0F;	todo = 2;	}
		else if (ch <= 0xF7){	uni = ch&0x07;	todo = 3;	}
		else				{	throw logic_error("not a UTF-8 string");	}

		for (size_t j = 0; j < todo; ++j)
		{
			if (i == utf8.size())	throw logic_error("not a UTF-8 string");
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)  throw logic_error("not a UTF-8 string");
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)  throw logic_error("not a UTF-8 string");
		if (uni > 0x10FFFF)  throw logic_error("not a UTF-8 string");
		unicode.push_back(uni);
	}
	return unicode;
}

MyGUI::MouseButton AppGui::sdlButtonToMyGUI(/*Uint8*/int button)
{
	switch (button)
	{
	case SDL_BUTTON_LEFT:   return MyGUI::MouseButton::Left;
	case SDL_BUTTON_RIGHT:  return MyGUI::MouseButton::Right;
	case SDL_BUTTON_MIDDLE: return MyGUI::MouseButton::Middle;
	default:  return MyGUI::MouseButton::Left;
	}
}

//  gui key utils
//-----------------------------------------------------------------------------------
MyGUI::KeyCode AppGui::SDL2toGUIKey(SDL_Keycode code)
{
	MyGUI::KeyCode kc = MyGUI::KeyCode::None;

	auto key = mKeyMap.find(code);
	if (key != mKeyMap.end())
		kc = key->second;

	return kc;
}

void AppGui::SetupKeysForGUI()
{
	mKeyMap.clear();

	mKeyMap[SDLK_HOME] = MyGUI::KeyCode::Home;
	mKeyMap[SDLK_END] = MyGUI::KeyCode::End;
	mKeyMap[SDLK_PAGEUP] = MyGUI::KeyCode::PageUp;
	mKeyMap[SDLK_PAGEDOWN] = MyGUI::KeyCode::PageDown;
	mKeyMap[SDLK_UP] = MyGUI::KeyCode::ArrowUp;
	mKeyMap[SDLK_DOWN] = MyGUI::KeyCode::ArrowDown;
	mKeyMap[SDLK_LEFT] = MyGUI::KeyCode::ArrowLeft;
	mKeyMap[SDLK_RIGHT] = MyGUI::KeyCode::ArrowRight;

	mKeyMap[SDLK_DELETE] = MyGUI::KeyCode::Delete;
    mKeyMap[SDLK_BACKSPACE] = MyGUI::KeyCode::Backspace;
	mKeyMap[SDLK_SPACE] = MyGUI::KeyCode::Space;
    mKeyMap[SDLK_ESCAPE] = MyGUI::KeyCode::Escape;
	mKeyMap[SDLK_INSERT] = MyGUI::KeyCode::Insert;
	mKeyMap[SDLK_RETURN] = MyGUI::KeyCode::Return;
	// mKeyMap[SDLK_KP_0] = MyGUI::KeyCode::Numpad0;
	// mKeyMap[SDLK_KP_ENTER] = MyGUI::KeyCode::NumpadEnter;

	// Don't need more, rest comes in textInput
}
