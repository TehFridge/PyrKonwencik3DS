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

#define MAX_SPRITES   1
#define fmin(a, b) ((a) < (b) ? (a) : (b))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x300000

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

#define NUM_POINTS 15
#define BASE_HEIGHT (SCREEN_HEIGHT / 2)
#define WAVE_AMPLITUDE 10.0f
#define WAVE_FREQUENCY 0.025f
#define MAX_ENTRIES 803  // Set the maximum number of entries
char data_table[MAX_ENTRIES][256];
char godzina_table[MAX_ENTRIES][256];
char prowadzacy_table[MAX_ENTRIES][256];
char tytul_table[MAX_ENTRIES][256];
char opis_table[MAX_ENTRIES][2048];  // Larger buffer for the description
char sala_table[MAX_ENTRIES][256];
char strefa_table[MAX_ENTRIES][256];
C3D_RenderTarget* bottom;
typedef struct {
    float x, y;
    float speed;
    float radius;
    bool onTopScreen;
} Bubble;

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

int transpar = 255;
int transpar2 = 0;
size_t totalsajz = 0;
bool cpu_debug = false;

extern CWAVInfo cwavList[8];
extern int cwavCount;
char combinedText[128]; 
float speed = 20.0f; // High speed for fast reset
int selectioncodelol;
int selectionthemelol;
FILE *fptr;
static u32 *SOC_buffer = NULL;
int Scene;
C2D_TextBuf entry_name_Buf;
C2D_Text g_entry_nameText[810];
C2D_TextBuf description_Buf;
C2D_Text description_Text[810];
C2D_TextBuf prow_Buf;
C2D_Text prow_Text[810];
C2D_TextBuf loc_Buf;
C2D_Text loc_Text[810];
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
// Helper: Count UTF-8 characters (not bytes)
int utf8_strlen(const char* str) {
    int len = 0;
    while (*str) {
        if ((*str & 0xC0) != 0x80) len++;  // skip continuation bytes
        str++;
    }
    return len;
}

// Safely appends to dynamically allocated buffer
bool safe_append(char** dest, size_t* capacity, const char* addition) {
    size_t need = strlen(*dest) + strlen(addition) + 1;
    if (need > *capacity) {
        *capacity *= 2;
        char* newbuf = realloc(*dest, *capacity);
        if (!newbuf) return false;
        *dest = newbuf;
    }
    strcat(*dest, addition);
    return true;
}
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
char* format_title_with_newlines_utf8(const char* text, int maxCharsPerLine) {
    if (!text) return NULL;

    // Allocate buffer
    size_t capacity = strlen(text) * 2 + 128;
    char* result = malloc(capacity);
    if (!result) return NULL;
    result[0] = '\0';

    // Copy text and remove all existing newlines
    char* cleanText = strdup(text);
    for (char* p = cleanText; *p; ++p) {
        if (*p == '\n') *p = ' ';
    }

    char* token = strtok(cleanText, " ");
    int lineLen = 0;

    while (token) {
        int tokenChars = utf8_strlen(token);

        if (lineLen + tokenChars + (lineLen > 0 ? 1 : 0) > maxCharsPerLine) {
            if (!safe_append(&result, &capacity, "\n")) break;
            lineLen = 0;
        } else if (lineLen > 0) {
            if (!safe_append(&result, &capacity, " ")) break;
            lineLen++;
        }

        if (!safe_append(&result, &capacity, token)) break;
        lineLen += tokenChars;

        token = strtok(NULL, " ");
    }

    free(cleanText);
    return result;
}
extern int tileCount;
// Split text into three parts by finding spaces closest to 1/3 and 2/3 of the length
bool split_text_into_three(const char* text, char** out1, char** out2, char** out3) {
    size_t len = strlen(text);
    size_t one_third = len / 3;
    size_t two_third = 2 * len / 3;

    // Find closest spaces before the target indices
    size_t i = one_third;
    while (i > 0 && text[i] != ' ') i--;
    if (i == 0) return false;
    size_t split1 = i;

    i = two_third;
    while (i > split1 && text[i] != ' ') i--;
    if (i <= split1) return false;
    size_t split2 = i;

    // Create substrings
    *out1 = strndup(text, split1);
    *out2 = strndup(text + split1 + 1, split2 - split1 - 1);
    *out3 = strdup(text + split2 + 1);

    return true;
}
bool map = false;
int currentday = 0;
bool has_sec_page;
int textOffset = 0;
int textOffsetX = 0;
int descpage;
int offset_friday = 0;
int offset_saturday = 0;
int offset_sunday = 0;
int offset_ulub = 0;
int offset_search = 0;
int offset_caly;
char favorites[822][256];
int sobota_offset = 0;
int niedziela_offset = 0;
int fav_count = 0;
char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}
int search_results[801]; 
int search_result_count = 0;

void search_entries(const char *query) {
    search_result_count = 0;
    for (int i = 0; i < tileCount && search_result_count < 50; i++) {
        if (strcasestr(tytul_table[i], query) || strcasestr(opis_table[i], query)) {
            search_results[search_result_count++] = i;
        }
    }
}
void read_entry(){
	textOffset = 0;
	C2D_TextBufClear(description_Buf);
	C2D_TextBufClear(prow_Buf);
	if (currentday == 0) {
		offset_caly = (selectioncodelol - 3) + offset_friday;
	} else if (currentday == 1) {
		offset_caly = (selectioncodelol - 3) + sobota_offset + offset_saturday;
	} else if (currentday == 2) {
		offset_caly = (selectioncodelol - 3) + niedziela_offset + offset_sunday;
	} else if (currentday == 3) {
		const char *fav_title = favorites[selectioncodelol - 3 + offset_ulub];
		offset_caly = -1;
		for (int i = 0; i < tileCount; i++) {
			if (strcmp(tytul_table[i], fav_title) == 0) {
				offset_caly = i;
				break;
			}
		}
		if (offset_caly == -1) {
			printf("Favorite title not found: %s\n", fav_title);
			return;
		}
	} else if (currentday == 4) {
		int search_index = search_results[selectioncodelol - 3 + offset_search];
		offset_caly = search_index;
	}

	if (strlen(opis_table[offset_caly]) > 871) {
		has_sec_page = true;
		char *part1, *part2, *part3;
		split_text_into_three(opis_table[offset_caly], &part1, &part2, &part3);
	    C2D_TextParse(&description_Text[selectioncodelol - 3], description_Buf, part1);
	    C2D_TextOptimize(&description_Text[selectioncodelol - 3]);
	    C2D_TextParse(&description_Text[selectioncodelol - 2], description_Buf, part2);
	    C2D_TextOptimize(&description_Text[selectioncodelol - 2]);
	    C2D_TextParse(&description_Text[selectioncodelol - 1], description_Buf, part3);
	    C2D_TextOptimize(&description_Text[selectioncodelol - 1]);
		free(part1);
		free(part2);
		free(part3);
	} else {
		has_sec_page = false;
	    C2D_TextParse(&description_Text[selectioncodelol - 3], description_Buf, opis_table[offset_caly]);
	    C2D_TextOptimize(&description_Text[selectioncodelol - 3]);
	}
	char salaText[128];
	snprintf(salaText, sizeof(salaText), "Gdzie: %s", sala_table[offset_caly]);
	C2D_TextParse(&loc_Text[selectioncodelol - 3], loc_Buf, salaText);
	C2D_TextOptimize(&loc_Text[selectioncodelol - 3]);
	char prowText[128];
	const char* prowadzacy = prowadzacy_table[offset_caly];

	if (prowadzacy != NULL && prowadzacy[0] != '\0') {
		snprintf(prowText, sizeof(prowText), "Prowadzący:\n%s", format_title_with_newlines_utf8(prowadzacy, 22));
	} else {
		snprintf(prowText, sizeof(prowText), "Prowadzący:\nZgubił się szukając\nsleep roomu");
	}

	C2D_TextParse(&prow_Text[0], prow_Buf, prowText);
	C2D_TextOptimize(&prow_Text[0]);
	char prow2Text[128];
	snprintf(prow2Text, sizeof(prow2Text), "Kiedy: %s", godzina_table[offset_caly]);

	C2D_TextParse(&prow_Text[1], prow_Buf, prow2Text);
	C2D_TextOptimize(&prow_Text[1]);
	descpage = 0;
	transpar = 255;
	Scene = 8;
}
C2D_Image couponbutton_pressed;
C2D_Image couponbutton;
C2D_Image entry_pressed;
C2D_Image entrybutton;
extern Button buttonsy[810];
int friday_ent;
int saturd_ent;
int sunday_ent;
float frlimit;
float stlimit;
float snlimit;

int max_scrollY = 0;
bool can_further = false;

void load_friday_page() {
	removeButtonEntries(809);
	C2D_TextBufClear(entry_name_Buf);	
	max_scrollY = 0;
	int furthercounter = 0;
	can_further = false;
	for (int i = 0; i < 6; i++) {
		if (strcmp(data_table[i + offset_friday], "13.06 piątek") == 0) {
			buttonsy[i + 3] = (Button){99999, 99999, 309, 70, entrybutton, entry_pressed, false, 4, 78, 78, 78, 78, 1.0f, read_entry};
			const char* title = tytul_table[i + offset_friday];
			if (title != NULL && title[0] != '\0') {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, title);
			} else {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "Brak Tytułu :(");
			}
			C2D_TextOptimize(&g_entry_nameText[i]);
			max_scrollY += 70;
			furthercounter += 1;
		} else {
			C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "");
			C2D_TextOptimize(&g_entry_nameText[i]);
		}
	}
	
	if (furthercounter >= 6) {
		can_further = true;
	}
	max_scrollY -= 190;
}
void load_saturday_page() {
	removeButtonEntries(809);
	C2D_TextBufClear(entry_name_Buf);	
	max_scrollY = 0;
	int furthercounter = 0;
	can_further = false;
	for (int i = 0; i < 6; i++) {
		if (strcmp(data_table[i + sobota_offset + offset_saturday], "14.06 sobota") == 0) {
			buttonsy[i + 3] = (Button){99999, 99999, 309, 70, entrybutton, entry_pressed, false, 4, 78, 78, 78, 78, 1.0f, read_entry};
			const char* title = tytul_table[i + sobota_offset + offset_saturday];
			if (title != NULL && title[0] != '\0') {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, title);
			} else {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "Brak Tytułu :(");
			}
			C2D_TextOptimize(&g_entry_nameText[i]);
			max_scrollY += 70;
			furthercounter += 1;
		} else {
			C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "");
			C2D_TextOptimize(&g_entry_nameText[i]);
		}
	}
	if (furthercounter >= 6) {
		can_further = true;
	}
	max_scrollY -= 190;
}
void load_sunday_page() {
	removeButtonEntries(809);
	C2D_TextBufClear(entry_name_Buf);	
	max_scrollY = 0;
	int furthercounter = 0;
	can_further = false;
	for (int i = 0; i < 6; i++) {
		if (strcmp(data_table[i + niedziela_offset + offset_sunday], "15.06 niedziela") == 0) {
			buttonsy[i + 3] = (Button){99999, 99999, 309, 70, entrybutton, entry_pressed, false, 4, 78, 78, 78, 78, 1.0f, read_entry};
			const char* title = tytul_table[i + niedziela_offset + offset_sunday];
			if (title != NULL && title[0] != '\0') {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, title);
			} else {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "Brak Tytułu :(");
			}
			C2D_TextOptimize(&g_entry_nameText[i]);
			max_scrollY += 70;
			furthercounter += 1;
		} else {
			C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "");
			C2D_TextOptimize(&g_entry_nameText[i]);
		}
	}
	if (furthercounter >= 6) {
		can_further = true;
	}
	max_scrollY -= 190;
}
void save_favorites(char favorites[][256], int count, const char *filename) {
    json_t *array = json_array();
    for (int i = 0; i < count; i++) {
        json_array_append_new(array, json_string(favorites[i]));
    }

    json_dump_file(array, filename, JSON_INDENT(2));
    json_decref(array);
}

int load_favorites_from_json(const char *filename, char favorites[][256], int max_count) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        //printf("Error opening file for reading.\n");
        return 0;
    }

    json_error_t error;
    json_t *json_array = json_loadf(f, 0, &error);
    fclose(f);

    if (!json_array || !json_is_array(json_array)) {
        //printf("Error loading JSON data: %s\n", error.text);
        return 0;
    }

    size_t index;
    json_t *value;
    int count = 0;

    json_array_foreach(json_array, index, value) {
        if (count >= max_count) break;
        if (json_is_string(value)) {
            const char *title = json_string_value(value);
            strncpy(favorites[count], title, 127);
            favorites[count][127] = '\0';
            count++;
        }
    }

    json_decref(json_array);
    return count;
}

void load_ulubione_buttons(const char *filename) {
    json_error_t error;
    json_t *root = json_load_file(filename, 0, &error);

    if (!root || !json_is_array(root)) {
       // printf("Failed to load ulubione.json: %s\n", error.text);
        return;
    }

    removeButtonEntries(809);
    C2D_TextBufClear(entry_name_Buf);
    max_scrollY = 0;
    can_further = false;

    size_t index;
    json_t *value;
    int loaded = 0;
    int offset = offset_ulub;

    json_array_foreach(root, index, value) {
        if (index < offset) continue;
        if (loaded >= 6) break;
        if (!json_is_string(value)) continue;

        const char *fav_title = json_string_value(value);

        // Find the title in tytul_table
        int found_index = -1;
        for (int i = 0; i < tileCount; i++) {
            if (strcmp(tytul_table[i], fav_title) == 0) {
                found_index = i;
                break;
            }
        }

        if (found_index == -1) continue;

        buttonsy[loaded + 3] = (Button){
            99999, 99999, 309, 70, entrybutton, entry_pressed, false,
            4, 78, 78, 78, 78, 1.0f, read_entry
        };

        const char *title = tytul_table[found_index];
        C2D_TextParse(&g_entry_nameText[loaded], entry_name_Buf,
                      (title && title[0] != '\0') ? title : "Brak Tytułu :(");
        C2D_TextOptimize(&g_entry_nameText[loaded]);
        max_scrollY += 70;
        loaded++;
    }

    for (int i = loaded; i < 6; i++) {
        buttonsy[i + 3] = (Button){
            99999, 99999, 309, 70, entrybutton, entry_pressed, true,
            0, 0, 0, 0, 0, 1.0f, NULL
        };
        C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "");
        C2D_TextOptimize(&g_entry_nameText[i]);
    }

    if (loaded >= 6 && fav_count > 6) can_further = true;
    max_scrollY = (loaded > 0) ? (loaded * 70 - 190) : 0;

    json_decref(root);
}


void load_ulubione() {
	removeButtonEntries(809);
	C2D_TextBufClear(entry_name_Buf);	
	max_scrollY = 0;
	int furthercounter = 0;
	can_further = false;
	for (int i = 0; i < 6; i++) {
		if (strcmp(data_table[i + 562 + offset_sunday], "15.06 niedziela") == 0) {
			buttonsy[i + 3] = (Button){99999, 99999, 309, 70, entrybutton, entry_pressed, false, 4, 78, 78, 78, 78, 1.0f, read_entry};
			const char* title = tytul_table[i + 562 + offset_sunday];
			if (title != NULL && title[0] != '\0') {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, title);
			} else {
				C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "Brak Tytułu :(");
			}
			C2D_TextOptimize(&g_entry_nameText[i]);
			max_scrollY += 70;
			furthercounter += 1;
		}
	}
	if (furthercounter > 5) {
		can_further = true;
	}
	max_scrollY -= 190;
}

void load_search_page() {
    removeButtonEntries(809);
    C2D_TextBufClear(entry_name_Buf);
    max_scrollY = 0;
    can_further = false;

    int display_count = 0;
    for (int i = offset_search; i < search_result_count && display_count < 6; i++) {
        int entry_index = search_results[i];
        buttonsy[display_count + 3] = (Button){
            99999, 99999, 309, 70, entrybutton, entry_pressed, false,
            4, 78, 78, 78, 78, 1.0f, read_entry
        };

        const char* title = tytul_table[entry_index];
        C2D_TextParse(&g_entry_nameText[display_count], entry_name_Buf,
            (title && title[0] != '\0') ? title : "Brak Tytułu :(");
        C2D_TextOptimize(&g_entry_nameText[display_count]);
        max_scrollY += 70;
        display_count++;
    }
    for (int i = display_count; i < 6; i++) {
        C2D_TextParse(&g_entry_nameText[i], entry_name_Buf, "");
        C2D_TextOptimize(&g_entry_nameText[i]);
    }
	
    if (offset_search + 6 < search_result_count) can_further = true;
    max_scrollY -= 190;
}


void load_current_program() {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "Accept: application/json");

    refresh_data("https://core.pyrkon.pl/wp-json/pyrkon/v1/planner-items-search?type=standard&selectedlang=all&time=&offset=1&lang=pl&site_id=12&api_token=1c57cd904562dc3691b101d2c338f484&offset=0&limit=729", "", headers);

    json_error_t error;
    json_t *root = json_loads(global_response.data, 0, &error);

    if (!root) {
        printf("JSON parse error: %s\n", error.text);
        return;
    }

    json_dump_file(root, "/3ds/my_pyrkon.json", JSON_INDENT(2));
    json_decref(root);
}
// Function to remove all "<br />\r\n" tags from a string and replace with a newline
void remove_br_tags(char *str) {
    const char *needle = "<br />\r\n";
    char *src = str;
    char *dst = str;

    while (*src) {
        // If we find "<br />\r\n", replace it with a newline
        if (strncmp(src, needle, strlen(needle)) == 0) {
            *dst++ = '\n';  // Insert a newline instead of "<br />\r\n"
            src += strlen(needle);  // Skip over the "<br />\r\n"
        } else {
            *dst++ = *src++;  // Copy the character from source to destination
        }
    }

    *dst = '\0';  // Null-terminate the cleaned string
}
void replace_ampersands(char *str) {
    char *src = str;
    char *dst = str;

    while (*src) {
        if (strncmp(src, "&amp;", 5) == 0) {
            *dst++ = '&';
            src += 5;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

void process_program() {
    json_error_t error;
    json_t *root = json_load_file("/3ds/my_pyrkon.json", 0, &error);
    if (!root) {
        fprintf(stderr, "Error: %s\n", error.text);
        return;
    }

    json_t *data_array = json_object_get(root, "data");
    if (!json_is_array(data_array)) {
        fprintf(stderr, "Error: 'data' is not an array.\n");
        json_decref(root);
        return;
    }

    size_t index;
    json_t *entry;
    int entry_count = 0;

    int piatek_count = 0;
    int sobota_count = 0;
    int niedziela_count = 0;
	#define PROGRESS_BAR_WIDTH 40
    json_array_foreach(data_array, index, entry) {
        const char *data = json_string_value(json_object_get(entry, "date"));
        const char *godzina = json_string_value(json_object_get(entry, "range"));
		const char *tytul = json_string_value(json_object_get(entry, "title"));
		char *cleaned_tytul = strdup(tytul);  // Copy title for cleaning
		remove_br_tags(cleaned_tytul);
		replace_ampersands(cleaned_tytul);
		char *formatted = format_title_with_newlines_utf8(cleaned_tytul, 40);

        const char *opis = json_string_value(json_object_get(entry, "description"));
        char *cleaned_opis = strdup(opis);  // Create a copy of the description for cleaning

        // Remove <br />\r\n tags from the description and replace with a newline
        remove_br_tags(cleaned_opis);

        // Format the cleaned description
        char *formatted2 = format_title_with_newlines_utf8(cleaned_opis, 46);

        const char *sala = json_string_value(json_object_get(entry, "room"));

        json_t *blocks = json_object_get(entry, "blocks");
        const char *strefa = json_string_value(json_object_get(blocks, "list"));

        const char *prowadzacy = "";
        json_t *speakers = json_object_get(entry, "speakers");
        json_t *list = json_object_get(speakers, "list");

        char prow_buffer[256] = "";
        if (json_is_object(list)) {
            const char *key;
            json_t *val;
            size_t speaker_index = 0;
            json_object_foreach(list, key, val) {
                const char *name = json_string_value(json_object_get(val, "title"));
                if (name) {
                    if (speaker_index > 0) {
                        strncat(prow_buffer, ", ", sizeof(prow_buffer) - strlen(prow_buffer) - 1);
                    }
                    strncat(prow_buffer, name, sizeof(prow_buffer) - strlen(prow_buffer) - 1);
                    speaker_index++;
                }
            }
            prowadzacy = prow_buffer;
        }

        if (entry_count < MAX_ENTRIES) {
            snprintf(data_table[entry_count], sizeof(data_table[entry_count]), "%s", data);
            snprintf(godzina_table[entry_count], sizeof(godzina_table[entry_count]), "%s", godzina);
            snprintf(prowadzacy_table[entry_count], sizeof(prowadzacy_table[entry_count]), "%s", prowadzacy);
            snprintf(tytul_table[entry_count], sizeof(tytul_table[entry_count]), "%s", formatted);
            snprintf(opis_table[entry_count], sizeof(opis_table[entry_count]), "%s", formatted2);
            snprintf(sala_table[entry_count], sizeof(sala_table[entry_count]), "%s", sala);
            snprintf(strefa_table[entry_count], sizeof(strefa_table[entry_count]), "%s", strefa);

            //printf("Entry [%d]: '%s'\n", entry_count, data_table[entry_count]);
            entry_count++;
            tileCount++;
			if ((index + 1) % 10 == 0 || (index + 1) == json_array_size(data_array)) {
				float progress = (float)(index + 1) / json_array_size(data_array);
				int bar_chars = (int)(progress * PROGRESS_BAR_WIDTH);
				
				printf("\x1b[29;1H[");  // Move to bottom row (e.g., row 29), column 1
				for (int i = 0; i < PROGRESS_BAR_WIDTH; i++) {
					if (i < bar_chars) printf("=");
					else printf(" ");
				}
				printf("] %3.0f%%", progress * 100);
				fflush(stdout);  // Ensure it's printed immediately
			}
            if (strstr(data, "piątek")) {
                piatek_count++;
            } else if (strstr(data, "sobota")) {
                sobota_count++;
            } else if (strstr(data, "niedziela")) {
                niedziela_count++;
            }
        }

		free(formatted);
		free(cleaned_tytul);
        free(formatted2);
        free(cleaned_opis);  // Don't forget to free the cleaned description
    }

    // Offsets
    sobota_offset = piatek_count;
    niedziela_offset = piatek_count + sobota_count;
    json_decref(root);
}

typedef struct {
    float x, y;
} WavePoint;

WavePoint wave[NUM_POINTS];
float phaseOffsets[NUM_POINTS]; // for local randomness

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

C2D_TextBuf totpBuf = NULL;
C2D_Text g_totpText[5];  // Just declared, will be initialized when needed
// extern bool logplz;
// extern float text_w, text_h;
// extern float max_scroll;
const char *nejmenmachen;
int amountzappsy;
float currentY = -400.0f;
float currentY2 = -400.0f;
float timer = 0.0f;
float timer2 = 255.0f;
float timer3 = 0.0f;
float x = 0.0f;
float y = 0.0f;
float startY = -400.0f;    
float endY = 0.0f;    
float duration = 7.0f; 
float elapsed = 0.0f;   
float deltaTime = 0.1f; 
char themes[100][100];
struct memory {
  char *response;
  size_t size;
};
float easeHop(float t, float start, float end, float duration) {
    float s = 1.70158f * 1.5f; // overshoot factor
    t /= duration;
    t -= 1.0f;
    return (end - start) * (t * t * ((s + 1) * t + s) + 1.0f) + start;
}


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

void drawShadowedText(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };

    for (int i = 0; i < 4; i++) {
        C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor,
                     x + shadowOffsets[i][0], y + shadowOffsets[i][1],
                     depth, scaleX, scaleY, shadowColor);
    }

    C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y, depth, scaleX, scaleY, color);
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
typedef struct {
    C2D_Sprite spr;
    float dx, dy;
} Sprite;

C2D_TextBuf g_staticBuf;
C2D_Text g_staticText[100];
C2D_TextBuf themeBuf;
C2D_Text themeText[100];
C2D_Font font[1];
static C2D_SpriteSheet background_top, background_down, logo, buttons, settings, scan, points, coupons, qrframe, stat, zappbar, couponenbuttonen, goback, act_buttons, deact_buttons, more_b, logout_buttons, themename_border, too_less;
C2D_Image bgtop, bgdown, logo3ds, buttonsmol, buttonmed, buttonbeeg;

static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES / 2;
bool isScrolling;
bool isLogged;
float easeInQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (t * t);
}
float easeOutQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (1 - (1 - t) * (1 - t));
}
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
	map = false;
}
static bool running = true;
time_t createDate(int year, int month, int day) {
    struct tm date = {0};
    date.tm_year = year - 1900; // Years since 1900
    date.tm_mon = month - 1;    // 0 = Jan
    date.tm_mday = day;
    return mktime(&date);
}
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
	if (access("/3ds/ulubione.json", F_OK) != 0) {
		FILE *file = fopen("3ds/ulubione.json", "w");
		fprintf(file, "%s\n", "{}");
		fclose(file);
	}
	fav_count = load_favorites_from_json("/3ds/ulubione.json", favorites, 810);
	entry_name_Buf = C2D_TextBufNew(5096);
	description_Buf = C2D_TextBufNew(5096);
	loc_Buf = C2D_TextBufNew(5096);
	prow_Buf = C2D_TextBufNew(5096);
    char* body;
    time_t now = time(NULL); // Current date and time
    struct tm *today = localtime(&now);

    // Create a target date (e.g., December 31, 2025)
    time_t future = createDate(2025, 06, 13);

    double seconds = difftime(future, now);
    int days = (int)(seconds / (60 * 60 * 24));
	if (days < 0) {
		days = 0;
	}
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
 	C2D_TextBuf memBuf = C2D_TextBufNew(128);
	C2D_Text memtext[100];
	static SwkbdState swkbd;
	static char mybuf[256];
	static char mybuf2[256];
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;
	bool didit = false;
	bool swkbdTriggered = false;
    Scene = 0;
	bool sceneStarted = false; 
	bool rei = false;
	bool chiyoko = true;
	//przycskmachen = true;
    g_staticBuf = C2D_TextBufNew(256);

	C2D_SpriteSheet scrollbarsheet;
	C2D_Image scrollbar;
	C2D_SpriteSheet splash = C2D_SpriteSheetLoad("romfs:/gfx/splash.t3x");
	background_top = C2D_SpriteSheetLoad("romfs:/gfx/bg.t3x");
	logo = C2D_SpriteSheetLoad("romfs:/gfx/logo.t3x");
	couponenbuttonen = C2D_SpriteSheetLoad("romfs:/gfx/coupon_button_machen.t3x");
	C2D_SpriteSheet entry_sheet = C2D_SpriteSheetLoad("romfs:/gfx/entry.t3x");
	C2D_SpriteSheet mapas = C2D_SpriteSheetLoad("romfs:/gfx/mapa.t3x");
	C2D_SpriteSheet mapas1 = C2D_SpriteSheetLoad("romfs:/gfx/mapa1.t3x");
	C2D_SpriteSheet mapas2 = C2D_SpriteSheetLoad("romfs:/gfx/mapa2.t3x");
	C2D_SpriteSheet mapas3 = C2D_SpriteSheetLoad("romfs:/gfx/mapa3.t3x");
	C2D_SpriteSheet programb = C2D_SpriteSheetLoad("romfs:/gfx/program.t3x");
	bgtop = C2D_SpriteSheetGetImage(background_top, 0);
	C2D_Image logo3ds = C2D_SpriteSheetGetImage(logo, 0);
	C2D_Image splash1 = C2D_SpriteSheetGetImage(splash, 0);
	C2D_Image splash2 = C2D_SpriteSheetGetImage(splash, 1);	
	couponbutton = C2D_SpriteSheetGetImage(couponenbuttonen, 0); 
	couponbutton_pressed = C2D_SpriteSheetGetImage(couponenbuttonen, 1); 
	entrybutton = C2D_SpriteSheetGetImage(entry_sheet, 0); 
	entry_pressed = C2D_SpriteSheetGetImage(entry_sheet, 1); 
	C2D_Image progbutton = C2D_SpriteSheetGetImage(programb, 0); 
	C2D_Image prog_pressed = C2D_SpriteSheetGetImage(programb, 1); 
	C2D_Image mapa1 = C2D_SpriteSheetGetImage(mapas, 0); 
	C2D_Image mapa2 = C2D_SpriteSheetGetImage(mapas1, 0);  
	C2D_Image mapa3 = C2D_SpriteSheetGetImage(mapas2, 0);  
	C2D_Image mapa4 = C2D_SpriteSheetGetImage(mapas3, 0); 
	
    buttonsy[0] = (Button){20, 35, 134, 179, couponbutton, couponbutton_pressed, false, 2, 7, 7, 7, 7, 1.0f, mapa};
	buttonsy[1] = (Button){165, 35, 134, 179, progbutton, prog_pressed, false, 2, 7, 7, 7, 7, 1.0f, program_entry_selector};
	populateCwavList();
	float tajmer;
    isScrolling = false;
    C2D_TextParse(&g_staticText[0], g_staticBuf, "Wciśnij A.");
    C2D_TextOptimize(&g_staticText[0]); 
	char dayText[64];
	snprintf(dayText, sizeof(dayText), "%d", days);
    C2D_TextParse(&g_staticText[1], g_staticBuf, dayText);
	C2D_TextParse(&g_staticText[2], g_staticBuf, "Dni");
	C2D_TextParse(&g_staticText[3], g_staticBuf, "Pozostało:");
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
	//C2D_TextParse(&g_staticText[16], g_staticBuf, "Ładowanie...");
	//C2D_TextParse(&g_staticText[17], g_staticBuf, "Pobieranie z serwerów...");
    //C2D_TextOptimize(&g_staticText[1]); 
	// CWAV* bgm = cwavList[1].cwav;
	// CWAV* loginbgm = cwavList[2].cwav;
	// CWAV* menu = day ? cwavList[3].cwav : cwavList[5].cwav;
	CWAV* splashb = cwavList[0].cwav;
	CWAV* menu = cwavList[1].cwav;
	CWAV* sfx = cwavList[2].cwav;
	menu->volume = 0.6f;
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
	if (access("/3ds/my_pyrkon.json", F_OK) == 0) {
		const char* msg = "Loading... (cierpliwosc plz)";
		int screenWidthcur = topConsole.windowWidth;
		int screenHeightcur = topConsole.windowHeight;
		int xcur = (screenWidthcur - strlen(msg)) / 2;
		int ycur = screenHeightcur / 2;
		printf("\x1b[%d;%dH%s", ycur, xcur, msg);  // ANSI escape to move cursor to (y, x)
		process_program();
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
	consoleClear();

    gfxExit(); 
    gfxInitDefault();


    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	float scalemapa = 0.5f;
	float time = 0.0f;
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
	bool splashPlayed = false;
	u64 splashStartTime = 0;
	bool splashDone = false;
	float splashTimer = 0.0f;
	float splashY = 240.0f;       // Start below screen
	float splashHopTime = 0.0f;
	const float splashHopDuration = 0.8f; // Total hop time
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
		if (kDown & KEY_A) {
			if (Scene == 1 && timer > 7.0f) {
				cwavPlay(sfx, 0, 1);
				timer = 0.0f;
				transpar = 255;
				Scene = 2;
			}
		}
		if (kDown & KEY_B) {
			if ((Scene == 3 || Scene == 4) && timer > 7.0f) {
				cwavPlay(sfx, 0, 1);
				timer = 0.0f;
				transpar = 255;
				Scene = 2;
			}
			if (Scene == 8 && timer > 7.0f && currentday != 3) {
				cwavPlay(sfx, 0, 1);
				timer = 0.0f;
				transpar = 255;
				Scene = 4;
			} else if (Scene == 8 && timer > 7.0f && currentday == 3) {
				cwavPlay(sfx, 0, 1);
				timer = 0.0f;
				transpar = 255;
				load_ulubione_buttons("/3ds/ulubione.json");
				Scene = 4;	
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
		if (kDown & KEY_L) {
			if (Scene == 3 && timer > 7.0f && scalemapa != 0.0f) {
				scalemapa -= 0.1f;
			}
			if (Scene == 8 && has_sec_page && descpage != 0) {
				cwavPlay(sfx, 0, 1);
				descpage -= 1;
			}
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
					if (fav_count < 810) {
						strncpy(favorites[fav_count], current_title, sizeof(favorites[fav_count]) - 1);
						favorites[fav_count][sizeof(favorites[fav_count]) - 1] = '\0';
						fav_count++;
					}
				}

				save_favorites(favorites, fav_count, "/3ds/ulubione.json");
			}
		}

		if (kDown & KEY_R) {
			if (Scene == 3 && timer > 7.0f) {
				scalemapa += 0.1f;
			}
			if (Scene == 8 && has_sec_page && descpage != 2) {
				cwavPlay(sfx, 0, 1);
				descpage += 1;
			}
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
		
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		if (Scene == 0) {
			if (!splashDone) {
				float dt = 1.0f / 60.0f;
				splashTimer += dt;

				if (!splashPlayed) {
					cwavPlay(splashb, 0, 1);
					splashPlayed = true;
				}

				// Only increment once here
				if (splashHopTime < splashHopDuration) {
					splashHopTime += dt;
					splashY = easeHop(splashHopTime, 100.0f, 0.0f, splashHopDuration);
				} else {
					splashY = 0.0f;
				}

				if (splashTimer < 4.6f) {
					C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
					C2D_SceneBegin(top);
					C2D_DrawImageAt(splash1, 0.0f, splashY, 0.0f, NULL, 1.0f, 1.0f);
					if (splashTimer > 1.2f) {
						C2D_DrawImageAt(splash2, 0.0f, splashY, 0.0f, NULL, 1.0f, 1.0f);
					}
					if (splashTimer > 1.2f && transpar != 0) {
						transpar -= 5;
						C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, transpar));
					}
					if (splashTimer > 3.5f) {
						if (transpar2 != 255){
							transpar2 += 5;
						}
						C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, transpar2));
					}

					C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
					C2D_SceneBegin(bottom);
				} else {
					splashDone = true;
				}
			} else {
				// if (bgm->numChannels == 2) {
					// cwavPlay(bgm, 0, 1);
				// } else {
					// cwavPlay(bgm, 0, -1);
					
				// }
				Scene = 1;
				splashPlayed = false;
			}
		} else if (Scene == 1) {
			//time += 0.1f;
			//updateWave(time);
			if (!splashPlayed) {
				cwavPlay(menu, 0, 1);
				splashPlayed = true;
			}
            timer += 0.2f ;
            if (timer > 7.0f) {
                isScrolling = true;
            }
            
            C2D_TargetClear(top, C2D_Color32f(1.0f, 1.0f, 1.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }
			
            if (isScrolling) {
                if (elapsed < duration) {
                    currentY = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }
                
            //C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
            //drawWaveFill();
            C2D_TargetClear(bottom, C2D_Color32f(1.0f, 1.0f, 1.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            //C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			//C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));	
			drawShadowedText(&g_staticText[0], 160.0f, -currentY + 100.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
        } else if (Scene == 2) {
			//updateBubbles();
			time += 0.1f;
			updateWave(time);
            timer += 0.2f ;
            if (timer > 20.0f) {
                isScrolling = true;
            }
            if (transpar != 0) {
				transpar -= 15;
			}
            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
                    currentY = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            //C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
            drawWaveFill();
			drawBubblesTop();
			float delta = 1.0f / 60.0f; // Approx. 60 FPS
			tajmer += delta;

			float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
			drawShadowedText(&g_staticText[1], 205.0f, 80.0f + yOffset, 0.5f, 2.3f, 2.3f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[2], 202.5f, 140.0f + yOffset, 0.5f, 0.7f, 0.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[3], 205.0f, 40.0f + yOffset, 0.5f, 1.4f, 1.4f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
			drawBubblesBottom();
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));				
        } else if (Scene == 3) {
			//time += 0.1f;
			//updateWave(time);
            timer += 0.2f ;
            if (timer > 20.0f) {
                isScrolling = true;
            }
            if (transpar != 0) {
				transpar -= 15;
			}
            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
                    currentY = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
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
            
            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			float delta = 1.0f / 60.0f; // Approx. 60 FPS
			tajmer += delta;

			float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
			drawShadowedText(&g_staticText[9], 160.0f, 40.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[10], 160.0f, 60.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[11], 160.0f, 80.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));				
        } else if (Scene == 4) {
			//updateBubbles();
			time += 0.1f;
			updateWave(time);
            timer += 0.2f ;
            if (timer > 20.0f) {
                isScrolling = true;
            }
            if (transpar != 0) {
				transpar -= 15;
			}
            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
                    currentY = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            //C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
            drawWaveFill();
			drawBubblesTop();
			float delta = 1.0f / 60.0f; // Approx. 60 FPS
			tajmer += delta;

			float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
			drawShadowedText(&g_staticText[15], 200.0f, 150.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[9], 200.0f, 170.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[8], 200.0f, 190.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&g_staticText[7], 200.0f, 210.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			if (currentday == 0) {
				drawShadowedText(&g_staticText[4], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
			} else if (currentday == 1) {
				drawShadowedText(&g_staticText[5], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
			} else if (currentday == 2) {
				drawShadowedText(&g_staticText[6], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
			} else if (currentday == 3) {
				drawShadowedText(&g_staticText[12], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
			} else if (currentday == 4) {
				drawShadowedText(&g_staticText[14], 200.0f, 80.0f + yOffset, 0.5f, 1.7f, 1.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
			}
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
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
								 C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				//printf("buttonsy[%d] = (%f, %f)\n", i + 3, buttonsy[i + 3].x, buttonsy[i + 3].y);
			}
			C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));		
			
        } else if (Scene == 8) {
			//updateBubbles();
			time += 0.1f;
			updateWave(time);
            timer += 0.2f ;
            if (timer > 20.0f) {
                isScrolling = true;
            }
            if (transpar != 0) {
				transpar -= 15;
			}
            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
                    currentY = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            //C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
            //drawWaveFill();
			//drawBubblesTop();
			float delta = 1.0f / 60.0f; // Approx. 60 FPS
			tajmer += delta;

			float yOffset = sinf(tajmer * 2.0f) * 5.0f; // 2 Hz sine wave, 5px amplitude
			drawShadowedText(&g_staticText[9], 50.0f, 10.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			if (has_sec_page) {
				drawShadowedText(&g_staticText[7], 200.0f, 10.0f + yOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			}
			drawShadowedText(&g_staticText[13], 340.0f, 13.0f + yOffset, 0.5f, 0.45f, 0.45f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));

			drawShadowedText(&prow_Text[0], 200.0f, 80.0f + yOffset, 0.5f, 1.2f, 1.2f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			drawShadowedText(&prow_Text[1], 200.0f, 30.0f + yOffset, 0.5f, 0.9f, 0.9f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			// drawShadowedText(&g_staticText[2], 202.5f, 140.0f + yOffset, 0.5f, 0.7f, 0.7f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			// drawShadowedText(&g_staticText[3], 205.0f, 40.0f + yOffset, 0.5f, 1.4f, 1.4f, C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(76, 25, 102, 220));
			//drawBubblesBottom();
			for (int i = 0; i < 810; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			drawShadowedText(&description_Text[selectioncodelol - 3 + descpage], 160.0f, 70.0f - textOffset, 0.5f, 0.5f, 0.5f,
							C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			float originalWidth;
			C2D_TextGetDimensions(&loc_Text[selectioncodelol - 3], 1.0f, 1.0f, &originalWidth, NULL);  // Only need width

			float maxWidth = 300.0f;
			float scale = maxWidth / originalWidth;
			drawShadowedText(&loc_Text[selectioncodelol - 3], 160.0f, 10.0f - textOffset, 0.5f, scale, scale,
							C2D_Color32(76, 25, 102, 220), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			C2D_DrawRectSolid(0.0f,0.0f,1.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(255, 255, 255, transpar));	
						
        }
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
