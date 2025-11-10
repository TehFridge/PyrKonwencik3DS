#include <citro2d.h>
#include <citro3d.h>

#ifndef GLOBAL_DRAWS_H
#define GLOBAL_DRAWS_H

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

#define BASE_HEIGHT (SCREEN_HEIGHT / 2)
#define NUM_POINTS 15
#define WAVE_AMPLITUDE 10.0f
#define WAVE_FREQUENCY 0.025f
extern int Scene;
extern int selectioncodelol;
extern int selectionthemelol;
extern C3D_RenderTarget* top;
extern C3D_RenderTarget* bottom;
extern int transpar;
extern int transpar2;
extern float timer;
extern float timer2;
extern float timer3;
extern float tajmer;
extern float time_wave;
extern float currentY;
extern float currentY2;
extern bool splashPlayed;
extern u64 splashStartTime;
extern bool splashDone;
extern float splashTimer;
extern float splashY;       // Start below screen
extern float splashHopTime;
extern const float splashHopDuration; // Total hop time
extern float startY;    
extern float endY;    
extern float duration; 
extern float elapsed;   
extern float deltaTime; 
extern bool isScrolling;
extern bool isLogged;
extern int textOffset;
extern int textOffsetX;
extern float scalemapa;
float easeInQuad(float t, float start, float end, float duration);
float easeOutQuad(float t, float start, float end, float duration);
float easeHop(float t, float start, float end, float duration);
void drawShadowedText(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor, bool shadow_border);
void drawShadowedTextWrapped(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor);
void drawShadowedText_noncentered(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor);
void initBubbles();
void updateBubbles();
void drawBubblesTop();
void drawBubblesBottom();
void drawKwadraty();
void initWaveOffsets();
void updateWave(float time);
void drawWaveFill();
#endif