#include "boxedwine.h"
#include "knativesystem.h"
#include <SDL.h>

#ifndef __TEST
int boxedmain(int argc, const char** argv);

int main(int argc, char** argv) {
    return boxedmain(argc, (const char**)argv);
}
#endif

bool KNativeSystem::init(bool allowVideo, bool allowAudio) {
    U32 flags = SDL_INIT_EVENTS;

    if (allowVideo) {
        flags |= SDL_INIT_VIDEO;
    }
    if (allowAudio) {
        flags |= SDL_INIT_AUDIO;
    }
    if (SDL_Init(flags) != 0) {
        klog("SDL_Init Error: %s", SDL_GetError());
        return false;
    }
    return true;
}

void KNativeSystem::cleanup() {
    SDL_Quit();
}

void KNativeSystem::postQuit() {
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}

U32 KNativeSystem::getTicks() {
    return SDL_GetTicks();
}

bool KNativeSystem::getScreenDimensions(U32* width, U32* height) {
    SDL_DisplayMode mode;
    if (!SDL_GetCurrentDisplayMode(0, &mode)) {
        *width = mode.w;
        *height = mode.h;
        return true;
    }
    return false;
}

std::string KNativeSystem::getAppDirectory() {
    char* s = SDL_GetBasePath();
    std::string result;

    if (s) {
        result = s;
        SDL_free(s);
    }
    return result;
}

std::string KNativeSystem::getLocalDirectory() {
    char* s = SDL_GetPrefPath("", "Boxedwine");
    std::string result;

    if (s) {
        result = s;
        SDL_free(s);
    }
    return result;
}

bool KNativeSystem::clipboardHasText() {
    return SDL_HasClipboardText()?true:false;
}

std::string KNativeSystem::clipboardGetText() {
    char* s = SDL_GetClipboardText();
    std::string result;
    if (s) {
        result = s;
        SDL_free(s);
    }
    return result;
}

bool KNativeSystem::clipboardSetText(const std::string& text) {
    return SDL_SetClipboardText(text.c_str()) ? true : false;
}

U32 KNativeSystem::getDpiScale() {
    const float defaultDPI =
#ifdef __APPLE__
        72.0f;
#else
        96.0f;
#endif
    float dpi = defaultDPI;

    if (SDL_GetDisplayDPI(0, NULL, &dpi, NULL) != 0) {
        return SCALE_DENOMINATOR;
    }
    return (U32)(dpi / defaultDPI * SCALE_DENOMINATOR);
}

void KNativeSystem::preReturnToUI() {
    // make sure if the user closed the SDL windows for the game/app, that it doesn't carry over into the UI
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_QUIT);
}