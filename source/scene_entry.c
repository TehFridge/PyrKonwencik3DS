#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include "scene_entry.h"
#include "scene_manager.h"
// #include "main.h"
#include "cwav_shit.h"
#include "global_draws.h"
#include "global_parsing.h"
#include "sprites.h"
#include "buttons.h"
extern CWAVInfo cwavList[8];
extern Button buttonsy[2000];

void sceneEntryInit(void) {

}

void sceneEntryUpdate(uint32_t kDown, uint32_t kHeld) {
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
    if (kDown & KEY_B) {
        if (Scene == 8 && timer > 7.0f && currentday != 3) {
            cwavPlay(sfx, 0, 1);
            timer = 0.0f;
            transpar = 255;
            Scene = 4;
            sceneManagerSwitchTo(SCENE_PROGRAM);
        } else if (Scene == 8 && timer > 7.0f && currentday == 3) {
            cwavPlay(sfx, 0, 1);
            timer = 0.0f;
            transpar = 255;
            load_ulubione_buttons("/3ds/ulubione.json");
            Scene = 4;	
            sceneManagerSwitchTo(SCENE_PROGRAM);
        }
    }

    if (kDown & KEY_L) {
        if (Scene == 8 && has_sec_page && descpage != 0) {
            cwavPlay(sfx, 0, 1);
            descpage -= 1;
        }
    }
    if (kDown & KEY_SELECT) {
        if (Scene == 8) {
            const char* current_title = tytul_table[offset_caly];
            int index = -1;

            // Check if title is already in favorites
            for (int i = 0; i < fav_count; i++) {
                if (strcmp(favorites[i], current_title) == 0) {
                    index = i;
                    break;
                }
            }

            if (index != -1) {
                // Remove favorite
                for (int i = index; i < fav_count - 1; i++) {
                    strcpy(favorites[i], favorites[i + 1]);
                }
                fav_count--;
            } else {
                // Add new favorite
                if (fav_count < MAX_ENTRIES) {
                    strncpy(favorites[fav_count], current_title, sizeof(favorites[fav_count]) - 1);
                    favorites[fav_count][sizeof(favorites[fav_count]) - 1] = '\0';
                    fav_count++;
                }
            }

            save_favorites(favorites, fav_count, "/3ds/ulubione.json");
        }
    }

    if (kDown & KEY_R) {
        if (Scene == 8 && has_sec_page && descpage != 2) {
            cwavPlay(sfx, 0, 1);
            descpage += 1;
        }
    }
}
void sceneEntryRender(void) {

    C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(top);
    drawKwadraty();

    float delta = 1.0f / 60.0f; // Approx. 60 FPS
    tajmer += delta;

    float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
    drawShadowedText(&g_staticText[9], 50.0f, 10.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    if (has_sec_page) {
        drawShadowedText(&g_staticText[7], 200.0f, 10.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    }
    drawShadowedText(&g_staticText[13], 340.0f, 13.0f + yOffset, 0.5f, 0.45f, 0.45f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);

    drawShadowedText(&prow_Text[0], 200.0f, 80.0f + yOffset, 0.5f, 1.2f, 1.2f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    drawShadowedText(&prow_Text[1], 200.0f, 30.0f + yOffset, 0.5f, 0.9f, 0.9f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    if (!has_sec_page) {
        drawShadowedText(&prow_Text[2], 200.0f, 10.0f + yOffset, 0.5f, 0.7f, 0.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), true);
    }
    // drawShadowedText(&g_staticText[2], 202.5f, 140.0f + yOffset, 0.5f, 0.7f, 0.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
    // drawShadowedText(&g_staticText[3], 205.0f, 40.0f + yOffset, 0.5f, 1.4f, 1.4f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
    C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(bottom);
    
    drawKwadraty();
    C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
    //drawBubblesBottom();
    for (int i = 0; i < 810; i++) {
        drawButton(&buttonsy[i], Scene);
    }
    drawShadowedText(&description_Text[selectioncodelol - 3 + descpage], 160.0f, 70.0f - textOffset, 0.5f, 0.5f, 0.5f,
                    C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
    float originalWidth;
    C2D_TextGetDimensions(&loc_Text[selectioncodelol - 3], 1.0f, 1.0f, &originalWidth, NULL);  // Only need width

    float maxWidth = 300.0f;
    float scale = maxWidth / originalWidth;
    drawShadowedText(&loc_Text[selectioncodelol - 3], 160.0f, 10.0f - textOffset, 0.5f, scale, scale,
                    C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff), false);
    C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
}

void sceneEntryExit(void) {
    // Free resources if needed
}