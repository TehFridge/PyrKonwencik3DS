#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include "scene_mapa.h"
#include "scene_manager.h"
#include "buttons.h"
// #include "main.h"
#include "cwav_shit.h"
#include "global_draws.h"
#include "sprites.h"

extern Button buttonsy[2000];

void sceneMapaInit(void) {

}

void sceneMapaUpdate(uint32_t kDown, uint32_t kHeld) {
    //time += 0.1f;
    //updateWave(time);
    timer += 0.2f ;
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
    if (kDown & KEY_B) {
        if ((Scene == 3 || Scene == 4) && timer > 7.0f) {
            cwavPlay(sfx, 0, 1);
            timer = 0.0f;
            transpar = 255;
            Scene = 2;
            sceneManagerSwitchTo(SCENE_MAIN_MENU);
        }
    }
    if (kDown & KEY_L) {
        if (Scene == 3 && timer > 7.0f && scalemapa != 0.0f) {
            scalemapa -= 0.1f;
        }
    }
    if (kDown & KEY_R) {
        if (Scene == 3 && timer > 7.0f) {
            scalemapa += 0.1f;
        }
    }
}
void sceneMapaRender(void) {

    C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(top);
    drawKwadraty();

    float centerX = 400.0f / 3.7f;
    float centerY = 240.0f / 5.0f;
    float drawX1 = (0.0f - textOffsetX) * scalemapa + centerX;
    float drawY1 = (0.0f - textOffset)  * scalemapa + centerY;

    float drawX2 = (1000.0f - textOffsetX) * scalemapa + centerX;
    float drawY2 = (0.0f    - textOffset)  * scalemapa + centerY;

    float drawX3 = (0.0f - textOffsetX) * scalemapa + centerX;
    float drawY3 = (708.0f - textOffset) * scalemapa + centerY;

    float drawX4 = (1000.0f - textOffsetX) * scalemapa + centerX;
    float drawY4 = (708.0f  - textOffset)  * scalemapa + centerY;

    C2D_DrawImageAt(mapa1, drawX1, drawY1, 0.0f, NULL, scalemapa, scalemapa);
    C2D_DrawImageAt(mapa2, drawX2, drawY2, 0.0f, NULL, scalemapa, scalemapa);
    C2D_DrawImageAt(mapa3, drawX3, drawY3, 0.0f, NULL, scalemapa, scalemapa);
    C2D_DrawImageAt(mapa4, drawX4, drawY4, 0.0f, NULL, scalemapa, scalemapa);
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    
    C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(bottom);
    
    drawKwadraty();
    C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
    for (int i = 0; i < 100; i++) {
        drawButton(&buttonsy[i], Scene);
    }
    float delta = 1.0f / 60.0f; // Approx. 60 FPS
    tajmer += delta;

    float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
    drawShadowedText(&g_staticText[9], 160.0f, 40.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
    drawShadowedText(&g_staticText[10], 160.0f, 60.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
    drawShadowedText(&g_staticText[11], 160.0f, 80.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
}

void sceneMapaExit(void) {
    // Free resources if needed
}