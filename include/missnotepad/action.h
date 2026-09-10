#ifndef MISSNOTEPAD_ACTION_H
#define MISSNOTEPAD_ACTION_H

typedef enum {
    ACT_NONE = 0,
    ACT_NEW,
    ACT_OPEN,
    ACT_SAVE,
    ACT_SAVE_AS,
    ACT_EXIT,
    ACT_UNDO,
    ACT_REDO,
    ACT_CUT,
    ACT_COPY,
    ACT_PASTE,
    ACT_DELETE,
    ACT_SELECT_ALL,
    ACT_FIND,
    ACT_FIND_NEXT,
    ACT_REPLACE,
    ACT_WORD_WRAP,
    ACT_LINE_NUMBERS,
    ACT_ABOUT,
    ACT_HELP_KEYS,
    ACT_THEME_NOTEPAD,
    ACT_THEME_NANO,
    ACT_THEME_VI,
    ACT_THEME_EMACS,
    ACT_MOVE_LEFT,
    ACT_MOVE_RIGHT,
    ACT_MOVE_UP,
    ACT_MOVE_DOWN,
    ACT_MOVE_HOME,
    ACT_MOVE_END,
    ACT_KILL_LINE,
    ACT_VI_COLON,
    /* nano extras: actions the editor core supports but which only the
     * nano theme exposes directly. */
    ACT_INSERT_FILE,
    ACT_PAGE_UP,
    ACT_PAGE_DOWN,
    ACT_DOC_HOME,
    ACT_DOC_END,
    ACT_CURSOR_POS,
    ACT_DELETE_WORD_LEFT,
    ACT_DELETE_WORD_RIGHT,
    ACT_GOTO_LINE
} Action;

#endif
