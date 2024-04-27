#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "CGui.h"
#include "GuiCom.h"
#include "CGame.h"

#include "MultiList2.h"
#include "Slider.h"
#include "CData.h"
#include "TracksXml.h"

#include <MyGUI.h>
using namespace MyGUI;
using namespace std;


///  🎛️ Gui Init  🏆 Championships
//---------------------------------------------------------------------------------------------------------------------

void CGui::InitGuiChamps()
{
	//  for find defines
	Btn btn, bchk;  Cmb cmb;
	Slider* sl;  SV* sv;  Ck* ck;
	Mli2 li;
	int i,c;
	app->mMainSetupBtns[1]->setVisible(pSet->difficulty < 4);  // tutorials

	//  track descr  2nd set
	gcom->panTrkDesc[1] = fWP("panTrkDesc1");
	gcom->trkDesc[1] = fEd("TrackDesc1");  gcom->trkAdvice[1] = fEd("TrackAdvice1");
	txTrkName = fTxt("TrackText");

	//  track stats text  2nd set
	for (i=0; i < gcom->StTrk; ++i)    gcom->stTrk[1][i] = fTxt("2st"+toStr(i));
	for (i=0; i < gcom->ImStTrk; ++i)  gcom->imStTrk[1][i] = fImg("2ist"+toStr(i));
	for (i=0; i < gcom->InfTrk; ++i)
	{	gcom->infTrk[1][i] = fTxt("2ti"+toStr(i));  gcom->imInfTrk[1][i] = fImg("2iti"+toStr(i));  }


	//  champ
	edChInfo = fEd("ChampInfo");
	if (edChInfo)  edChInfo->setVisible(pSet->champ_info);
	Btn("btnChampInfo",btnChampInfo);

	panCh = fWP("panCh");
	txtCh = fTxt("txtChDetail");  txtChName = fTxt("txtChName");
	valCh = fTxt("valChDetail");
	for (i=0; i<3; ++i) {  Ogre::String s = toStr(i);
		txtChP[i] = fTxt("txtChP"+s);
		valChP[i] = fTxt("valChP"+s);  }
	edChDesc = fEd("ChampDescr");


	//  🏆 Champs list  -------------
	Tbi trktab = fTbi("TabChamps");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	Lev(li, ChampChng);  li->setVisible(false);

	li->removeAllColumns();  c=0;
	li->addColumn("#80A080", colCh[c++]);
	li->addColumn(TR("#40F040#{Name}"), colCh[c++]);		li->addColumn(TR("#F0F040#{Difficulty}"), colCh[c++]);
	li->addColumn(TR("#80F0C0#{Stages}"), colCh[c++]);		li->addColumn(TR("#80E0FF#{Time} #{TimeMS}"), colCh[c++]);
	li->addColumn(TR("#D0C0FF#{Progress}"), colCh[c++]);	li->addColumn(TR("#F0E0F0#{Score}"), colCh[c++]);
	li->addColumn(" ", colCh[c++]);
	liChamps = li;

	//  🥇 Challs list  -------------
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	Lev(li, ChallChng);  li->setVisible(false);

	li->removeAllColumns();  c=0;
	li->addColumn("#80A080", colChL[c++]);
	li->addColumn(TR("#60F060#{Name}"), colChL[c++]);		li->addColumn(TR("#F0D040#{Difficulty}"), colChL[c++]);
	li->addColumn(TR("#F09090#{Vehicles}"), colChL[c++]);
	li->addColumn(TR("#80F0C0#{Stages}"), colChL[c++]);		li->addColumn(TR("#80E0FF#{Time} m"), colChL[c++]);
	li->addColumn(TR("#D0C0FF#{Progress}"), colChL[c++]);
	li->addColumn(TR("#F0F8FF#{Prize}"), colChL[c++]);		li->addColumn(TR("#F0D0F0#{Score}"), colChL[c++]);
	li->addColumn(" ", colChL[c++]);
	liChalls = li;

	//  🏞️ Stages list  -------------
	trktab = (TabItem*)app->mWndGame->findWidget("TabStages");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->setColour(Colour(0.7,0.73,0.76));
	Lev(li, StageChng);  li->setVisible(false);

	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colSt[c++]);
	li->addColumn(TR("#50F050#{Track}"),          colSt[c++]);  li->addColumn(TR("#80FF80#{Scenery}"), colSt[c++]);
	li->addColumn(TR("#F0F040#{Difficulty}"),     colSt[c++]);  li->addColumn(TR("#60E0A0#{Laps}"), colSt[c++]);
	li->addColumn(TR("#80E0FF#{Time} #{TimeMS}"), colSt[c++]);  li->addColumn(TR("#F0E0F0#{Score}"), colSt[c++]);
	li->addColumn(" ", colSt[c++]);
	liStages = li;


	//  tabs
	tabChamp = fTab("ChampType");        Tev(tabChamp, ChampType);  tabChamp->setIndexSelected(pSet->champ_type);
	tabChall = fTab("SubTabChallType");  Tev(tabChall, ChallType);  tabChall->setIndexSelected(pSet->chall_type);
	imgTut   = fImg("imgTut");
	imgChamp = fImg("imgChamp");
	imgChall = fImg("imgChall");

	updChampListDim();
	ChampsListUpdate();  listChampChng(liChamps, liChamps->getIndexSelected());
	ChallsListUpdate();  listChallChng(liChalls, liChalls->getIndexSelected());

	Btn("btnTutStart",  btnChampStart);  btStTut = btn;
	Btn("btnChampStart",btnChampStart);  btStChamp = btn;
	Btn("btnChallStart",btnChallStart);  btStChall = btn;
	Btn("btnChRestart", btnChRestart);   btChRestart = btn;


	//  ch other
	ck= &ckChampRev;	ck->Init("ChampRev",    &pSet->gui.champ_rev);   Cev(ChampRev);
	ck= &ckCh_All;  	ck->Init("Ch_All",      &pSet->ch_all);          Cev(Ch_All);

	Btn("btnChampStageBack", btnChampStageBack);
	Btn("btnChampStageStart",btnChampStageStart);  btChampStage = btn;
	Btn("btnChampEndClose",  btnChampEndClose);

	Btn("btnChallStageBack", btnChallStageBack);
	Btn("btnChallStageStart",btnChallStageStart);  btChallStage = btn;
	Btn("btnChallEndClose",  btnChallEndClose);

	Btn("btnStageNext", btnStageNext);
	Btn("btnStagePrev", btnStagePrev);
	valStageNum = fTxt("StageNum");

	edChampStage = fEd("ChampStageText");  edChampEnd = fEd("ChampEndText");
	edChallStage = fEd("ChallStageText");  edChallEnd = fEd("ChallEndText");
	//  stage prv
	imgChampStage = fImg("ChampStageImg");  imgChampStage->setImageTexture("PrvStCh");
	imgChallStage = fImg("ChallStageImg");	imgChallStage->setImageTexture("PrvStCh");

	imgChampEndCup = fImg("ChampEndImgCup");   txChampEndF = fTxt("ChampEndFinished");
	imgChallFail   = fImg("ChallEndImgFail");  txChallEndF = fTxt("ChallEndFinished");
	imgChallCup    = fImg("ChallEndImgCup");   txChallEndC = fTxt("ChallEndCongrats");

	UpdChampTabVis();
}
