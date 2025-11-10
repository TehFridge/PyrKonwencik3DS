#include <citro2d.h>
#include <time.h>
#include "sprites.h"
#include "global_parsing.h"
#include "cwav_shit.h"
//----------------------------------------
C2D_Image couponbutton_pressed;
C2D_Image couponbutton;
C2D_Image entry_pressed;
C2D_Image entrybutton;
static C2D_SpriteSheet background_top, background_down, logo, buttons, settings, scan, points, coupons, qrframe, stat, zappbar, couponenbuttonen, goback, act_buttons, deact_buttons, more_b, logout_buttons, themename_border, too_less, splash;
C2D_Image bgtop, bgdown, logo3ds, buttonsmol, buttonmed, buttonbeeg;
C2D_SpriteSheet entry_sheet;
C2D_SpriteSheet mapas;
C2D_SpriteSheet mapas1;
C2D_SpriteSheet mapas2;
C2D_SpriteSheet mapas3;
C2D_SpriteSheet programb;
C2D_Image splash1;
C2D_Image splash2;	
C2D_Image progbutton; 
C2D_Image prog_pressed; 
C2D_Image mapa1; 
C2D_Image mapa2;  
C2D_Image mapa3;  
C2D_Image mapa4;
C2D_SpriteSheet scrollbarsheet;
C2D_Image scrollbar;
extern CWAVInfo cwavList[8];

CWAV* sfx;
void spritesInit(){
    sfx = cwavList[2].cwav;
    time_t now = time(NULL); // Current date and time
    struct tm *today = localtime(&now);

    // Create a target date (e.g., December 31, 2025)
    time_t future = createDate(2026, 06, 19);

    double seconds = difftime(future, now);
	days = (int)ceil(seconds / (60 * 60 * 24));
	if (days < 0) {
		days = 0;
	}
    g_staticBuf = C2D_TextBufNew(256);

    C2D_TextParse(&g_staticText[0], g_staticBuf, "Wciśnij A.");
    C2D_TextOptimize(&g_staticText[0]); 
	char dayText[64];
	snprintf(dayText, sizeof(dayText), "%d", days);
	if (days != 0) {
		C2D_TextParse(&g_staticText[1], g_staticBuf, dayText);
		C2D_TextParse(&g_staticText[2], g_staticBuf, "Dni");
		C2D_TextParse(&g_staticText[3], g_staticBuf, "Pozostało:");
	} else {
		C2D_TextParse(&g_staticText[1], g_staticBuf, "ITZ PYRKA TIME");
	}
	C2D_TextParse(&g_staticText[4], g_staticBuf, "Piątek");
	C2D_TextParse(&g_staticText[5], g_staticBuf, "Sobota");
	C2D_TextParse(&g_staticText[6], g_staticBuf, "Niedziela");
	C2D_TextParse(&g_staticText[7], g_staticBuf, "L/R - Wstecz/Dalej");
	C2D_TextParse(&g_staticText[8], g_staticBuf, "DPad Left/Right - Zmień Dzień");
	C2D_TextParse(&g_staticText[9], g_staticBuf, "B - Powrót");
	C2D_TextParse(&g_staticText[10], g_staticBuf, "Touch Screen - Poruszaj mapą");
	C2D_TextParse(&g_staticText[11], g_staticBuf, "L/R - Zoom");
	C2D_TextParse(&g_staticText[12], g_staticBuf, "Ulubione");
	C2D_TextParse(&g_staticText[13], g_staticBuf, "Select - Polub/Odlub");
	C2D_TextParse(&g_staticText[14], g_staticBuf, "Szukane");
	C2D_TextParse(&g_staticText[15], g_staticBuf, "Y - Wyszukaj");
	splash = C2D_SpriteSheetLoad("romfs:/gfx/splash.t3x");
	background_top = C2D_SpriteSheetLoad("romfs:/gfx/bg.t3x");
	logo = C2D_SpriteSheetLoad("romfs:/gfx/logo.t3x");
	couponenbuttonen = C2D_SpriteSheetLoad("romfs:/gfx/coupon_button_machen.t3x");
	entry_sheet = C2D_SpriteSheetLoad("romfs:/gfx/entry.t3x");
	mapas = C2D_SpriteSheetLoad("romfs:/gfx/mapa.t3x");
	mapas1 = C2D_SpriteSheetLoad("romfs:/gfx/mapa1.t3x");
	mapas2 = C2D_SpriteSheetLoad("romfs:/gfx/mapa2.t3x");
	mapas3 = C2D_SpriteSheetLoad("romfs:/gfx/mapa3.t3x");
	programb = C2D_SpriteSheetLoad("romfs:/gfx/program.t3x");
	bgtop = C2D_SpriteSheetGetImage(background_top, 0);
	logo3ds = C2D_SpriteSheetGetImage(logo, 0);
	splash1 = C2D_SpriteSheetGetImage(splash, 0);
	splash2 = C2D_SpriteSheetGetImage(splash, 1);	
	couponbutton = C2D_SpriteSheetGetImage(couponenbuttonen, 0); 
	couponbutton_pressed = C2D_SpriteSheetGetImage(couponenbuttonen, 1); 
	entrybutton = C2D_SpriteSheetGetImage(entry_sheet, 0); 
	entry_pressed = C2D_SpriteSheetGetImage(entry_sheet, 1); 
	progbutton = C2D_SpriteSheetGetImage(programb, 0); 
	prog_pressed = C2D_SpriteSheetGetImage(programb, 1); 
	mapa1 = C2D_SpriteSheetGetImage(mapas, 0); 
	mapa2 = C2D_SpriteSheetGetImage(mapas1, 0);  
	mapa3 = C2D_SpriteSheetGetImage(mapas2, 0);  
	mapa4 = C2D_SpriteSheetGetImage(mapas3, 0); 
	entry_name_Buf = C2D_TextBufNew(5096);
	description_Buf = C2D_TextBufNew(5096);
	loc_Buf = C2D_TextBufNew(5096);
	prow_Buf = C2D_TextBufNew(5096);
}