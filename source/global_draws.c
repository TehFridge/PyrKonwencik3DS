
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "global_draws.h"
#include "sprites.h"
int Scene = 0;
int selectioncodelol;
int selectionthemelol;
float x = 0.0f;
float y = 0.0f;
int transpar = 255;
int transpar2 = 0;
float timer = 0.0f;
float timer2 = 255.0f;
float timer3 = 0.0f;
float tajmer;
float time_wave = 0.0f;
float currentY = -400.0f;
float currentY2 = -400.0f;

float startY = -400.0f;    
float endY = 0.0f;    
float duration = 7.0f; 
float elapsed = 0.0f;   
float deltaTime = 0.1f; 

bool splashPlayed = false;
u64 splashStartTime = 0;
bool splashDone = false;
float splashTimer = 0.0f;
float splashY = 240.0f;       // Start below screen
float splashHopTime = 0.0f;
const float splashHopDuration = 0.8f; // Total hop time
C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
typedef struct {
    float x, y;
    float speed;
    float radius;
    bool onTopScreen;
} Bubble;
bool isScrolling;
bool isLogged;
int textOffset = 0;
int textOffsetX = 0;
float scalemapa = 0.5f;
float easeInQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (t * t);
}
float easeOutQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (1 - (1 - t) * (1 - t));
}
float easeHop(float t, float start, float end, float duration) {
    float s = 1.70158f * 1.5f; // overshoot factor
    t /= duration;
    t -= 1.0f;
    return (end - start) * (t * t * ((s + 1) * t + s) + 1.0f) + start;
}
void drawShadowedText(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor, bool shadow_border) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };
	if (shadow_border) {
		for (int i = 0; i < 4; i++) {
			C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor,
						x + shadowOffsets[i][0], y + shadowOffsets[i][1],
						depth, scaleX, scaleY, shadowColor);
		}
	} else {
		C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y + 1.35, depth, scaleX, scaleY, color);
	}
	if (shadow_border) {
    	C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y, depth, scaleX, scaleY, color);
	} else {
		C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y, depth, scaleX, scaleY, shadowColor);
	}
}


void drawShadowedTextWrapped(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };

    for (int i = 0; i < 4; i++) {
        C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor,
                     x + shadowOffsets[i][0], y + shadowOffsets[i][1],
                     depth, scaleX, scaleY, shadowColor, 300.0f);
    }

    C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y, depth, scaleX, scaleY, color, 300.0f);
}
void drawShadowedText_noncentered(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };

    for (int i = 0; i < 4; i++) {
        C2D_DrawText(text, C2D_WithColor,
                     x + shadowOffsets[i][0], y + shadowOffsets[i][1],
                     depth, scaleX, scaleY, shadowColor);
    }

    C2D_DrawText(text, C2D_WithColor, x, y, depth, scaleX, scaleY, color);
}
#define MAX_BUBBLES 37
Bubble bubbles[MAX_BUBBLES];

void initBubbles() {
    for (int i = 0; i < MAX_BUBBLES; i++) {
        bubbles[i].x = rand() % 340;
        bubbles[i].y = 240;
        bubbles[i].speed = 0.5f + (rand() % 5) * 0.1f;
        bubbles[i].radius = 5 + (rand() % 10);
        bubbles[i].onTopScreen = false;
    }
}

void updateBubbles() {
    for (int i = 0; i < MAX_BUBBLES; i++) {
        bubbles[i].y -= bubbles[i].speed;

        // Switch to top screen when off bottom screen
        if (!bubbles[i].onTopScreen && bubbles[i].y < -bubbles[i].radius) {
            bubbles[i].onTopScreen = true;
            bubbles[i].y = 240;  // Start from bottom of top screen
        }

        // Reset completely when reaching a certain height on top screen
        if (bubbles[i].onTopScreen && bubbles[i].y < 120.0f) {
            bubbles[i].y = 270;
            bubbles[i].x = rand() % 340;
            bubbles[i].speed = 0.5f + (rand() % 5) * 0.1f;
            bubbles[i].radius = 5 + (rand() % 10);
            bubbles[i].onTopScreen = false;
        }
    }
}

void drawBubblesTop() {
    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (bubbles[i].onTopScreen) {
            float alpha = (bubbles[i].y - 120.0f) / (240.0f - 120.0f); // 1.0 at y=240, 0.0 at y=120
            if (alpha < 0.0f) alpha = 0.0f;
            if (alpha > 1.0f) alpha = 1.0f;
            u32 bubbleColor = C2D_Color32(137, 90, 164, (u8)(alpha * 129)); // original alpha was 129

            C2D_DrawCircle(bubbles[i].x + 40.0f, bubbles[i].y, 0.0f, bubbles[i].radius,
                bubbleColor, bubbleColor, bubbleColor, bubbleColor);
        }
    }
}

void drawBubblesBottom() {
    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (!bubbles[i].onTopScreen) {
            u32 bubbleColor = C2D_Color32(137, 90, 164, 129);
            C2D_DrawCircle(bubbles[i].x, bubbles[i].y, 0.0f, bubbles[i].radius,
                bubbleColor, bubbleColor, bubbleColor, bubbleColor);
        }
    }
}

typedef struct {
    float x, y;
} WavePoint;

WavePoint wave[NUM_POINTS];
float phaseOffsets[NUM_POINTS]; // for local randomness
void drawKwadraty(){
    //updateBubbles();
    if (y > -40.0f) {
        x -= 0.25f;
        y -= 0.25f;
    } else {
        x = 0.0f;
        y = 0.0f;
    }

    C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
}
void initWaveOffsets() {
    srand(time(NULL)); // seed RNG
    for (int i = 0; i < NUM_POINTS; ++i) {
        phaseOffsets[i] = ((rand() % 1000) / 1000.0f) * 2.0f * M_PI;
    }
}

void updateWave(float time) {
    const float t1 = time * 20.0f;
    const float t2 = time * 12.0f;
    const float t3 = time * 7.0f;
    const float waveFreq2 = WAVE_FREQUENCY * 2.0f;
    const float waveFreq05 = WAVE_FREQUENCY * 0.5f;

    for (int i = 0; i < NUM_POINTS; ++i) {
        float norm = (float)i / (NUM_POINTS - 1);
        float x = norm * SCREEN_WIDTH;
        float phase = x + phaseOffsets[i];

        float y = BASE_HEIGHT
                + sinf(WAVE_FREQUENCY * (phase + t1)) * WAVE_AMPLITUDE
                + sinf(waveFreq2 * (x + t2)) * (WAVE_AMPLITUDE * 0.5f)
                + sinf(waveFreq05 * (x + t3)) * (WAVE_AMPLITUDE * 0.3f);

        y += (((float)(rand() % 100) / 100.0f) - 0.5f) * 1.5f; // organic jitter

        wave[i].x = x;
        wave[i].y = y;
    }
}

void drawWaveFill() {
    u32 fillColor = C2D_Color32(76, 25, 102, 220);

    for (int i = 1; i < NUM_POINTS; ++i) {
        float x0 = wave[i - 1].x, y0 = wave[i - 1].y;
        float x1 = wave[i].x,     y1 = wave[i].y;

        // One quad split into two triangles
        C2D_DrawTriangle(x0, y0, fillColor,
                         x1, y1, fillColor,
                         x0, SCREEN_HEIGHT, fillColor, 0);

        C2D_DrawTriangle(x1, y1, fillColor,
                         x1, SCREEN_HEIGHT, fillColor,
                         x0, SCREEN_HEIGHT, fillColor, 0);
    }
}