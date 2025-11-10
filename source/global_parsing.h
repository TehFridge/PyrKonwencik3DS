#include <time.h>
#include <citro2d.h>
#ifndef GLOBAL_PARSING_H
#define GLOBAL_PARSING_H

#define MAX_ENTRIES 2000  // Set the maximum number of entries

extern int days;
extern int currentday;
extern bool has_sec_page;

extern int descpage;
extern int offset_friday;
extern int offset_saturday;
extern int offset_sunday;
extern int offset_ulub;
extern int offset_search;
extern int offset_caly;
extern char favorites[2000][256];
extern int sobota_offset;
extern int niedziela_offset;
extern int fav_count;
time_t createDate(int year, int month, int day);
extern C2D_TextBuf entry_name_Buf;
extern C2D_Text g_entry_nameText[810];
extern C2D_TextBuf description_Buf;
extern C2D_Text description_Text[810];
extern C2D_TextBuf prow_Buf;
extern C2D_Text prow_Text[810];
extern C2D_TextBuf loc_Buf;
extern C2D_Text loc_Text[810];
extern char data_table[MAX_ENTRIES][256];
extern char godzina_table[MAX_ENTRIES][256];
extern char prowadzacy_table[MAX_ENTRIES][256];
extern char tytul_table[MAX_ENTRIES][256];
extern char opis_table[MAX_ENTRIES][2048];  // Larger buffer for the description
extern char sala_table[MAX_ENTRIES][256];
extern char strefa_table[MAX_ENTRIES][256];
extern time_t data_time_table[MAX_ENTRIES];
extern int friday_ent;
extern int saturd_ent;
extern int sunday_ent;
extern float frlimit;
extern float stlimit;
extern float snlimit;

extern int max_scrollY;
extern bool can_further;

// Helper: Count UTF-8 characters (not bytes)
int utf8_strlen(const char* str);

// Safely appends to dynamically allocated buffer
bool safe_append(char** dest, size_t* capacity, const char* addition);
char* format_title_with_newlines_utf8(const char* text, int maxCharsPerLine);
extern int tileCount;
// Split text into three parts by finding spaces closest to 1/3 and 2/3 of the length
bool split_text_into_three(const char* text, char** out1, char** out2, char** out3);
extern bool map;

char *strcasestr(const char *haystack, const char *needle);
extern int search_results[801]; 
extern int search_result_count;

void search_entries(const char *query);
void read_entry();
void remove_br_tags(char *str);
void replace_ampersands(char *str);
void process_program(bool online);
void load_friday_page();
void load_saturday_page();
void load_sunday_page();
void save_favorites(char favorites[][256], int count, const char *filename);
int load_favorites_from_json(const char *filename, char favorites[][256], int max_count);

void load_ulubione_buttons(const char *filename);
void load_ulubione();
void load_search_page();

#endif