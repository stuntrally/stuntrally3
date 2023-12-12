#include "pch.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "settings.h"
#include "game.h"  // for surfaces map
#include <OgreSceneNode.h>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace std;
using namespace Ogre;


//  Save
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::SaveXml(String file)
{
	XMLDocument xml;
	XMLElement* root = xml.NewElement("scene");

	XMLElement* ver = xml.NewElement("ver");
		int v = SET_VER;
		ver->SetAttribute("num",     toStrC( v ));
		ver->SetAttribute("baseTrk", baseTrk.c_str());
		ver->SetAttribute("secEd",   toStrC( secEdited ));
	root->InsertEndChild(ver);

	XMLElement* car = xml.NewElement("car");  // 🚗 game
		car->SetAttribute("tires",	asphalt ? "1":"0");
		if (damageMul != 1.f)
			car->SetAttribute("damage",	toStrC( damageMul ));
		if (!road1mtr)
			car->SetAttribute("road1mtr", road1mtr ? "1":"0");
		if (noWrongChks)
			car->SetAttribute("noWrongChks", noWrongChks ? "1":"0");
		if (needLights)
			car->SetAttribute("needLights", needLights ? "1":"0");

		if (denyReversed)
			car->SetAttribute("denyRev", "1");
		if (gravity != 9.81f)
			car->SetAttribute("gravity", toStrC( gravity ));
	root->InsertEndChild(car);


	XMLElement* st = xml.NewElement("start");  // 🏁
		string s = toStr(startPos[0][0])+" "+toStr(startPos[0][1])+" "+toStr(startPos[0][2]);
		st->SetAttribute("pos",	s.c_str());

		s = toStr(startRot[0][0])+" "+toStr(startRot[0][1])+" "+toStr(startRot[0][2])+" "+toStr(startRot[0][3]);
		st->SetAttribute("rot",	s.c_str());

		s = toStr(startPos[1][0])+" "+toStr(startPos[1][1])+" "+toStr(startPos[1][2]);
		st->SetAttribute("pos2",	s.c_str());

		s = toStr(startRot[1][0])+" "+toStr(startRot[1][1])+" "+toStr(startRot[1][2])+" "+toStr(startRot[1][3]);
		st->SetAttribute("rot2",	s.c_str());
	root->InsertEndChild(st);


	XMLElement* snd = xml.NewElement("sound");  // 🔉
		snd->SetAttribute("ambient",		ambientSnd.c_str());
		if (ambientVol != 1.f)
			snd->SetAttribute("vol",		toStrC( ambientVol ));
		snd->SetAttribute("reverbs",		sReverbs.c_str());
	root->InsertEndChild(snd);
	

	XMLElement* sky = xml.NewElement("sky");  // ⛅
		sky->SetAttribute("material",	skyMtr.c_str());
		if (rainEmit > 0 && rainName != "")
		{	sky->SetAttribute("rainName",	rainName.c_str());
			sky->SetAttribute("rainEmit",	toStrC( rainEmit ));
		}
		if (rain2Emit > 0 && rain2Name != "")
		{	sky->SetAttribute("rain2Name",	rain2Name.c_str());
			sky->SetAttribute("rain2Emit",	toStrC( rain2Emit ));
		}
		if (windAmt != 0.f)
			sky->SetAttribute("windAmt",	toStrC( windAmt ));
		if (skyYaw != 0.f)
			sky->SetAttribute("skyYaw",	toStrC( skyYaw ));
	root->InsertEndChild(sky);

	XMLElement* fog = xml.NewElement("fog");  // 🌫️
		fog->SetAttribute("color",		fogClr.Save().c_str() );
		fog->SetAttribute("color2",		fogClr2.Save().c_str() );
		fog->SetAttribute("linStart",	toStrC( fogStart ));
		fog->SetAttribute("linEnd",		toStrC( fogEnd ));
	root->InsertEndChild(fog);

	XMLElement* fogH = xml.NewElement("fogH");  // 🌫️
		fogH->SetAttribute("color",		fogClrH.Save().c_str() );
		fogH->SetAttribute("height",	toStrC( fogHeight ));
		fogH->SetAttribute("dens",		toStrC( fogHDensity ));
		fogH->SetAttribute("linStart",	toStrC( fogHStart ));
		fogH->SetAttribute("linEnd",	toStrC( fogHEnd ));
		if (fHDamage > 0.f)
			fogH->SetAttribute("dmg",	toStrC( fHDamage ));
	root->InsertEndChild(fogH);

	XMLElement* li = xml.NewElement("light");  // 💡
		li->SetAttribute("pitch",		toStrC( ldPitch ));
		li->SetAttribute("yaw",			toStrC( ldYaw ));
		li->SetAttribute("ambient",		lAmb.Save().c_str() );
		li->SetAttribute("diffuse",		lDiff.Save().c_str() );
		li->SetAttribute("specular",	lSpec.Save().c_str() );
	root->InsertEndChild(li);
	

	XMLElement* fls = xml.NewElement("fluids");  // 💧
		for (int i=0; i < fluids.size(); ++i)
		{
			const FluidBox* fb = &fluids[i];
			XMLElement* fe = xml.NewElement("fluid");
			fe->SetAttribute("hq",		toStrC( fb->hq ));
			fe->SetAttribute("name",	fb->name.c_str() );
			fe->SetAttribute("pos",		toStrC( fb->pos ));
			fe->SetAttribute("rot",		toStrC( fb->rot ));
			fe->SetAttribute("size",	toStrC( fb->size ));
			fe->SetAttribute("tile",	toStrC( fb->tile ));
			fls->InsertEndChild(fe);
		}
	root->InsertEndChild(fls);


	XMLElement* ters = xml.NewElement("terrains");  // ⛰️
	for (int n = 0; n < tds.size(); ++n)
	{
		const auto& td = tds[n];
		XMLElement* ter = xml.NewElement("terrain");
			// ter.SetAttribute("size",		toStrC( td.iVertsX ));  // no, from filesize
			ter->SetAttribute("ofsZ",		toStrC( td.ofsZ ));
			ter->SetAttribute("triangle",	toStrC( td.fTriangleSize ));
			if (td.posX != 0.f)  ter->SetAttribute("posX",	toStrC( td.posX ));
			if (td.posZ != 0.f)  ter->SetAttribute("posZ",	toStrC( td.posZ ));

			ter->SetAttribute("diff",	toStrC( td.clrDiff ));
			if (td.emissive)
				ter->SetAttribute("emissive",	td.emissive ? 1 : 0);
			if (td.reflect)
				ter->SetAttribute("reflect",	td.reflect);
			// if (td.specularPow != 32.f)
			// 	ter->SetAttribute("specPow",	toStrC( td.specularPow ));
			if (td.specularPowEm != 2.f)
				ter->SetAttribute("specPowEm",	toStrC( td.specularPowEm ));

			if (td.iHorizon)	ter->SetAttribute("horiz",	toStrC( td.iHorizon ));
			if (!td.collis)		ter->SetAttribute("collis",	td.collis ? 1 : 0 );
			if (!td.bL)		ter->SetAttribute("bL",	td.bL ? 1 : 0 );
			if (!td.bR)		ter->SetAttribute("bR",	td.bR ? 1 : 0 );
			if (!td.bF)		ter->SetAttribute("bF",	td.bF ? 1 : 0 );
			if (!td.bB)		ter->SetAttribute("bB",	td.bB ? 1 : 0 );

			const TerLayer* l;
			for (int i=0; i < 6; ++i)
			{
				l = &td.layersAll[i];
				XMLElement* tex = xml.NewElement("texture");
				tex->SetAttribute("on",		l->on ? 1 : 0);
				tex->SetAttribute("file",	l->texFile.c_str());
				tex->SetAttribute("fnorm",	l->texNorm.c_str());
				tex->SetAttribute("scale",	toStrC( l->tiling ));
				tex->SetAttribute("surf",	l->surfName.c_str());
				#define setDmst()  \
					tex->SetAttribute("dust",	toStrC( l->dust ));  \
					tex->SetAttribute("dustS",	toStrC( l->dustS )); \
					tex->SetAttribute("mud",	toStrC( l->mud ));   \
					tex->SetAttribute("smoke",	toStrC( l->smoke )); \
					tex->SetAttribute("tclr",	l->tclr.Save().c_str() );
				setDmst();  // ⚫💭
				if (l->fDamage > 0.f)
					tex->SetAttribute("dmg",	toStrC( l->fDamage ));

				tex->SetAttribute("angMin",	toStrC( l->angMin ));
				tex->SetAttribute("angMax",	toStrC( l->angMax ));
				tex->SetAttribute("angSm",	toStrC( l->angSm ));
				tex->SetAttribute("hMin",	toStrC( l->hMin ));
				tex->SetAttribute("hMax",	toStrC( l->hMax ));
				tex->SetAttribute("hSm",	toStrC( l->hSm ));

				tex->SetAttribute("nOn",	l->nOnly ? 1 : 0);
				if (l->triplanar)  tex->SetAttribute("triplanar", 1);

				tex->SetAttribute("noise",	toStrC( l->noise ));
				tex->SetAttribute("n_1",	toStrC( l->nprev ));
				tex->SetAttribute("n2",		toStrC( l->nnext2 ));

				XMLElement* noi = xml.NewElement("noise");
				for (int n=0; n < 2; ++n)
				{	string sn = toStr(n), s;
					s = "frq"+sn;  noi->SetAttribute(s.c_str(),  toStrC( l->nFreq[n] ));
					s = "oct"+sn;  noi->SetAttribute(s.c_str(),  toStrC( l->nOct[n] ));
					s = "prs"+sn;  noi->SetAttribute(s.c_str(),  toStrC( l->nPers[n] ));
					s = "pow"+sn;  noi->SetAttribute(s.c_str(),  toStrC( l->nPow[n] ));
				}
				tex->InsertEndChild(noi);
				ter->InsertEndChild(tex);
			}
			for (int i=0; i < 4; ++i)  // not here
			{
				l = &layerRoad[i];
				XMLElement* tex = xml.NewElement("texture");
				tex->SetAttribute("road",	toStrC(i+1));
				tex->SetAttribute("surf",	l->surfName.c_str());
				setDmst();
				ter->InsertEndChild(tex);
			}
			
			XMLElement* par = xml.NewElement("par");  // ⚫💭 surface par
				par->SetAttribute("dust",	sParDust.c_str());
				par->SetAttribute("mud",	sParMud.c_str());
				par->SetAttribute("smoke",	sParSmoke.c_str());
				//  fluids if not default
				if (sFluidWater   != "FluidWater")    par->SetAttribute("flWater",  sFluidWater.c_str());
				if (sFluidMud     != "FluidMud")      par->SetAttribute("flMud",  sFluidMud.c_str());
				if (sFluidMudSoft != "FluidMudSoft")  par->SetAttribute("flMudSoft",  sFluidMudSoft.c_str());
			ter->InsertEndChild(par);

		ters->InsertEndChild(ter);
	}
	root->InsertEndChild(ters);
	

	XMLElement* pgd = xml.NewElement("veget");  // 🌳🪨 vegetation
		pgd->SetAttribute("densGrass",   toStrC( densGrass ));
		pgd->SetAttribute("densTrees",   toStrC( densTrees ));
		//  grass
		pgd->SetAttribute("grPage",      toStrC( grPage ));
		pgd->SetAttribute("grDist",      toStrC( grDist ));
		pgd->SetAttribute("grDensSmooth",toStrC( grDensSmooth ));

		//  trees
		pgd->SetAttribute("trPage",		toStrC( trPage ));
		pgd->SetAttribute("trDist",		toStrC( trDist ));
		pgd->SetAttribute("trDistImp",	toStrC( trDistImp ));
		pgd->SetAttribute("trRdDist",	toStrC( trRdDist  ));

		int i;
		for (int i=0; i < ciNumGrLay; ++i)  // 🌿 grass
		{
			const SGrassLayer& g = grLayersAll[i];
			XMLElement* grl = xml.NewElement("grass");
			grl->SetAttribute("on",		g.on ? 1 : 0);
			grl->SetAttribute("mtr",	g.material.c_str());
			// grl->SetAttribute("clr",	g.colorMap.c_str());  // old drop-
			auto c = g.color;
			if (c.h != 0.f || c.s != 0.f || c.v != 1.f || c.a != 1.f || c.n != 0.f)
				grl->SetAttribute("cl", c.Save().c_str() );  // new
			grl->SetAttribute("dens",	toStrC( g.dens ));
			grl->SetAttribute("chan",	toStrC( g.iChan ));

			grl->SetAttribute("minSx",	toStrC( g.minSx ));
			grl->SetAttribute("maxSx",	toStrC( g.maxSx ));
			grl->SetAttribute("minSy",	toStrC( g.minSy ));
			grl->SetAttribute("maxSy",	toStrC( g.maxSy ));

			grl->SetAttribute("swayDistr",	toStrC( g.swayDistr ));
			grl->SetAttribute("swayLen",	toStrC( g.swayLen ));
			grl->SetAttribute("swaySpeed",	toStrC( g.swaySpeed ));
			pgd->InsertEndChild(grl);
		}

		for (i=0; i < 4; ++i)
		{
			const SGrassChannel& g = grChan[i];
			XMLElement* gch = xml.NewElement("gchan");
			gch->SetAttribute("amin",	toStrC( g.angMin ));
			gch->SetAttribute("amax",	toStrC( g.angMax ));
			gch->SetAttribute("asm",	toStrC( g.angSm ));

			gch->SetAttribute("hmin",	toStrC( g.hMin ));
			gch->SetAttribute("hmax",	toStrC( g.hMax ));
			gch->SetAttribute("hsm",	toStrC( g.hSm ));

			gch->SetAttribute("ns",		toStrC( g.noise ));
			gch->SetAttribute("frq",	toStrC( g.nFreq ));
			gch->SetAttribute("oct",	toStrC( g.nOct ));
			gch->SetAttribute("prs",	toStrC( g.nPers ));
			gch->SetAttribute("pow",	toStrC( g.nPow ));

			gch->SetAttribute("rd",		toStrC( g.rdPow ));
			pgd->InsertEndChild(gch);
		}

		for (i=0; i < ciNumVegLay; ++i)  // 🌳🪨 models
		{
			const VegetLayer& l = vegLayersAll[i];
			XMLElement* vgl = xml.NewElement("layer");
			vgl->SetAttribute("on",			l.on ? 1 : 0);
			vgl->SetAttribute("name",		l.name.c_str());
			vgl->SetAttribute("dens",		toStrC( l.dens ));
			vgl->SetAttribute("minScale",	toStrC( l.minScale ));
			vgl->SetAttribute("maxScale",	toStrC( l.maxScale ));

			vgl->SetAttribute("ofsY",		toStrC( l.ofsY ));
			vgl->SetAttribute("addTrRdDist",toStrC( l.addRdist ));
			vgl->SetAttribute("maxRdist",	toStrC( l.maxRdist ));
			vgl->SetAttribute("windFx",		toStrC( l.windFx ));
			vgl->SetAttribute("windFy",		toStrC( l.windFy ));

			vgl->SetAttribute("maxTerAng",	toStrC( l.maxTerAng ));
			vgl->SetAttribute("minTerH",	toStrC( l.minTerH ));
			vgl->SetAttribute("maxTerH",	toStrC( l.maxTerH ));
			vgl->SetAttribute("maxDepth",	toStrC( l.maxDepth ));
			pgd->InsertEndChild(vgl);
		}
	root->InsertEndChild(pgd);


	XMLElement* cam = xml.NewElement("cam");  // 🎥
		cam->SetAttribute("pos",	toStrC( camPos ));
		cam->SetAttribute("dir",	toStrC( camDir ));
	root->InsertEndChild(cam);


	XMLElement* objs = xml.NewElement("objects");  // 📦
		for (i=0; i < objects.size(); ++i)
		{
			const Object* o = &objects[i];
			XMLElement* oe = xml.NewElement("o");
			oe->SetAttribute("name",	o->name.c_str() );
			if (!o->material.empty())
				oe->SetAttribute("mat", o->material.c_str() );

			string s = toStr(o->pos[0])+" "+toStr(o->pos[1])+" "+toStr(o->pos[2]);
			oe->SetAttribute("pos",		s.c_str());

			s = toStr(o->rot[0])+" "+toStr(o->rot[1])+" "+toStr(o->rot[2])+" "+toStr(o->rot[3]);
			oe->SetAttribute("rot",		s.c_str());

			if (o->scale != Vector3::UNIT_SCALE)  // dont save default
				oe->SetAttribute("sc",	toStrC( o->scale ));

			if (!o->shadow)
				oe->SetAttribute("sh",	o->shadow ? 1 : 0 );
			objs->InsertEndChild(oe);
		}
	root->InsertEndChild(objs);


	XMLElement* emts = xml.NewElement("emitters");  // 🔥 particles
		for (i=0; i < emitters.size(); ++i)
		{
			const SEmitter* e = &emitters[i];
			XMLElement* oe = xml.NewElement("e");
			oe->SetAttribute("name",		e->name.c_str() );

			oe->SetAttribute("pos",	toStrC(e->pos));
			oe->SetAttribute("sc",	toStrC(e->size));
			oe->SetAttribute("up",	toStrC(e->up));
			oe->SetAttribute("rot",	toStrC(e->rot));
			
			oe->SetAttribute("rate",	toStrC(e->rate));
			oe->SetAttribute("st",	e->stat ? 1 : 0);
			oe->SetAttribute("psc",	toStrC(e->parScale));

			emts->InsertEndChild(oe);
		}
	root->InsertEndChild(emts);


	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}
