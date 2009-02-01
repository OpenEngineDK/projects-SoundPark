#include "Factory.h"

// from OpenEngine
#include <Logging/Logger.h>
#include <Display/Frustum.h>
#include <Display/Viewport.h>
#include <Display/ViewingVolume.h>
#include <Scene/GeometryNode.h>
#include <Scene/TransformationNode.h>
#include <Resources/ResourceManager.h>
#include <Utils/Statistics.h>

#include <Display/SDLFrame.h>
#include <Devices/SDLInput.h>

#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Renderers/OpenGL/TextureLoader.h>
#include <Renderers/OpenGL/LightRenderer.h>

#include <Sound/OpenALSoundSystem.h>
#include <Sound/ISoundSystem.h>
#include <Sound/ISound.h>
#include <Scene/SoundNode.h>
#include <Sound/SoundRenderer.h>

#include <Resources/VorbisResource.h>
#include <Resources/ISoundResource.h>
#include <Resources/TGAResource.h>
#include <Resources/ITextureResource.h>

#include <Scene/SceneNode.h>
#include <Scene/SphereNode.h>

#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>

#include <Utils/MoveHandler.h>
#include <Utils/QuitHandler.h>
#include <Utils/PropertyList.h>
#include <Sky/SkyDome.h>

#include <Core/IModule.h>


#include <string>

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Sound;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Utils;

using OpenEngine::Sky::SkyDome;

class Handler : public IListener<KeyboardEventArg>, public IListener<ProcessEventArg> {
private:
    TransformationNode*** spheres;
    int type;
public:

    Handler(TransformationNode*** spheres): spheres(spheres), type(0){}
    ~Handler() {}
    
    void Handle(KeyboardEventArg arg) {
        if (arg.type == EVENT_PRESS) {
            switch (arg.sym) {
            case KEY_u: type = 1; break;
            case KEY_i: type = 2; break;
            case KEY_o: type = 3; break;
            default: 
                break;
            }
        }
    }
    
    void Handle(ProcessEventArg arg) {
        float dt = arg.approx / 1000.0;
        if (type == 0) return;
        float dist = 0;
        switch (type) {
        case 1: dist = 100; break;
        case 2: dist = 500; break;
        case 3: dist = 800; break;
        }
        
        float currdist = (spheres[0][1]->GetPosition() - spheres[0][0]->GetPosition()).GetLength();
        if (currdist < dist) {
            currdist += 1*dt;
        }
        else currdist -= 1*dt;

        if (max(currdist-dist,dist-currdist) < 10) {
            currdist = dist;
            type = 0;
        }
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                spheres[i][j]->SetPosition(Vector<3,float>(i*currdist,0,j*currdist));
            }
        }
    }
};


Factory::Factory() {
    frame    = new SDLFrame(800, 600, 32);
    viewport = new Viewport(*frame);
        
    camera = new Camera(*(new ViewingVolume()));
    camera->SetPosition(Vector<3,float>(0,0,0));
    //viewport->SetViewingVolume(camera);

    // frustum hack
    Frustum* frustum = new Frustum(*camera);
    frustum->SetFar(15000);
    frustum->SetNear(1);
    viewport->SetViewingVolume(frustum);

    renderer = new Renderer(viewport);
    renderer->ProcessEvent().Attach(*(new RenderingView(*viewport)));
    renderer->InitializeEvent().Attach(*(new TextureLoader()));
    renderer->PreProcessEvent().Attach(*(new LightRenderer(*camera)));
}

Factory::~Factory() {
    delete frame;
    delete viewport;
    delete camera;
    delete renderer;
}

bool Factory::SetupEngine(IEngine& engine) {
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
    this->renderer->SetSceneRoot(root);

    //First we set the resources directory
    //         DirectoryManager::AppendPath("/");
    DirectoryManager::AppendPath(""); //current directory
    DirectoryManager::AppendPath("projects/SoundPark/data/GregoryTripi22/ogg/");        
    DirectoryManager::AppendPath("projects/SoundPark/data/Skydome/");        
    // load the resource plug-ins
    ResourceManager<ISoundResource>::AddPlugin(new VorbisResourcePlugin());
    ResourceManager<ITextureResource>::AddPlugin(new TGAPlugin());
        
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



    //        engine.AddModule(*(new Statistics(1000)));

//     } catch (const Exception& ex) {
//         logger.error << "An exception occurred: " << ex.what() << logger.end;
//         throw ex;
//     }
    // Return true to signal success.
    return true;
}

// Get methods for the factory.
IFrame*      Factory::GetFrame()      { return frame; }
IRenderer*   Factory::GetRenderer()   { return renderer; }
