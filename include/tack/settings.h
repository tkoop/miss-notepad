#ifndef TACK_SETTINGS_H
#define TACK_SETTINGS_H

#include <stddef.h>

typedef enum {
    THEME_NOTEPAD = 0,
    THEME_NANO,
    THEME_VI,
    THEME_EMACS
} KeyTheme;

typedef struct {
    int show_linenum;
    int word_wrap;
    int tabstop;
    KeyTheme theme;
} Settings;

void settings_defaults(Settings *s);
int settings_parse(Settings *s, const char *text, size_t n);
char *settings_format(const Settings *s, size_t *out_len);
int settings_load(Settings *s, const char *path);
int settings_save(const Settings *s, const char *path);
const char *settings_theme_name(KeyTheme theme);
int settings_theme_from_name(const char *name, KeyTheme *out);

#endif
