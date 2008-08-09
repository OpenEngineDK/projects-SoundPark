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

#include <Sound/OpenALSoundSystem.h>
#include <Sound/ISoundSystem.h>
#include <Sound/ISound.h>
#include <Scene/SoundNode.h>
#include <Sound/SoundRenderer.h>

#include <Resources/VorbisResource.h>
#include <Resources/ISoundResource.h>

#include <Scene/SphereNode.h>

#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>

#include <Utils/MoveHandler.h>
#include <Utils/QuitHandler.h>

#include <string>

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Sound;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Utils;


class PlayHandler : public IListener<KeyboardEventArg> {

private:
    SoundNode* sn;
public:

    PlayHandler(SoundNode* sn): sn(sn){}
    ~PlayHandler() {}
    
    void Handle(KeyboardEventArg arg) {
        if (arg.type == KeyboardEventArg::PRESS) {
            switch (arg.sym) {
            case KEY_p: sn->GetSound()->Play(); break;
            case KEY_o: sn->GetSound()->Stop(); break;
            case KEY_i: sn->GetSound()->Pause(); break;
            case KEY_UP: sn->GetSound()->SetMaxDistance(sn->GetSound()->GetMaxDistance()+1); break;
            case KEY_DOWN: sn->GetSound()->SetMaxDistance(sn->GetSound()->GetMaxDistance()-1); break;
            default: 
                break;
            }
        }
    }
    
    void BindToEventSystem() {
        IKeyboard::keyEvent.Attach(*this);
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
    frustum->SetFar(5000);
    frustum->SetNear(0.0001);
    viewport->SetViewingVolume(frustum);

    renderer = new Renderer();
    renderer->process.Attach(*(new RenderingView(*viewport)));
}

Factory::~Factory() {
    delete frame;
    delete viewport;
    delete camera;
    delete renderer;
}

bool Factory::SetupEngine(IGameEngine& engine) {
    try {
        // Setup input handling
        SDLInput* input = new SDLInput();
        engine.AddModule(*input);


        // Register the handler as a listener on up and down keyboard events.
        MoveHandler* move_h = new MoveHandler(*camera);
        engine.AddModule(*move_h);
        move_h->BindToEventSystem();
        QuitHandler* quit_h = new QuitHandler();
        quit_h->BindToEventSystem();

        // Create scene root
        SceneNode* root = new SceneNode();
        this->renderer->SetSceneRoot(root);

        //First we set the resources directory
        //         DirectoryManager::AppendPath("/");
        DirectoryManager::AppendPath(""); //current directory
        DirectoryManager::AppendPath("projects/SoundPark/data/GregoryTripi22/ogg");        
        // load the resource plug-ins
        ResourceManager<ISoundResource>::AddPlugin(new VorbisResourcePlugin());
        
        ISoundSystem* openalsmgr = new OpenALSoundSystem(root, camera);
        engine.AddModule(*openalsmgr);

        SoundRenderer* sr = new SoundRenderer();
        renderer->preProcess.Attach(*sr);
        string soundarray[3][3] = {{"Atmosphere Pad 01.ogg","Atmosphere Pad 02.ogg", "Atmosphere Pad 03.ogg"}, 
                                      {"Atmosphere Pad 04.ogg", "Atmosphere Pad 05.ogg", "Atmosphere Pad 06.ogg"},
                                      {"Atmosphere Pad 07.ogg", "Atmosphere Pad 08.ogg", "Atmosphere Pad 08.ogg"}};
     
        
        float dist = 100;
        float soundrad = 50;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                ISoundResourcePtr soundres = 
                    ResourceManager<ISoundResource>::Create(string("projects/SoundPark/data/GregoryTripi22/ogg/")+soundarray[i][j]);
                ISound* sound = openalsmgr->CreateSound(soundres);
                sound->SetMaxDistance(soundrad*2);
                sound->SetLooping(true);
                sound->SetGain(10.0);
                sound->SetMaxGain(20.0);
                sound->SetMinGain(0.0);
                sound->SetRolloffFactor(0.5);
                //sound->SetReferenceDistance(soundrad);
                // float g = sound->GetGain();
//                 logger.info << "g: " << g <<logger.end;
                SoundNode* soundNode = new SoundNode(sound);
                sound->Play();
                

                TransformationNode* tn = new TransformationNode();
                tn->Move(i*dist*2,0,j*dist*2);
                Vector<3,float> col;
                if ((i+j) %2 == 0)
                    col = Vector<3,float>(1.0,0.2,0.2);
                else col = Vector<3,float>(0.2,1.0,0.2);
                sr->AddSoundNode(soundNode,col);
                tn->AddNode(soundNode);
                root->AddNode(tn);
            }
        }

        // lighting setup
        TransformationNode* tn2 = new TransformationNode();
        tn2->Move(0,0,-100);
        PointLightNode* dln = new PointLightNode();
        dln->linearAtt = 0.001;
        dln->quadAtt = 0.0001;
        dln->constAtt = 0.5;
        tn2->AddNode(dln);
        root->AddNode(tn2);
        SphereNode* sphere2 = new SphereNode();        
        dln->AddNode(sphere2);



        //        engine.AddModule(*(new Statistics(1000)));

    } catch (const Exception& ex) {
        logger.error << "An exception occurred: " << ex.what() << logger.end;
        throw ex;
    }
    // Return true to signal success.
    return true;
}

// Get methods for the factory.
IFrame*      Factory::GetFrame()      { return frame; }
IRenderer*   Factory::GetRenderer()   { return renderer; }
