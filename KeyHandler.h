#include <Scene/TransformationNode.h>
#include <Core/IListener.h>
#include <Devices/IKeyboard.h>
#include <Core/EngineEvents.h>

#include <string>

using namespace OpenEngine::Core;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Devices;

using namespace std;

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
        
        float currdist = (spheres[0][1]->GetPosition() - 
                          spheres[0][0]->GetPosition()).GetLength();
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
                spheres[i][j]
                    ->SetPosition(Vector<3,float>(i*currdist,0,j*currdist));
            }
        }
    }
};
