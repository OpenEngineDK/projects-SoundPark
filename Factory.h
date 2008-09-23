#ifndef _FACTORY_
#define _FACTORY_

#include <Core/IEngine.h>
#include <Display/Camera.h>
#include <Display/Viewport.h>
#include <Renderers/IRenderer.h>

class OpenEngine::Scene::ISceneNode;

using OpenEngine::Scene::ISceneNode;

using namespace OpenEngine::Renderers;
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;

class Factory {
private:
    IFrame*    frame;
    IRenderer* renderer;
    Viewport*  viewport;
    Camera*    camera; 

public:
    Factory();
    virtual ~Factory();
    bool         SetupEngine(IEngine& engine);
    IFrame*      GetFrame();
    IRenderer*   GetRenderer();

    static string filename;
};

#endif
