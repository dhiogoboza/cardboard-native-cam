#ifndef NV_APP_H_
#define NV_APP_H_

#include <thread>
#include "util.h"

namespace nv {

namespace render {
class NVRenderer;
}

class NVApp {
    public:
        NVApp(ndk_hello_cardboard::VRConfigs configs);

        ~NVApp();

        void Init();

        void Resume();

        void Pause();

        void Deinit();

        render::NVRenderer *Render();

    protected:
        void CreateGLThread();

        void DestroyGLThread();

    private:
        ndk_hello_cardboard::VRConfigs vr_configs_;
        render::NVRenderer *renderer_;
        std::thread gl_thread_;
};

}


#endif //NV_APP_H_