#include "scene_manager.h"
#include "scene_mainmenu.h"
#include "scene_logo.h"
#include "scene_pass.h"
#include "scene_program.h"
#include "scene_title.h"
#include "scene_mapa.h"
#include "scene_entry.h"
// #include "sprites.h"

bool debug = true;
static SceneType currentScene = SCENE_NONE;

void sceneManagerInit(SceneType initialScene) {
    sceneManagerSwitchTo(initialScene);
}

void sceneManagerUpdate(uint32_t kDown, uint32_t kHeld) {
    switch (currentScene) {
        case SCENE_MAIN_MENU: sceneMainMenuUpdate(kDown, kHeld); break;
        case SCENE_PROGRAM: sceneProgramUpdate(kDown, kHeld); break;
        case SCENE_LOGO: sceneLogoUpdate(kDown, kHeld); break;
        case SCENE_MAPA: sceneMapaUpdate(kDown, kHeld); break;
        case SCENE_TITLE: sceneTitleUpdate(kDown, kHeld); break;
        case SCENE_ENTRY: sceneEntryUpdate(kDown, kHeld); break;
        case SCENE_PASS: scenePassUpdate(kDown, kHeld); break;
        default: break;
    }
}

void sceneManagerRender(void) {
    switch (currentScene) {
        case SCENE_MAIN_MENU: sceneMainMenuRender(); break;
        case SCENE_PROGRAM: sceneProgramRender(); break;
        case SCENE_LOGO: sceneLogoRender(); break;
        case SCENE_MAPA: sceneMapaRender(); break;
        case SCENE_TITLE: sceneTitleRender(); break;
        case SCENE_ENTRY: sceneEntryRender(); break;
        case SCENE_PASS: scenePassRender(); break;
        default: break;
    }
}

void sceneManagerSwitchTo(SceneType nextScene) {
    
    switch (currentScene) {
        case SCENE_MAIN_MENU: sceneMainMenuExit(); break;
        case SCENE_PROGRAM: sceneProgramExit(); break;
        case SCENE_LOGO: sceneLogoExit(); break;
        case SCENE_MAPA: sceneMapaExit(); break;
        case SCENE_TITLE: sceneTitleExit(); break;
        case SCENE_ENTRY: sceneEntryExit(); break;
        case SCENE_PASS: scenePassExit(); break;
        default: break;
    }


    currentScene = nextScene;
    switch (currentScene) {
        case SCENE_MAIN_MENU: sceneMainMenuInit(); break;
        case SCENE_PROGRAM: sceneProgramInit(); break;
        case SCENE_LOGO: sceneLogoInit(); break;
        case SCENE_MAPA: sceneMapaInit(); break;
        case SCENE_TITLE: sceneTitleInit(); break;
        case SCENE_ENTRY: sceneEntryInit(); break;
        case SCENE_PASS: scenePassInit(); break;
        default: break;
    }
}

void sceneManagerExit(void) {
    sceneManagerSwitchTo(SCENE_NONE);
}
