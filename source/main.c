#include <citro2d.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <3ds.h>
#include <3ds/types.h>
#include <3ds/synchronization.h>
#include <cwav.h>
#include <ncsnd.h>
#include <curl/curl.h>
#include <malloc.h>
#include <jansson.h>
#include <unistd.h>
#include "buttons.h"
#include "cwav_shit.h"
#include "request.h"
#include <ctype.h>
#include "global_draws.h"
#include "global_parsing.h"
#include "scene_manager.h"
#include "sprites.h"
#define MAX_SPRITES   1
#define fmin(a, b) ((a) < (b) ? (a) : (b))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x300000







size_t totalsajz = 0;
bool cpu_debug = false;

extern CWAVInfo cwavList[8];
extern int cwavCount;
char combinedText[128]; 
float speed = 20.0f; // High speed for fast reset

FILE *fptr;
static u32 *SOC_buffer = NULL;


bool is_network_connected() {
    if (R_FAILED(acInit())) {
        return false;
    }
    u32 status;
    bool connected = false;
    
    if (R_SUCCEEDED(ACU_GetStatus(&status))) {
        connected = (status == 3);
    }
    acExit();
    
    return connected;
}

extern Button buttonsy[2000];



void load_current_program() {
    refresh_data("https://core.pyrkon.pl/wp-json/pyrkon/v1/planner-items-search?type=standard&selectedlang=all&time=&offset=1&lang=pl&site_id=12&api_token=1c57cd904562dc3691b101d2c338f484&offset=0&limit=2000", "", NULL);

    // json_error_t error;
    // json_t *root = json_loads(global_response.data, 0, &error);

    // if (!root) {
    //     printf("JSON parse error: %s\n", error.text);
    //     return;
    // }

    // json_dump_file(root, "/3ds/my_pyrkon.json", 0);
    // json_decref(root);
}
// Function to remove all "<br />\r\n" tags from a string and replace with a newline


extern bool czasfuckup;

C2D_TextBuf totpBuf = NULL;
C2D_Text g_totpText[5];  // Just declared, will be initialized when needed
// extern bool logplz;
// extern float text_w, text_h;
// extern float max_scroll;
const char *nejmenmachen;
int amountzappsy;

char themes[100][100];
struct memory {
  char *response;
  size_t size;
};



void createDirectory(const char* dirPath) {
    FS_Path fsPath = fsMakePath(PATH_ASCII, dirPath);
    FS_Archive sdmcArchive;
    Result rc = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
    if (R_FAILED(rc)) {
        printf("Failed to open SDMC archive: 0x%08lX\n", rc);
        return;
    }
    rc = FSUSER_CreateDirectory(sdmcArchive, fsPath, 0);
    if (R_FAILED(rc)) {
        printf("Failed to create directory '%s': 0x%08lX\n", dirPath, rc);
    } else {
        printf("Directory '%s' created successfully.\n", dirPath);
    }
    FSUSER_CloseArchive(sdmcArchive);
}


typedef struct {
    C2D_Sprite spr;
    float dx, dy;
} Sprite;

C2D_TextBuf g_staticBuf;
C2D_Text g_staticText[100];
C2D_TextBuf themeBuf;
C2D_Text themeText[100];
C2D_Font font[1];


static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES / 2;

static size_t cb(void *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)clientp;
 
  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(ptr == NULL)
    return 0;  /* out of memory! */
 
  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
 
  return realsize;
}

// void rebuild_buffer() {
	// C2D_TextBufClear(g_staticBuf);
    // C2D_TextFontParse(&g_staticText[0], font[0], g_staticBuf, "Wciśnij A.");
    // C2D_TextOptimize(&g_staticText[0]); 
	// C2D_TextFontParse(&g_staticText[1], font[0], g_staticBuf, "Ładowanie...");
	// C2D_TextOptimize(&g_staticText[1]); 
	// C2D_TextFontParse(&g_staticText[2], font[0], g_staticBuf, "Nie wykryto danych konta Żappka.\nWciśnij A by kontynuować");
	// C2D_TextOptimize(&g_staticText[2]); 
	// C2D_TextFontParse(&g_staticText[3], font[0], g_staticBuf, "Tak"); 
	// C2D_TextFontParse(&g_staticText[4], font[0], g_staticBuf, "Nie");
	// C2D_TextFontParse(&g_staticText[5], font[0], g_staticBuf, "Wprowadź numer telefonu.");
	// C2D_TextOptimize(&g_staticText[5]);
	// C2D_TextFontParse(&g_staticText[6], font[0], g_staticBuf, "Wprowadź kod SMS.");
	// C2D_TextOptimize(&g_staticText[6]);
	// C2D_TextFontParse(&g_staticText[7], font[0], g_staticBuf, combinedText);
    // C2D_TextFontParse(&g_staticText[8], font[0], g_staticBuf, "Twoje Żappsy");
    // C2D_TextOptimize(&g_staticText[8]); 
	// C2D_TextFontParse(&g_staticText[9], font[0], g_staticBuf, zappsystr);
	// C2D_TextOptimize(&g_staticText[9]); 
    // C2D_TextFontParse(&g_staticText[10], font[0], g_staticBuf, "B - Powrót");
    // C2D_TextOptimize(&g_staticText[10]);
    // C2D_TextFontParse(&g_staticText[11], font[0], g_staticBuf, "Brak internetu :(");
    // C2D_TextOptimize(&g_staticText[11]);
	// C2D_TextFontParse(&g_staticText[12], font[0], g_staticBuf, "Kupony");
    // C2D_TextOptimize(&g_staticText[12]);
	// C2D_TextFontParse(&g_staticText[13], font[0], g_staticBuf, "Gotowe :)");
    // C2D_TextOptimize(&g_staticText[13]);
	// C2D_TextFontParse(&g_staticText[14], font[0], g_staticBuf, "Opcje");
    // C2D_TextOptimize(&g_staticText[14]);
	// C2D_TextFontParse(&g_staticText[15], font[0], g_staticBuf, "Motyw:");
    // C2D_TextOptimize(&g_staticText[15]);
	// C2D_TextFontParse(&g_staticText[16], font[0], g_staticBuf, "(Zrestartuj aplikacje by zapisać zmiany)");
    // C2D_TextOptimize(&g_staticText[16]);
// }

extern char tileNames[100][256];


void chuj() {
	printf("chuj..");
}

void executeButtonFunction(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < 100 && buttonsy[buttonIndex].onClick != NULL) {
        buttonsy[buttonIndex].onClick();
    } else {
        printf("Invalid button index or function not assigned!\n");
    }
}

void mapa(){
	transpar = 255;
	textOffsetX = 0;
	textOffset = 0;
	Scene = 3;
	sceneManagerSwitchTo(SCENE_MAPA);
	map = true;
}
void program_entry_selector(){
	if (currentday == 0) {
		load_friday_page();
	} else if (currentday == 1) {
		load_saturday_page();
	} else if (currentday == 2) {
		load_sunday_page();
	}
	textOffsetX = 0;
	textOffset = 0;
	transpar = 255;
	Scene = 4;
	sceneManagerSwitchTo(SCENE_PROGRAM);
	map = false;
}
static bool running = true;

int main(int argc, char* argv[]) {
	cwavEnvMode_t mode = CWAV_ENV_DSP;
	cwavUseEnvironment(mode);
    romfsInit();
	cfguInit(); 
    gfxInitDefault();
	
    ndspInit();
	initBubbles();
	json_t *jsonfl;
    Result ret;
    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	PrintConsole topConsole;
    consoleInit(GFX_TOP, &topConsole);
	if(SOC_buffer == NULL) {
		printf("memalign: failed to allocate\n");
	}
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	printf("socInit: 0x%08X\n", (unsigned int)ret);
	}
	if (access("/3ds/ulubione.json", F_OK) == 0) {
		fav_count = load_favorites_from_json("/3ds/ulubione.json", favorites, MAX_ENTRIES);
	}
	

    char* body;

    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
 	C2D_TextBuf memBuf = C2D_TextBufNew(128);
	C2D_Text memtext[100];

    Scene = 0;
	bool sceneStarted = false; 
	bool rei = false;
	bool chiyoko = true;
	//przycskmachen = true;
    g_staticBuf = C2D_TextBufNew(256);

	
	

	populateCwavList();
	spritesInit();
    buttonsy[0] = (Button){20, 35, 134, 179, couponbutton, couponbutton_pressed, false, 2, 7, 7, 7, 7, 1.0f, mapa};
	buttonsy[1] = (Button){165, 35, 134, 179, progbutton, prog_pressed, false, 2, 7, 7, 7, 7, 1.0f, program_entry_selector};
    isScrolling = false;
	//C2D_TextParse(&g_staticText[16], g_staticBuf, "Ładowanie...");
	//C2D_TextParse(&g_staticText[17], g_staticBuf, "Pobieranie z serwerów...");
    //C2D_TextOptimize(&g_staticText[1]); 
	// CWAV* bgm = cwavList[1].cwav;
	// CWAV* loginbgm = cwavList[2].cwav;
	// CWAV* menu = day ? cwavList[3].cwav : cwavList[5].cwav;
	
	
	
	
	bool fuckeduplel = false;
	if (is_network_connected()) {
		const char* msg = "Pobieranie danych z serwera...";
		const char* msg2 = "Troche to trwa sorry :(";
		int screenWidth = topConsole.windowWidth;
		int screenHeight = topConsole.windowHeight;
		int x = (screenWidth - strlen(msg)) / 2;
		int x2 = (screenWidth - strlen(msg2)) / 2;
		int y = screenHeight / 2;
		printf("\x1b[%d;%dH%s", y, x, msg);  // ANSI escape to move cursor to (y, x)
		printf("\x1b[%d;%dH%s", y+1, x2, msg2);  // ANSI escape to move cursor to (y, x)
		//printf("Pobieranie z serwerów...");
		load_current_program();
	} 
	consoleClear();
	if (!czasfuckup) {
		if (access("/3ds/my_pyrkon.json", F_OK) == 0) {
			const char* msg = "Loading... (cierpliwosc plz)";
			int screenWidthcur = topConsole.windowWidth;
			int screenHeightcur = topConsole.windowHeight;
			int xcur = (screenWidthcur - strlen(msg)) / 2;
			int ycur = screenHeightcur / 2;
			printf("\x1b[%d;%dH%s", ycur, xcur, msg);  // ANSI escape to move cursor to (y, x)
			process_program(is_network_connected());
		} else {
			const char* msg = "Brak pobranego programu!";
			const char* msg2 = "Polacz sie do internetu i";
			const char* msg3 = "zrestartuj by pobrac.";
			int screenWidth = topConsole.windowWidth;
			int screenHeight = topConsole.windowHeight;
			int x = (screenWidth - strlen(msg)) / 2;
			int x2 = (screenWidth - strlen(msg2)) / 2;
			int x3 = (screenWidth - strlen(msg3)) / 2;
			int y = screenHeight / 2;
			printf("\x1b[%d;%dH%s", y, x, msg);  // ANSI escape to move cursor to (y, x)
			printf("\x1b[%d;%dH%s", y+1, x2, msg2);  // ANSI escape to move cursor to (y, x)
			printf("\x1b[%d;%dH%s", y+2, x3, msg3);  // ANSI escape to move cursor to (y, x)
			//printf("Pobieranie z serwerów...");
			sleep(5);
			fuckeduplel = true;
		}
	} else {
		const char* msg = "Zle ustawienia czasu!";
		const char* msg2 = "W Rosalina Menu zrob:";
		const char* msg3 = "Misc. Settings > Set Time via NTP";
		int screenWidth = topConsole.windowWidth;
		int screenHeight = topConsole.windowHeight;
		int x = (screenWidth - strlen(msg)) / 2;
		int x2 = (screenWidth - strlen(msg2)) / 2;
		int x3 = (screenWidth - strlen(msg3)) / 2;
		int y = screenHeight / 2;
		printf("\x1b[%d;%dH%s", y, x, msg);  // ANSI escape to move cursor to (y, x)
		printf("\x1b[%d;%dH%s", y+1, x2, msg2);  // ANSI escape to move cursor to (y, x)
		printf("\x1b[%d;%dH%s", y+2, x3, msg3);  // ANSI escape to move cursor to (y, x)
		//printf("Pobieranie z serwerów...");
		sleep(5);
		fuckeduplel = true;
	}
	consoleClear();

    gfxExit(); 
    gfxInitDefault();


    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
	
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	
	
    int lastTouchY = -1;
	int lastTouchX = -1;
	float scrollVelocity = 0.0f;
	float scrollVelocityX = 0.0f;
	float scrollVelocityY = 0.0f;
	float friction = 0.9f;
	int prevTouchX = -1;
	bool isDragging = false;
	int dragStartX = -1;
	int dragStartY = -1;
	const int SCROLL_THRESHOLD = 10;
	sceneManagerSwitchTo(SCENE_LOGO);
    while(aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START) {
            break;
        }
        if (fuckeduplel) {
            break;
        }
        touchPosition touch;
        hidTouchRead(&touch);
		updateBubbles();
		if (kHeld & KEY_TOUCH) {
			if (dragStartX == -1 || dragStartY == -1) {
				dragStartX = touch.px;
				dragStartY = touch.py;
				isDragging = false;
				scrollVelocity = 0.0f;
				scrollVelocityX = 0.0f; // NEW
			} else {
				int dx = abs(touch.px - dragStartX);
				int dy = abs(touch.py - dragStartY);
				if (dx > SCROLL_THRESHOLD || dy > SCROLL_THRESHOLD) {
					isDragging = true;
					for (int i = 0; i < 100; i++) {
						buttonsy[i].isPressed = false;
					}
				}
			}

			if (isDragging) {
				int currentTouchY = touch.py;
				int currentTouchX = touch.px;

				if (lastTouchY >= 0 && lastTouchX >= 0) {
					int deltaY = currentTouchY - lastTouchY;
					int deltaX = currentTouchX - lastTouchX;

					textOffset  -= (currentTouchY - lastTouchY) * scalemapa * 3;
					textOffsetX -= (currentTouchX - lastTouchX) * scalemapa * 3;

					if (textOffset < 0) textOffset = 0;
					if (textOffsetX < 0) textOffsetX = 0;

					// Optional: clamp to max_scrollX/Y if needed
					if (!map) {
						if (textOffset > max_scrollY) textOffset = max_scrollY;
					}
					// if (textOffsetX > max_scrollX) textOffsetX = max_scrollX;

					scrollVelocity = (float)deltaY;
					scrollVelocityX = (float)deltaX;
				}

				lastTouchY = currentTouchY;
				lastTouchX = currentTouchX;
				prevTouchX = touch.px;

			} else {
				for (int i = 0; i < 100; i++) {
					if (buttonsy[i].scene != Scene && buttonsy[i].scene2 != Scene) continue;
					if (isButtonPressed(&buttonsy[i], touch, Scene)) {
						buttonsy[i].isPressed = true;
						break;
					}
				}
			}
		} else {
			if (!isDragging) {
				for (int i = 0; i < 100; i++) {
					if (buttonsy[i].scene != Scene && buttonsy[i].scene2 != Scene) continue;
					if (buttonsy[i].isPressed) {
						buttonsy[i].isPressed = false;
						selectioncodelol = i;
						cwavPlay(sfx, 0, 1);
						executeButtonFunction(i);
						break;
					}
				}
			}
			lastTouchY = -7;
			dragStartX = -1;
			dragStartY = -1;
			isDragging = false;
			prevTouchX = -1;
		}



		
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		if (Scene == 0) {

		} else if (Scene == 1) {

        } else if (Scene == 2) {

        } else if (Scene == 3) {
			
        } else if (Scene == 4) {
	
        } else if (Scene == 8) {
				
        }
		sceneManagerUpdate(kDown, kHeld);
		sceneManagerRender();
		if (cpu_debug) {
			C2D_SceneBegin(top);
			float cpuStartX = 20.0f;
			float cpuStartY = 20.0f;
			float cpuMaxLength = 260.0f;
			float cpuUsage = C3D_GetProcessingTime() * 6.0f;
			if (cpuUsage > 100.0f) cpuUsage = 100.0f;
			float cpuLineLength = (cpuUsage / 100.0f) * cpuMaxLength;
			C2D_DrawLine(cpuStartX, cpuStartY, C2D_Color32(255, 255, 0, 255), cpuStartX + cpuLineLength, cpuStartY, C2D_Color32(255, 255, 0, 255), 5.0f, 1.0f);
			C2D_TextBufClear(memBuf);
			char memeText[64];
			snprintf(memeText, sizeof(memeText), "CPU: %.2f%%", cpuUsage);
			C2D_TextParse(&memtext[0], memBuf, memeText);
			C2D_TextOptimize(&memtext[0]);
			C2D_DrawText(&memtext[0], C2D_AlignLeft | C2D_WithColor, 20, 25, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			float drawUsage = C3D_GetDrawingTime() * 6.0f;
			if (drawUsage > 100.0f) drawUsage = 100.0f;
			float drawLineLength = (drawUsage / 100.0f) * cpuMaxLength;
			C2D_DrawLine(cpuStartX, cpuStartY + 30, C2D_Color32(255, 255, 0, 255), cpuStartX + drawLineLength, cpuStartY + 30, C2D_Color32(255, 255, 0, 255), 5.0f, 1.0f);
			C2D_TextBufClear(memBuf);
			char drawText[64];
			snprintf(memeText, sizeof(memeText), "GPU: %.2f%%", drawUsage);
			C2D_TextParse(&memtext[1], memBuf, memeText);
			C2D_TextOptimize(&memtext[1]);
			C2D_DrawText(&memtext[1], C2D_AlignLeft | C2D_WithColor, 20, 55, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
		}

		C3D_FrameEnd(0);
	} 
	char *json_text = global_response.data; 

	FILE *f = fopen("/3ds/my_pyrkon.json", "w");
	if (!f) {
		printf("Failed to open file\n");
	} else {
		fputs(json_text, f); 
		fclose(f);
	}
	C2D_TextBufDelete(g_staticBuf);
	//C2D_TextBufDelete(kupon_text_Buf);
    ndspExit();
	//cwavFree(bgm);
	cfguExit();
	freeCwavList();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
