#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include "scene_mainmenu.h"
#include "scene_manager.h"
// #include "main.h"
#include "cwav_shit.h"
#include "buttons.h"
#include "global_draws.h"
#include "global_parsing.h"
#include "sprites.h"

extern Button buttonsy[2000];

void sceneMainMenuInit(void) {

}

void sceneMainMenuUpdate(uint32_t kDown, uint32_t kHeld) {
    time_wave += 0.1f;
    updateWave(time_wave);
    timer += 0.2f;
    if (timer > 20.0f) {
        isScrolling = true;
    }
    if (transpar != 0) {
        transpar -= 15;
    }
    if (isScrolling) {
        if (elapsed < duration) {
            currentY = easeOutQuad(elapsed, startY, endY, duration);
            elapsed += deltaTime;
        }
    }

}
void sceneMainMenuRender(void) {

    C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(top);

    drawKwadraty();
    drawWaveFill();
    drawBubblesTop();
    float delta = 1.0f / 60.0f; // Approx. 60 FPS
    tajmer += delta;

    float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
    if (days != 0) {
        drawShadowedText(&g_staticText[1], 205.0f, 80.0f + yOffset, 0.5f, 2.3f, 2.3f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        drawShadowedText(&g_staticText[2], 202.5f, 140.0f + yOffset, 0.5f, 0.7f, 0.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        drawShadowedText(&g_staticText[3], 205.0f, 40.0f + yOffset, 0.5f, 1.4f, 1.4f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    } else {
        drawShadowedText(&g_staticText[1], 205.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    }
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(bottom);
    
    drawKwadraty();
    C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
    drawBubblesBottom();
    for (int i = 0; i < 100; i++) {
        drawButton(&buttonsy[i], Scene);
    }
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));				
}

void sceneMainMenuExit(void) {
    // Free resources if needed
}