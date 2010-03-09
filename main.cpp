// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "KeyHandler.h"

// OpenEngine stuff
#include <Meta/Config.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Core/Engine.h>
#include <Display/Viewport.h>
#include <Display/Camera.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Resources/ResourceManager.h>
#include <Scene/SceneNode.h>
#include <Scene/SphereNode.h>
#include <Scene/PointLightNode.h>
#include <Devices/IMouse.h>
#include <Devices/IKeyboard.h>
#include <Renderers/TextureLoader.h>
#include <Utils/MoveHandler.h>
#include <Utils/QuitHandler.h>

// SDL extension
#include <Display/SDLEnvironment.h>

// OpenGL stuff
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Renderers/OpenGL/LightRenderer.h>

//skydome
#include <Utils/PropertyList.h>
#include <Sky/SkyDome.h>

// Sound stuff
#include <Sound/OpenALSoundSystem.h>
#include <Sound/ISoundSystem.h>
#include <Sound/ISound.h>
#include <Scene/SoundNode.h>
#include <Sound/SoundRenderer.h>

#include <Resources/VorbisResource.h>
#include <Resources/ISoundResource.h>
#include <Resources/TGAResource.h>
#include <Resources/ITexture2D.h>

using namespace OpenEngine::Core;
using namespace OpenEngine::Devices;
using namespace OpenEngine::Display;
using namespace OpenEngine::Logging;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Sound;
using namespace OpenEngine::Sky;

Engine* engine;
IEnvironment* env;
IFrame* frame;
Viewport* viewport;
Renderer* renderer;
IMouse* mouse;
IKeyboard* keyboard;
ISceneNode* scene;
Camera* camera;
IRenderingView* renderingview;
TextureLoader* textureloader;

class TextureLoadOnInit
    : public IListener<RenderingEventArg> {
    TextureLoader& tl;
public:
    TextureLoadOnInit(TextureLoader& tl) : tl(tl) { }
    void Handle(RenderingEventArg arg) {
        if (arg.renderer.GetSceneRoot() != NULL)
            tl.Load(*arg.renderer.GetSceneRoot());
    }
};


int main(int argc, char** argv) {
    // Setup logging facilities.
    Logger::AddLogger(new StreamLogger(&std::cout));

    // Print usage info.
    logger.info << "========= Running OpenEngine Test Project =========" << logger.end;

    frame    = new SDLFrame(800, 600, 32);
    viewport = new Viewport(*frame);
        
    camera = new Camera(*(new PerspectiveViewingVolume()));
    camera->SetPosition(Vector<3,float>(0,0,0));
    viewport->SetViewingVolume(camera);

    renderer = new Renderer(viewport);
    renderer->ProcessEvent().Attach(*(new RenderingView(*viewport)));
    renderer->PreProcessEvent().Attach(*(new LightRenderer(*viewport)));

    Engine engine;
    engine.InitializeEvent().Attach(*frame);
    engine.ProcessEvent().Attach(*frame);
    engine.DeinitializeEvent().Attach(*frame);

    engine.InitializeEvent().Attach(*renderer);
    engine.ProcessEvent().Attach(*renderer);
    engine.DeinitializeEvent().Attach(*renderer);

    // Setup input handling
    SDLInput* input = new SDLInput();
    engine.InitializeEvent().Attach(*input);
    engine.ProcessEvent().Attach(*input);
    engine.DeinitializeEvent().Attach(*input);

    // Register the handler as a listener on up and down keyboard events.
    MoveHandler* move_h = new MoveHandler(*camera, *input);
    input->KeyEvent().Attach(*move_h);
    engine.InitializeEvent().Attach(*move_h);
    engine.ProcessEvent().Attach(*move_h);
    engine.DeinitializeEvent().Attach(*move_h);

    QuitHandler* quit_h = new QuitHandler(engine);
    input->KeyEvent().Attach(*quit_h);

    // Create scene root
    ISceneNode* root = new SceneNode();
    renderer->SetSceneRoot(root);

    //First we set the resources directory
    //         DirectoryManager::AppendPath("/");
    DirectoryManager::AppendPath("projects/SoundPark/data/GregoryTripi22/ogg/");        
    DirectoryManager::AppendPath("projects/SoundPark/data/Skydome/");        
    // load the resource plug-ins
    ResourceManager<ISoundResource>::AddPlugin(new VorbisResourcePlugin());
    ResourceManager<ITexture2D>::AddPlugin(new TGAPlugin());
        
    ISoundSystem* openalsmgr = new OpenALSoundSystem(root, camera);
    engine.InitializeEvent().Attach(*openalsmgr);
    engine.ProcessEvent().Attach(*openalsmgr);
    engine.DeinitializeEvent().Attach(*openalsmgr);

    SoundRenderer* sr = new SoundRenderer();
    renderer->PreProcessEvent().Attach(*sr);
    string soundarray[3][3] = {{"Atmosphere Pad 01.ogg","Atmosphere Pad 02.ogg", "Atmosphere Pad 03.ogg"}, 
                               {"Atmosphere Pad 04.ogg", "Atmosphere Pad 05.ogg", "Atmosphere Pad 06.ogg"},
                               {"Atmosphere Pad 07.ogg", "Atmosphere Pad 08.ogg", "Atmosphere Pad 08.ogg"}};
        
    float dist = 500;
    float soundrad = 350;

    TransformationNode*** spheres = new TransformationNode**[3];
    for (int i = 0; i < 3; i++)
        spheres[i] = new TransformationNode*[3];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ISoundResourcePtr soundres = 
                ResourceManager<ISoundResource>::Create(soundarray[i][j]);
            IMonoSound* sound = (IMonoSound*)openalsmgr->CreateSound(soundres);
            sound->SetMaxDistance(soundrad);
            sound->SetLooping(true);
            sound->SetGain(10.0);
            //sound->SetMaxGain(20.0);
            //sound->SetMinGain(0.0);
            //sound->SetRolloffFactor(1);
            //sound->SetReferenceDistance(soundrad);
            // float g = sound->GetGain();
            //                 logger.info << "g: " << g <<logger.end;
            SoundNode* soundNode = new SoundNode(sound);
            sound->Play();
                
            TransformationNode* tn = new TransformationNode();
            spheres[i][j] = tn;
            tn->Move(i*dist,0,j*dist);
            Vector<3,float> col;
            if ((i+j) %2 == 0)
                col = Vector<3,float>(1.0,0.2,0.2);
            else col = Vector<3,float>(0.2,1.0,0.2);
            sr->AddSoundNode(soundNode,col);
            tn->AddNode(soundNode);
            root->AddNode(tn);
        }
    }

    // load skydome
    string skyfile = "skydome.txt";
    PropertyList* skylist = new PropertyList(skyfile);
    SkyDome* sky = new SkyDome(skylist);
    root->AddNode(sky->GetSceneNode());
    
    Handler* h = new Handler(spheres);
    input->KeyEvent().Attach(*h);
    engine.ProcessEvent().Attach(*h);

    // lighting setup
    TransformationNode* tn2 = new TransformationNode();
    tn2->Move(0,200,0);
    PointLightNode* pln = new PointLightNode();
    tn2->AddNode(pln);
    root->AddNode(tn2);
    SphereNode* sphere2 = new SphereNode();
    pln->AddNode(sphere2);

    textureloader = new TextureLoader(*renderer);
    renderer->PreProcessEvent().Attach(*textureloader);
    textureloader->Load(*root);

    engine.Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
