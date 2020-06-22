#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED
#include "devfb.h"
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
#include "../../ui/mainui.h"
#endif
#include "knativesystem.h"
#include "knativewindow.h"

extern U32 platformThreadCount;
extern U32 exceptionCount;
extern U32 dynamicCodeExceptionCount;
static U32 lastTitleUpdate = 0;
extern U32 nativeMemoryPagesAllocated;

bool doMainLoop() {
    while (platformThreadCount) {
        U32 timeout = 5000;
        U32 t = KSystem::getMilliesSinceStart();

        if (KSystem::killTime) {
            if (KSystem::killTime <= t) {
                KNativeSystem::cleanup();
                exit(9);
                return true;
            }
            if (t - KSystem::killTime < timeout) {
                timeout = t - KSystem::killTime;
            }
        }
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        if (uiIsRunning()) {
            timeout = 33;
        }
#endif
#ifdef BOXEDWINE_RECORDER
        if (Player::instance || Recorder::instance) {
            KNativeWindow::getNativeWindow()->waitForEvent(10);
            BOXEDWINE_RECORDER_RUN_SLICE();
        } else  {
            KNativeWindow::getNativeWindow()->waitForEvent(timeout);
        }
#else
        KNativeWindow::getNativeWindow()->waitForEvent(timeout);
#endif    
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        if (uiIsRunning()) {
            uiLoop();
        }
#endif                
        //flipFB();
        if (lastTitleUpdate+5000 < t) {
            char tmp[256];
            lastTitleUpdate = t;
            if (KSystem::title.length()) {
                snprintf(tmp, sizeof(tmp), "%s - BoxedWine 20.1.1", KSystem::title.c_str());
            } else {
                snprintf(tmp, sizeof(tmp), "BoxedWine 20.1.1 %dMB", (int)(nativeMemoryPagesAllocated >> 8));
            }
            KNativeWindow::getNativeWindow()->setTitle(tmp);
        }
        if (!KNativeWindow::getNativeWindow()->processEvents()) {
            return true;
        }
    };
    return true;
}

void waitForProcessToFinish(const std::shared_ptr<KProcess>& process, KThread* thread) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
    while (!process->isTerminated()) {
        BOXEDWINE_CONDITION_WAIT(KSystem::processesCond);
    }
}

#endif
