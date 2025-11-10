#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include "scene_logo.h"
#include "scene_manager.h"
// #include "main.h"
#include "cwav_shit.h"
#include "global_draws.h"
#include "sprites.h"
extern CWAVInfo cwavList[8];


CWAV* splashb;
void sceneLogoInit(void) {
    splashb = cwavList[0].cwav;
}

void sceneLogoUpdate(uint32_t kDown, uint32_t kHeld) {
    if (!splashDone) {
        float dt = 1.0f / 60.0f;
        splashTimer += dt;

        if (!splashPlayed) {
            cwavPlay(splashb, 0, 1);
            splashPlayed = true;
        }

        // Only increment once here
        if (splashHopTime < splashHopDuration) {
            splashHopTime += dt;
            splashY = easeHop(splashHopTime, 100.0f, 0.0f, splashHopDuration);
        } else {
            splashY = 0.0f;
        }
    }
}
void sceneLogoRender(void) {
    if (!splashDone) {
        if (splashTimer < 4.6f) {
            C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
            C2D_SceneBegin(top);
            C2D_DrawImageAt(splash1, 0.0f, splashY, 0.0f, NULL, 1.0f, 1.0f);
            if (splashTimer > 1.2f) {
                C2D_DrawImageAt(splash2, 0.0f, splashY, 0.0f, NULL, 1.0f, 1.0f);
            }
            if (splashTimer > 1.2f && transpar != 0) {
                transpar -= 5;
                C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, transpar));
            }
            if (splashTimer > 3.5f) {
                if (transpar2 != 255){
                    transpar2 += 5;
                }
                C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, transpar2));
            }

            C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
            C2D_SceneBegin(bottom);
        } else {
            splashDone = true;
        }
    } else {
        sceneManagerSwitchTo(SCENE_TITLE);
        splashPlayed = false;
    }
}

void sceneLogoExit(void) {
    // Free resources if needed
}