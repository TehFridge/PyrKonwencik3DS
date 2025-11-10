#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include "scene_title.h"
#include "scene_manager.h"
// #include "main.h"
#include "cwav_shit.h"
#include "global_draws.h"
#include "sprites.h"
extern CWAVInfo cwavList[8];
CWAV* menu;

void sceneTitleInit(void) {
    menu = cwavList[1].cwav;
    menu->volume = 0.6f;
}

void sceneTitleUpdate(uint32_t kDown, uint32_t kHeld) {
    time_wave += 0.1f;
    updateWave(time_wave);
    if (!splashPlayed) {
        cwavPlay(menu, 0, 1);
        splashPlayed = true;
    }
    timer += 0.2f ;
    if (timer > 7.0f) {
        isScrolling = true;
    }

    if (isScrolling) {
        if (elapsed < duration) {
            currentY = easeOutQuad(elapsed, startY, endY, duration);
            elapsed += deltaTime;
        }
    }
    if (kDown & KEY_A) {
        if (timer > 7.0f) {
            //cwavPlay(sfx, 0, 1);
            timer = 0.0f;
            transpar = 255;
            Scene = 2;
            sceneManagerSwitchTo(SCENE_MAIN_MENU);
        }
    }
}
void sceneTitleRender(void) {
    C2D_TargetClear(top, C2D_Color32f(1.0f, 1.0f, 1.0f, 1.0f));
    C2D_SceneBegin(top);
 
    C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
    C2D_TargetClear(bottom, C2D_Color32f(1.0f, 1.0f, 1.0f, 1.0f));
    C2D_SceneBegin(bottom);
	
    drawShadowedText(&g_staticText[0], 160.0f, -currentY + 100.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
}

void sceneTitleExit(void) {
    // Free resources if needed
}