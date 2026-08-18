#include "tack/settings.h"

#include "tack/fileio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void settings_defaults(Settings *s)
{
    s->show_linenum = 1;
    s->word_wrap = 0;
    s->tabstop = 4;
    s->theme = THEME_NOTEPAD;
}

const char *settings_theme_name(KeyTheme theme)
{
    switch (theme) {
    case THEME_NANO:
        return "nano";
    case THEME_VI:
        return "vi";
    case THEME_EMACS:
        return "emacs";
    case THEME_NOTEPAD:
    default:
        return "notepad";
    }
}

int settings_theme_from_name(const char *name, KeyTheme *out)
{
    if (name == NULL || out == NULL) {
        return -1;
    }
    if (strcmp(name, "nano") == 0) {
        *out = THEME_NANO;
        return 0;
    }
    if (strcmp(name, "vi") == 0 || strcmp(name, "vim") == 0) {
        *out = THEME_VI;
        return 0;
    }
    if (strcmp(name, "emacs") == 0) {
        *out = THEME_EMACS;
        return 0;
    }
    if (strcmp(name, "notepad") == 0) {
        *out = THEME_NOTEPAD;
        return 0;
    }
    return -1;
}

static int parse_bool(const char *v)
{
    return strcmp(v, "1") == 0 || strcmp(v, "true") == 0 ||
           strcmp(v, "yes") == 0 || strcmp(v, "on") == 0;
}

int settings_parse(Settings *s, const char *text, size_t n)
{
    size_t i = 0;
    settings_defaults(s);
    if (text == NULL) {
        return 0;
    }
    while (i < n) {
        char line[256];
        size_t k = 0;
        char *eq;
        while (i < n && text[i] != '\n' && k + 1 < sizeof(line)) {
            if (text[i] != '\r') {
                line[k++] = text[i];
            }
            i++;
        }
        if (i < n && text[i] == '\n') {
            i++;
        }
        line[k] = '\0';
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }
        eq = strchr(line, '=');
        if (eq == NULL) {
            continue;
        }
        *eq = '\0';
        if (strcmp(line, "show_linenum") == 0) {
            s->show_linenum = parse_bool(eq + 1);
        } else if (strcmp(line, "word_wrap") == 0) {
            s->word_wrap = parse_bool(eq + 1);
        } else if (strcmp(line, "tabstop") == 0) {
            int t = atoi(eq + 1);
            if (t > 0 && t <= 16) {
                s->tabstop = t;
            }
        } else if (strcmp(line, "key_theme") == 0) {
            KeyTheme th;
            if (settings_theme_from_name(eq + 1, &th) == 0) {
                s->theme = th;
            }
        }
    }
    return 0;
}

char *settings_format(const Settings *s, size_t *out_len)
{
    char buf[256];
    char *copy;
    int n = snprintf(buf, sizeof(buf),
                     "show_linenum=%d\nword_wrap=%d\ntabstop=%d\nkey_theme=%s\n",
                     s->show_linenum ? 1 : 0, s->word_wrap ? 1 : 0, s->tabstop,
                     settings_theme_name(s->theme));
    if (n < 0) {
        return NULL;
    }
    copy = malloc((size_t)n + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, buf, (size_t)n + 1);
    if (out_len) {
        *out_len = (size_t)n;
    }
    return copy;
}

int settings_load(Settings *s, const char *path)
{
    char *data = NULL;
    size_t len = 0;
    int rc;
    settings_defaults(s);
    rc = file_read_all(path, &data, &len);
    if (rc == 1) {
        return 0;
    }
    if (rc != 0) {
        return -1;
    }
    settings_parse(s, data, len);
    free(data);
    return 0;
}

int settings_save(const Settings *s, const char *path)
{
    size_t n = 0;
    char *text = settings_format(s, &n);
    int rc;
    if (text == NULL) {
        return -1;
    }
    rc = file_write_all(path, text, n);
    free(text);
    return rc;
}
