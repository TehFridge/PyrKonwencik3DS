#include <3ds.h>
#include <3ds/types.h>
#include <3ds/synchronization.h>
#include <citro2d.h>
#include <math.h>
#include "scene_program.h"
#include "scene_manager.h"
// #include "main.h"
#include "cwav_shit.h"
#include "global_draws.h"
#include "global_parsing.h"
#include "sprites.h"
#include "buttons.h"
extern CWAVInfo cwavList[8];
extern Button buttonsy[2000];
static SwkbdState swkbd;
static char mybuf[256];
static char mybuf2[256];
static SwkbdStatusData swkbdStatus;
static SwkbdLearningData swkbdLearning;
SwkbdButton button = SWKBD_BUTTON_NONE;
bool didit = false;
bool swkbdTriggered = false;
void sceneProgramInit(void) {

}

void sceneProgramUpdate(uint32_t kDown, uint32_t kHeld) {
    //updateBubbles();
    time_wave += 0.1f;
    updateWave(time_wave);
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
    if (kDown & KEY_Y) {
        if (Scene == 4) {
            swkbdTriggered = true;
            didit = true;
            swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
            swkbdSetInitialText(&swkbd, mybuf);
            swkbdSetHintText(&swkbd, "Co chcesz wyszukać?");
            swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
            swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
            button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
            search_entries(mybuf);
            swkbdTriggered = false;
            load_search_page();
            currentday = 4;
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
        if (Scene == 4) {
            if (currentday == 0 && offset_friday != 0) {
                cwavPlay(sfx, 0, 1);
                offset_friday -= 6;
                load_friday_page();
            } else if (currentday == 1 && offset_saturday != 0) {
                cwavPlay(sfx, 0, 1);
                offset_saturday -= 6;
                load_saturday_page();
            } else if (currentday == 2 && offset_sunday != 0) {
                cwavPlay(sfx, 0, 1);
                offset_sunday -= 6;
                load_sunday_page();
            } else if (currentday == 3 && offset_ulub != 0) {
                cwavPlay(sfx, 0, 1);
                offset_ulub -= 6;
                load_ulubione_buttons("/3ds/ulubione.json");
            } else if (currentday == 4 && offset_search != 0) {
                cwavPlay(sfx, 0, 1);
                offset_search -= 6;
                load_search_page();
            }
        }
    }
    if (kDown & KEY_R) {
        if (Scene == 4) {
            if (can_further) {
                if (currentday == 0) {
                    offset_friday += 6;
                    cwavPlay(sfx, 0, 1);
                    load_friday_page();
                } else if (currentday == 1) {
                    offset_saturday += 6;
                    cwavPlay(sfx, 0, 1);
                    load_saturday_page();
                } else if (currentday == 2) {
                    offset_sunday += 6;
                    cwavPlay(sfx, 0, 1);
                    load_sunday_page();
                } else if (currentday == 3) {
                    offset_ulub += 6;
                    cwavPlay(sfx, 0, 1);
                    load_ulubione_buttons("/3ds/ulubione.json");
                } else if (currentday == 4) {
                    offset_search += 6;
                    cwavPlay(sfx, 0, 1);
                    load_search_page();
                }
            }
        }
    }
    if (kDown & KEY_DRIGHT) {
        if (Scene == 4 && currentday != 3 && currentday != 4) {
            cwavPlay(sfx, 0, 1);
            currentday += 1;
            if (currentday == 1) {
                load_saturday_page();
            } else if (currentday == 2) {
                load_sunday_page();
            } else if (currentday == 3) {
                load_ulubione_buttons("/3ds/ulubione.json");
            }
        }
    }
    if (kDown & KEY_DLEFT) {
        if (Scene == 4 && currentday != 0) {
            cwavPlay(sfx, 0, 1);
            currentday -= 1;
            if (currentday == 0) {
                load_friday_page();
            } else if (currentday == 1) {
                load_saturday_page();
            } else if (currentday == 2) {
                load_sunday_page();
            } else if (currentday == 3) {
                currentday -= 1;
                load_sunday_page();
            }
        }
    }
}
void sceneProgramRender(void) {

    C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(top);
    drawKwadraty();

    
    //C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
    drawWaveFill();
    drawBubblesTop();
    float delta = 1.0f / 60.0f; // Approx. 60 FPS
    tajmer += delta;

    float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
    drawShadowedText(&g_staticText[15], 200.0f, 150.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    drawShadowedText(&g_staticText[9], 200.0f, 170.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    drawShadowedText(&g_staticText[8], 200.0f, 190.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    drawShadowedText(&g_staticText[7], 200.0f, 210.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    if (currentday == 0) {
        drawShadowedText(&g_staticText[4], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    } else if (currentday == 1) {
        drawShadowedText(&g_staticText[5], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    } else if (currentday == 2) {
        drawShadowedText(&g_staticText[6], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    } else if (currentday == 3) {
        drawShadowedText(&g_staticText[12], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    } else if (currentday == 4) {
        drawShadowedText(&g_staticText[14], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
        C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    }
    C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(bottom);
    
    drawKwadraty();
    C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
    drawBubblesBottom();
    for (int i = 0; i < 810; i++) {
        drawButton(&buttonsy[i], Scene);
    }

    for (int i = 0; i < 6; i++) {
        float buttonX = 5.0f;
        float buttonY = 30.0f + (i * 76.0f) - textOffset;
        buttonsy[i + 3].x = buttonX;
        buttonsy[i + 3].y = buttonY;
        float textX = buttonX + 155.0f;
        float textY = buttonY + 33.0f;

        float textWidth, textHeight;
        C2D_TextGetDimensions(&g_entry_nameText[i], 0.5f, 0.5f, &textWidth, &textHeight);
        textY -= textHeight / 2.0f;

        drawShadowedText(&g_entry_nameText[i], textX, textY, 0.5f, 0.5f, 0.5f,
                            C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
        //printf("buttonsy[%d] = (%f, %f)\n", i + 3, buttonsy[i + 3].x, buttonsy[i + 3].y);
    }
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));		

}

void sceneProgramExit(void) {
    // Free resources if needed
}