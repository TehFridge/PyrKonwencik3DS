
#ifndef SPRITES_H
#define SPRITES_H
#include "cwav_shit.h"
extern C2D_TextBuf g_staticBuf;
extern C2D_Text g_staticText[100];
extern C2D_Image couponbutton_pressed;
extern C2D_Image couponbutton;
extern C2D_Image entry_pressed;
extern C2D_Image entrybutton;
extern C2D_Image bgtop, bgdown, logo3ds, buttonsmol, buttonmed, buttonbeeg;
extern C2D_SpriteSheet entry_sheet;
extern C2D_SpriteSheet mapas;
extern C2D_SpriteSheet mapas1;
extern C2D_SpriteSheet mapas2;
extern C2D_SpriteSheet mapas3;
extern C2D_SpriteSheet programb;
extern C2D_Image logo3ds;
extern C2D_Image splash1;
extern C2D_Image splash2;	
extern C2D_Image progbutton; 
extern C2D_Image prog_pressed; 
extern C2D_Image mapa1; 
extern C2D_Image mapa2;  
extern C2D_Image mapa3;  
extern C2D_Image mapa4; 
extern C2D_SpriteSheet scrollbarsheet;
extern C2D_Image scrollbar;
extern CWAV* sfx;
void spritesInit();
#endif