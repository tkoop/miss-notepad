#include "missnotepad/menu.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void add_item(Menu *m, const char *label, const char *accel, char hot,
                     Action act, int checkable)
{
    MenuItem *it;
    if (m->count >= MISSNOTEPAD_ITEM_MAX) {
        return;
    }
    it = &m->items[m->count++];
    it->label = label;
    it->accel = accel;
    it->hot = hot;
    it->action = act;
    it->separator = 0;
    it->checkable = checkable;
}

static void add_sep(Menu *m)
{
    MenuItem *it;
    if (m->count >= MISSNOTEPAD_ITEM_MAX) {
        return;
    }
    it = &m->items[m->count++];
    it->label = NULL;
    it->accel = NULL;
    it->hot = 0;
    it->action = ACT_NONE;
    it->separator = 1;
    it->checkable = 0;
}

void menubar_init_notepad(MenuBar *m)
{
    Menu *u;
    memset(m, 0, sizeof(*m));
    m->open = -1;
    m->highlight = 0;
    m->active = 0;

    u = &m->menus[m->count++];
    u->label = "File";
    u->hot = 'F';
    add_item(u, "New", "Ctrl+N", 'N', ACT_NEW, 0);
    add_item(u, "Open...", "Ctrl+O", 'O', ACT_OPEN, 0);
    add_item(u, "Save", "Ctrl+S", 'S', ACT_SAVE, 0);
    add_item(u, "Save As...", NULL, 'A', ACT_SAVE_AS, 0);
    add_sep(u);
    add_item(u, "Exit", "Ctrl+Q", 'X', ACT_EXIT, 0);

    u = &m->menus[m->count++];
    u->label = "Edit";
    u->hot = 'E';
    add_item(u, "Undo", "Ctrl+Z", 'U', ACT_UNDO, 0);
    add_item(u, "Redo", "Ctrl+Y", 'R', ACT_REDO, 0);
    add_sep(u);
    add_item(u, "Cut", "Ctrl+X", 'T', ACT_CUT, 0);
    add_item(u, "Copy", "Ctrl+C", 'C', ACT_COPY, 0);
    add_item(u, "Paste", "Ctrl+V", 'P', ACT_PASTE, 0);
    add_item(u, "Delete", "Del", 'D', ACT_DELETE, 0);
    add_sep(u);
    add_item(u, "Select All", "Ctrl+A", 'L', ACT_SELECT_ALL, 0);
    add_item(u, "Find...", "Ctrl+F", 'F', ACT_FIND, 0);
    add_item(u, "Find Next", "F3", 'N', ACT_FIND_NEXT, 0);
    add_item(u, "Replace...", "Ctrl+H", 'E', ACT_REPLACE, 0);

    u = &m->menus[m->count++];
    u->label = "Format";
    u->hot = 'O';
    add_item(u, "Word Wrap", NULL, 'W', ACT_WORD_WRAP, 1);

    u = &m->menus[m->count++];
    u->label = "View";
    u->hot = 'V';
    add_item(u, "Line Numbers", NULL, 'L', ACT_LINE_NUMBERS, 1);
    add_sep(u);
    add_item(u, "Notepad keys", NULL, 'N', ACT_THEME_NOTEPAD, 1);
    add_item(u, "nano keys", NULL, 'A', ACT_THEME_NANO, 1);
    add_item(u, "vi keys", NULL, 'I', ACT_THEME_VI, 1);
    add_item(u, "Emacs keys", NULL, 'E', ACT_THEME_EMACS, 1);

    u = &m->menus[m->count++];
    u->label = "Help";
    u->hot = 'H';
    add_item(u, "Keyboard shortcuts", NULL, 'K', ACT_HELP_KEYS, 0);
    add_item(u, "About MissNotepad", NULL, 'A', ACT_ABOUT, 0);

    menubar_layout(m);
}

/* Menu accelerator hints per key theme. Entries only list shortcuts that
 * are actually active in that theme; NULL means no binding, so no hint is
 * shown. Themes in table order: Notepad, nano, vi, Emacs. */
static const struct {
    Action act;
    const char *notepad;
    const char *nano;
    const char *vi;
    const char *emacs;
} accel_table[] = {
    {ACT_NEW, "Ctrl+N", NULL, NULL, NULL},
    {ACT_OPEN, "Ctrl+O", NULL, NULL, "C-x C-f"},
    {ACT_SAVE, "Ctrl+S", "^O", ":w", "C-x C-s"},
    {ACT_SAVE_AS, NULL, NULL, NULL, "C-x C-w"},
    {ACT_EXIT, "Ctrl+Q", "^X", ":q", "C-x C-c"},
    {ACT_UNDO, "Ctrl+Z", NULL, "u", NULL},
    {ACT_REDO, "Ctrl+Y", NULL, NULL, NULL},
    {ACT_CUT, "Ctrl+X", "^K", NULL, "C-w"},
    {ACT_COPY, "Ctrl+C", NULL, NULL, NULL},
    {ACT_PASTE, "Ctrl+V", "^U", "p", "C-y"},
    {ACT_DELETE, "Del", "Del", "x", "Del"},
    {ACT_SELECT_ALL, "Ctrl+A", NULL, NULL, NULL},
    {ACT_FIND, "Ctrl+F", "^W", "/", "C-s"},
    {ACT_FIND_NEXT, "F3", NULL, "n", "C-r"},
    {ACT_REPLACE, "Ctrl+H", "^\\", NULL, NULL},
    {ACT_HELP_KEYS, NULL, "^G", NULL, NULL},
};

void menubar_set_theme(MenuBar *m, KeyTheme theme)
{
    size_t i;
    int k, j;

    for (k = 0; k < m->count; k++) {
        for (j = 0; j < m->menus[k].count; j++) {
            MenuItem *it = &m->menus[k].items[j];
            for (i = 0; i < sizeof(accel_table) / sizeof(accel_table[0]);
                 i++) {
                if (accel_table[i].act != it->action) {
                    continue;
                }
                switch (theme) {
                case THEME_NANO:
                    it->accel = accel_table[i].nano;
                    break;
                case THEME_VI:
                    it->accel = accel_table[i].vi;
                    break;
                case THEME_EMACS:
                    it->accel = accel_table[i].emacs;
                    break;
                case THEME_NOTEPAD:
                default:
                    it->accel = accel_table[i].notepad;
                    break;
                }
                break;
            }
        }
    }
    menubar_layout(m);
}

void menubar_layout(MenuBar *m)
{
    int i;
    int x = 1;
    for (i = 0; i < m->count; i++) {
        m->menus[i].x = x;
        m->menus[i].w = (int)strlen(m->menus[i].label) + 2;
        x += m->menus[i].w + 1;
    }
}

void menubar_close(MenuBar *m)
{
    m->open = -1;
    m->active = 0;
}

int menubar_open(MenuBar *m, int index)
{
    if (index < 0 || index >= m->count) {
        return 0;
    }
    m->open = index;
    m->highlight = 0;
    while (m->highlight < m->menus[index].count &&
           m->menus[index].items[m->highlight].separator) {
        m->highlight++;
    }
    m->active = 1;
    return 1;
}

int menubar_open_hot(MenuBar *m, char hot)
{
    int i;
    hot = (char)toupper((unsigned char)hot);
    for (i = 0; i < m->count; i++) {
        if (toupper((unsigned char)m->menus[i].hot) == hot) {
            return menubar_open(m, i);
        }
    }
    return 0;
}

int menubar_hit_bar(const MenuBar *m, int y, int x)
{
    int i;
    if (y != 0) {
        return -1;
    }
    for (i = 0; i < m->count; i++) {
        if (x >= m->menus[i].x && x < m->menus[i].x + m->menus[i].w) {
            return i;
        }
    }
    return -1;
}

static int dropdown_width(const Menu *u)
{
    int i;
    int w = 12;
    for (i = 0; i < u->count; i++) {
        int n;
        if (u->items[i].separator) {
            continue;
        }
        n = (int)strlen(u->items[i].label) + 4;
        if (u->items[i].accel) {
            n += (int)strlen(u->items[i].accel) + 2;
        }
        if (n > w) {
            w = n;
        }
    }
    return w;
}

int menubar_hit_item(const MenuBar *m, int drop_y, int y, int x)
{
    const Menu *u;
    int w;
    int row;
    if (m->open < 0 || m->open >= m->count) {
        return -1;
    }
    u = &m->menus[m->open];
    w = dropdown_width(u);
    if (x < u->x || x >= u->x + w) {
        return -1;
    }
    row = y - drop_y - 1;
    if (row < 0 || row >= u->count) {
        return -1;
    }
    if (u->items[row].separator) {
        return -1;
    }
    return row;
}

static void move_highlight(MenuBar *m, int dir)
{
    Menu *u;
    int guard = 0;
    if (m->open < 0) {
        return;
    }
    u = &m->menus[m->open];
    do {
        m->highlight += dir;
        if (m->highlight < 0) {
            m->highlight = u->count - 1;
        }
        if (m->highlight >= u->count) {
            m->highlight = 0;
        }
        guard++;
    } while (u->items[m->highlight].separator && guard < u->count + 2);
}

Action menubar_handle_event(MenuBar *m, const Event *ev)
{
    Menu *u;
    if (ev == NULL || ev->kind != EV_KEY) {
        return ACT_NONE;
    }
    if (!m->active) {
        if (ev->key == KEY_F10) {
            menubar_open(m, 0);
            return ACT_NONE;
        }
        if ((ev->mods & MOD_ALT) && ev->key == KEY_CHAR && ev->ch < 128) {
            menubar_open_hot(m, (char)ev->ch);
            return ACT_NONE;
        }
        return ACT_NONE;
    }
    if (ev->key == KEY_ESC) {
        menubar_close(m);
        return ACT_NONE;
    }
    if (ev->key == KEY_LEFT) {
        menubar_open(m, m->open > 0 ? m->open - 1 : m->count - 1);
        return ACT_NONE;
    }
    if (ev->key == KEY_RIGHT) {
        menubar_open(m, m->open + 1 < m->count ? m->open + 1 : 0);
        return ACT_NONE;
    }
    if (ev->key == KEY_DOWN) {
        move_highlight(m, 1);
        return ACT_NONE;
    }
    if (ev->key == KEY_UP) {
        move_highlight(m, -1);
        return ACT_NONE;
    }
    if (ev->key == KEY_ENTER && m->open >= 0) {
        Action act = m->menus[m->open].items[m->highlight].action;
        menubar_close(m);
        return act;
    }
    if (ev->key == KEY_CHAR && ev->ch < 128 && m->open >= 0) {
        int i;
        char hot = (char)toupper((unsigned char)ev->ch);
        u = &m->menus[m->open];
        for (i = 0; i < u->count; i++) {
            if (!u->items[i].separator &&
                toupper((unsigned char)u->items[i].hot) == hot) {
                Action act = u->items[i].action;
                menubar_close(m);
                return act;
            }
        }
    }
    return ACT_NONE;
}

void menubar_sync_checks(MenuBar *m, int line_numbers, int word_wrap,
                         KeyTheme theme)
{
    int i, j;
    menubar_set_theme(m, theme);
    for (i = 0; i < m->count; i++) {
        for (j = 0; j < m->menus[i].count; j++) {
            MenuItem *it = &m->menus[i].items[j];
            if (!it->checkable) {
                continue;
            }
            if (it->action == ACT_LINE_NUMBERS) {
                it->checkable = line_numbers ? 2 : 1;
            } else if (it->action == ACT_WORD_WRAP) {
                it->checkable = word_wrap ? 2 : 1;
            } else if (it->action == ACT_THEME_NOTEPAD) {
                it->checkable = theme == THEME_NOTEPAD ? 2 : 1;
            } else if (it->action == ACT_THEME_NANO) {
                it->checkable = theme == THEME_NANO ? 2 : 1;
            } else if (it->action == ACT_THEME_VI) {
                it->checkable = theme == THEME_VI ? 2 : 1;
            } else if (it->action == ACT_THEME_EMACS) {
                it->checkable = theme == THEME_EMACS ? 2 : 1;
            }
        }
    }
}

void menubar_render(const MenuBar *m, Screen *s, int y, int cols,
                    const char *right)
{
    int i;
    screen_fill(s, y, 0, cols, 1, (uint32_t)' ', STYLE_MENU);
    for (i = 0; i < m->count; i++) {
        unsigned char st =
            (m->active && m->open == i) ? STYLE_MENU_SEL : STYLE_MENU;
        screen_puts(s, y, m->menus[i].x, m->menus[i].label, st);
    }
    if (right != NULL) {
        int n = (int)strlen(right);
        int x = cols - n - 1;
        if (x < 2) {
            x = 2;
        }
        screen_puts(s, y, x, right, STYLE_MENU);
    }
    if (m->active && m->open >= 0 && m->open < m->count) {
        const Menu *u = &m->menus[m->open];
        int w = dropdown_width(u);
        int h = u->count + 2;
        int dx = u->x;
        int dy = y + 1;
        int r;
        if (dx + w >= cols) {
            dx = cols - w - 1;
            if (dx < 0) {
                dx = 0;
            }
        }
        screen_fill(s, dy, dx, w, h, (uint32_t)' ', STYLE_DIALOG);
        /* box */
        for (i = 0; i < w; i++) {
            screen_put(s, dy, dx + i, (uint32_t)'-', STYLE_DIALOG);
            screen_put(s, dy + h - 1, dx + i, (uint32_t)'-', STYLE_DIALOG);
        }
        for (r = 0; r < h; r++) {
            screen_put(s, dy + r, dx, (uint32_t)'|', STYLE_DIALOG);
            screen_put(s, dy + r, dx + w - 1, (uint32_t)'|', STYLE_DIALOG);
        }
        screen_put(s, dy, dx, (uint32_t)'+', STYLE_DIALOG);
        screen_put(s, dy, dx + w - 1, (uint32_t)'+', STYLE_DIALOG);
        screen_put(s, dy + h - 1, dx, (uint32_t)'+', STYLE_DIALOG);
        screen_put(s, dy + h - 1, dx + w - 1, (uint32_t)'+', STYLE_DIALOG);

        for (r = 0; r < u->count; r++) {
            const MenuItem *it = &u->items[r];
            int iy = dy + 1 + r;
            unsigned char st =
                (r == m->highlight) ? STYLE_MENU_SEL : STYLE_DIALOG;
            if (it->separator) {
                int c;
                for (c = 1; c < w - 1; c++) {
                    screen_put(s, iy, dx + c, (uint32_t)'-', STYLE_DIALOG);
                }
                continue;
            }
            screen_fill(s, iy, dx + 1, w - 2, 1, (uint32_t)' ', st);
            if (it->checkable == 2) {
                screen_put(s, iy, dx + 1, (uint32_t)'*', st);
            }
            screen_puts(s, iy, dx + 2, it->label, st);
            if (it->accel) {
                int ax = dx + w - 2 - (int)strlen(it->accel);
                if (ax > dx + 2) {
                    screen_puts(s, iy, ax, it->accel, st);
                }
            }
        }
    }
}
