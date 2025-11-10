#include "global_parsing.h"
#include "global_draws.h"
#include "scene_manager.h"
#include "buttons.h"
#include "sprites.h"
#include "request.h"
#include <citro2d.h>
#include <jansson.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>
#include <unistd.h>
int days;

int currentday = 0;
bool has_sec_page;
extern Button buttonsy[2000];
int descpage;
int offset_friday = 0;
int offset_saturday = 0;
int offset_sunday = 0;
int offset_ulub = 0;
int offset_search = 0;
int offset_caly;
char favorites[2000][256];
int sobota_offset = 0;
int niedziela_offset = 0;
int fav_count = 0;
time_t createDate(int year, int month, int day) {
    struct tm date = {0};
    date.tm_year = year - 1900; // Years since 1900
    date.tm_mon = month - 1;    // 0 = Jan
    date.tm_mday = day;
    return mktime(&date);
}
C2D_TextBuf entry_name_Buf;
C2D_Text g_entry_nameText[810];
C2D_TextBuf description_Buf;
C2D_Text description_Text[810];
C2D_TextBuf prow_Buf;
C2D_Text prow_Text[810];
C2D_TextBuf loc_Buf;
C2D_Text loc_Text[810];
char data_table[MAX_ENTRIES][256];
char godzina_table[MAX_ENTRIES][256];
char prowadzacy_table[MAX_ENTRIES][256];
char tytul_table[MAX_ENTRIES][256];
char opis_table[MAX_ENTRIES][2048];  // Larger buffer for the description
char sala_table[MAX_ENTRIES][256];
char strefa_table[MAX_ENTRIES][256];
time_t data_time_table[MAX_ENTRIES];
int friday_ent;
int saturd_ent;
int sunday_ent;
float frlimit;
float stlimit;
float snlimit;

int max_scrollY = 0;
bool can_further = false;

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
	if (offset_caly >= 0) {
		// data_table[offset_caly] holds something like "13.06 piątek"
		C2D_TextParse(&prow_Text[2], prow_Buf, data_table[offset_caly]);
		C2D_TextOptimize(&prow_Text[2]);
	}
	descpage = 0;
	transpar = 255;
	Scene = 8;
    sceneManagerSwitchTo(SCENE_ENTRY);
}
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

void process_program(bool online) {
    json_error_t error;
	json_t *root;
	if (!online) {
    	root = json_load_file("/3ds/my_pyrkon.json", 0, &error);
	} else {
		root = json_loads(global_response.data, 0, &error);
	}
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
void load_friday_page() {
	removeButtonEntries(2000);
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
	removeButtonEntries(2000);
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
	removeButtonEntries(2000);
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
		currentday = 2;
        return;
    }
    removeButtonEntries(2000);
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
	if (loaded == 0) {
		currentday = 2;
		load_sunday_page();
	}
    json_decref(root);
}


void load_ulubione() {
	removeButtonEntries(2000);
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
    removeButtonEntries(2000);
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
