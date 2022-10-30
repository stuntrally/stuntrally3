#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "SDL_scancode.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreWindow.h"

#include "Terra/Terra.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include "OgreGpuProgramManager.h"

#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
#    include "OgreAtmosphereNpr.h"
#endif

#include "game.h"  // snd
#include "SoundMgr.h"
#include "CarPosInfo.h"
#include "CGame.h"
#include "CarModel.h"
#include "FollowCamera.h"

#include "Def_Str.h"
using namespace Demo;
using namespace Ogre;


namespace Demo
{

    //  Update  frame
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::update( float dt )
    {

        if (pApp->bLoading)
        {
            pApp->NewGameDoLoad();
            // PROFILER.endBlock(" frameSt");
            // return;  //?
        }
        else 
        {
            ///  loading end  ------
            const int iFr = 3;
            if (pApp->iLoad1stFrames >= 0)
            {	++pApp->iLoad1stFrames;
                if (pApp->iLoad1stFrames == iFr)
                {
                    // LoadingOff();  // hide loading overlay
                    // mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);
                    // gui->Ch_LoadEnd();
                    pApp->bLoadingEnd = true;
                    // iLoad1stFrames = -1;  // for refl
                }
            }else if (pApp->iLoad1stFrames >= -1)
            {
                --pApp->iLoad1stFrames;  // -2 end

                // imgLoad->setVisible(false);  // hide back imgs
                // if (imgBack)
                //     imgBack->setVisible(false);
            }
        }
    
        if (pGame && pApp)
        {
            // enum PlayerActions
            // {	A_Throttle, A_Brake, A_Steering, A_HandBrake, A_Boost, A_Flip,
            //     A_ShiftUp, A_ShiftDown, A_PrevCamera, A_NextCamera, A_LastChk, A_Rewind, NumPlayerActions
            // };
            //  inputs
            pApp->inputs[0] = mArrows[2];
            pApp->inputs[1] = mArrows[3];
            pApp->inputs[2] = 0.5f * (1 + mArrows[1] - mArrows[0]);
            pApp->inputs[3] = mArrows[4];
            pApp->inputs[4] = mArrows[5];
            pApp->inputs[5] = 0.5f * (1 + mArrows[6] - mArrows[7]);

            pGame->OneLoop(dt);  // sim

            //  set car pos
            if (!pGame->cars.empty() && !pApp->carModels.empty())
            {
                auto carM = pApp->carModels[0];
                PosInfo pi, po;  pi.FromCar(pGame->cars[0]);
                pi.bNew = true;
                po.bNew = true;
                carM->Update(pi, pi, dt);

                if (mCubeCamera)  // refl
                    mCubeCamera->setPosition(pi.pos);

                //  update camera
                if (carM->fCam)
                    carM->fCam->update(dt, pi, /*&carPoses[qn][c]*/&po, &pGame->collision,
                        /*!bRplPlay &&*/ pApp->pSet->cam_bounce, carM->vtype == V_Sphere);
                // iCurPoses[c] = qn;  // atomic, set new index in queue

                Camera *camera = mGraphicsSystem->getCamera();
                if (carM->fCam)
                {   camera->setPosition(carM->fCam->camPosFinal);
                    camera->setOrientation(carM->fCam->camRotFinal);
                }
                /*Node *cameraNode = camera->getParentNode();

                cameraNode->_getFullTransformUpdated();
                cameraNode->setOrientation(po.rot);
                cameraNode->setPosition(po.pos);*/

                // camera->setPosition(po.pos);
                // camera->setOrientation(po.rot);
                
                ///))  upd sound camera
                if (/*c == 0 &&*/ pGame->snd)
                {
                    Vector3 x,y,z;
                    /*carPoses[qn][c]*/po.camRot.ToAxes(x,y,z);
                    bool camCar = carM->fCam && carM->fCam->TypeCar();  // fix
                    pGame->snd->setCamera(/*carPoses[qn][c]*/po.camPos, camCar ? -y : -z, camCar ? -z : y, Vector3::ZERO);
                }
            }
        }

        //  Keys
        float mul = shift ? 0.2f : ctrl ? 3.f : 1.f;
        int d = right ? 1 : left ? -1 : 0;
        if (d)
        {
            SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
            AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr*>( sceneManager->getAtmosphere() );
            AtmosphereNpr::Preset p = atmosphere->getPreset();
            float mul1 = 1.f + 0.003f * mul * d;
            switch (param)
            {
            case 0:  p.fogDensity *= mul1;  break;
            case 1:  p.densityCoeff *= mul1;  break;
            case 2:  p.densityDiffusion *= mul1;  break;
            case 3:  p.horizonLimit *= mul1;  break;
            case 4:  p.sunPower *= mul1;  break;
            case 5:  p.skyPower *= mul1;  break;
            case 6:  p.skyColour.x *= mul1;  break;
            case 7:  p.skyColour.y *= mul1;  break;
            case 8:  p.skyColour.z *= mul1;  break;
            case 9:   p.fogBreakMinBrightness *= mul1;  break;
            case 10:  p.fogBreakFalloff *= mul1;  break;
            case 11:  p.linkedLightPower *= mul1;  break;
            case 12:  p.linkedSceneAmbientUpperPower *= mul1;  break;
            case 13:  p.linkedSceneAmbientLowerPower *= mul1;  break;
            case 14:  p.envmapScale *= mul1;  break;
            }
            atmosphere->setPreset(p);
        }

        d = mKeys[0] - mKeys[1];
        if (d)
        {
            mPitch += d * mul * 0.6f * dt;
            mPitch = std::max( 0.f, std::min( mPitch, (float)Math::PI ) );
        }

        d = mKeys[2] - mKeys[3];
        if (d)
        {
            mYaw += d * mul * 1.5f * dt;
            mYaw = fmodf( mYaw, Math::TWO_PI );
            if( mYaw < 0.f )
                mYaw = Math::TWO_PI + mYaw;
        }
        //  Light  sun dir  ----
        Vector3 dir = Quaternion( Radian(mYaw), Vector3::UNIT_Y ) *
            Vector3( cosf( mPitch ), -sinf( mPitch ), 0.0 ).normalisedCopy();
        mSunLight->setDirection( dir );


        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        // sceneManager->setAmbientLight( Ogre::ColourValue( 0.3f, 0.5f, 0.7f ) * 0.1f * 0.75f,
        //                                Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 0.065f * 0.75f,
        //                                -light->getDirection() + Ogre::Vector3::UNIT_Y * 0.2f );
        sceneManager->setAmbientLight(
            ColourValue( 0.99f, 0.94f, 0.90f ) * 0.04f,  //** par
            ColourValue( 0.90f, 0.93f, 0.96f ) * 0.04f,
            // ColourValue( 0.93f, 0.91f, 0.38f ) * 0.04f,
            // ColourValue( 0.22f, 0.53f, 0.96f ) * 0.04f,
            // ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
            // ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
            -dir );

    #ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
        OGRE_ASSERT_HIGH( dynamic_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() ) );
        AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() );
        atmosphere->setSunDir( mSunLight->getDirection(), mPitch / Math::PI );
    #endif

        ///  Terrain  ----
        if (mTerra && mGraphicsSystem->getRenderWindow()->isVisible() )
        {
            // Force update the shadow map every frame to avoid the feeling we're "cheating" the
            // user in this sample with higher framerates than what he may encounter in many of
            // his possible uses.
            const float lightEpsilon = 0.0001f;  //** 0.0f slow
            mTerra->update( mSunLight->getDerivedDirectionUpdated(), lightEpsilon );
        }

        generateDebugText();

        TutorialGameState::update( dt );
    }


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    //  Key events
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::keyPressed( const SDL_KeyboardEvent &arg )
    {
        switch (arg.keysym.scancode)
        {
        case SDL_SCANCODE_RSHIFT:  shift = true;  break;  // mod
        case SDL_SCANCODE_RCTRL:   ctrl = true;   break;


        case SDL_SCANCODE_LEFT:   mArrows[0] = 1;  break;  // car
        case SDL_SCANCODE_RIGHT:  mArrows[1] = 1;  break;
        case SDL_SCANCODE_UP:     mArrows[2] = 1;  break;
        case SDL_SCANCODE_DOWN:   mArrows[3] = 1;  break;

        case SDL_SCANCODE_SPACE:  mArrows[4] = 1;  break;
        case SDL_SCANCODE_LCTRL:  mArrows[5] = 1;  break;
        case SDL_SCANCODE_1:      mArrows[6] = 1;  break;
        case SDL_SCANCODE_2:      mArrows[7] = 1;  break;


        case SDL_SCANCODE_HOME:  left  = true;  break;  // params
        case SDL_SCANCODE_END:   right = true;  break;
        case SDL_SCANCODE_PAGEUP:     --param;  break;
        case SDL_SCANCODE_PAGEDOWN:   ++param;  break;

        case SDL_SCANCODE_KP_PLUS:      mKeys[0] = 1;  break;  // sun
        case SDL_SCANCODE_KP_MINUS:     mKeys[1] = 1;  break;
        case SDL_SCANCODE_KP_MULTIPLY:  mKeys[2] = 1;  break;
        case SDL_SCANCODE_KP_DIVIDE:    mKeys[3] = 1;  break;

        // case SDL_SCANCODE_SPACE:  // snd test
            // pGame->snd_lapbest->start();  //)
            // break;

        
        //** terrain wireframe toggle
        case SDL_SCANCODE_R:
        {
            wireTerrain = !wireTerrain;
            Root *root = mGraphicsSystem->getRoot();
            HlmsManager *hlmsManager = root->getHlmsManager();
            HlmsDatablock *datablock = 0;
            datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
            if (wireTerrain)
            {
                datablock = hlmsManager->getHlms( HLMS_USER3 )->getDefaultDatablock();
                datablock->setMacroblock( macroblockWire );
            }
            mTerra->setDatablock( datablock );
        }   break;

        case SDL_SCANCODE_T:
            if (mTerra)
            {   DestroyTerrain();  CreatePlane();  }
            else
            {   CreateTerrain();  DestroyPlane();  }
            break;

        //  Vegetation add, destroy all
        case SDL_SCANCODE_V:  CreateVeget();  break;
        case SDL_SCANCODE_C:  DestroyVeget();  break;

        //  other
        case SDL_SCANCODE_F:  CreateParticles();  break;
        // case SDL_SCANCODE_G:  CreateCar();  break;

        case SDL_SCANCODE_K:  
            if (ndSky)
                DestroySkyDome();
            else
            {   switch (iSky)
                {
                case 0:  CreateSkyDome("sky-clearday1", 0.f);  ++iSky;  break;
                case 1:  CreateSkyDome("sky_photo6", 0.f);  iSky = 0;  break;  // clouds
                }
            }
            break;
        
        case SDL_SCANCODE_M:
        {
            // Vector3 camPos(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
            CreateManualObj(camPos);
        }   break;
        }
        
        TutorialGameState::keyPressed( arg );
    }
    
    void TerrainGame::keyReleased( const SDL_KeyboardEvent &arg )
    {
        switch (arg.keysym.scancode)
        {
        case SDL_SCANCODE_RSHIFT:  shift = false;  break;  // mod
        case SDL_SCANCODE_RCTRL:   ctrl = false;   break;


        case SDL_SCANCODE_LEFT:   mArrows[0] = 0;  break;  // car
        case SDL_SCANCODE_RIGHT:  mArrows[1] = 0;  break;
        case SDL_SCANCODE_UP:     mArrows[2] = 0;  break;
        case SDL_SCANCODE_DOWN:   mArrows[3] = 0;  break;

        case SDL_SCANCODE_SPACE:  mArrows[4] = 0;  break;
        case SDL_SCANCODE_LCTRL:  mArrows[5] = 0;  break;
        case SDL_SCANCODE_1:      mArrows[6] = 0;  break;
        case SDL_SCANCODE_2:      mArrows[7] = 0;  break;


        case SDL_SCANCODE_HOME:  left = false;   break;  // params
        case SDL_SCANCODE_END:   right = false;  break;

        case SDL_SCANCODE_KP_PLUS:      mKeys[0] = 0;  break;  // sun
        case SDL_SCANCODE_KP_MINUS:     mKeys[1] = 0;  break;
        case SDL_SCANCODE_KP_MULTIPLY:  mKeys[2] = 0;  break;
        case SDL_SCANCODE_KP_DIVIDE:    mKeys[3] = 0;  break;


        case SDL_SCANCODE_F4:
        if (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
        {
            //Hot reload of Terra shaders.
            Root *root = mGraphicsSystem->getRoot();
            HlmsManager *hlmsManager = root->getHlmsManager();

            Hlms *hlms = hlmsManager->getHlms( HLMS_USER3 );
            GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom( hlms->getDataFolder() );
        }   break;
        }

        TutorialGameState::keyReleased( arg );
    }
}
#pragma GCC diagnostic pop
