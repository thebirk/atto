#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <curses.h>

#include "array.c"
#include "buffer.c"

#define STATUS_BAR_SIZE 1

FILE *__log_file;
#define LOG(...) fprintf(__log_file, __VA_ARGS__)

typedef struct {
    WINDOW *curses_window;
    int terminal_height;
    int height;
    int width;
    
    bool running;
    
    int tab_width;
    
    Buffer *current_buffer;
} Txe;

void screen_resize(Txe *txe, int w, int h) {
    txe->terminal_height = h;
    txe->height = h - STATUS_BAR_SIZE;
    txe->width = w;
}

void init_curses(Txe *txe) {
    txe->curses_window = initscr();
    raw();
    cbreak();
    noecho();
    keypad(txe->curses_window, true);
    
    screen_resize(txe, COLS, LINES);
}

void redraw_text(Txe *txe) {
    move(STATUS_BAR_SIZE, 0);
    char *buff = buffer_stringify(txe->current_buffer);
    
    for(int i = 0; buff[i]; i++) {
        char ch = buff[i];
        if(i == txe->current_buffer->pre) {
            attron(A_REVERSE);
            if(ch == '\n') {
                addch(' ');
                addch('\n');
            } else if(ch == '\t') {
                addch(' ');
                attroff(A_REVERSE);
                fore(i, 1, txe->tab_width) {
                    addch(' ');
                }
            } else {
                addch(ch);
            }
            attroff(A_REVERSE);
        } else if (ch == '\t') {
            fore(i, 0, txe->tab_width) {
                addch(' ');
            }
        } else {
            addch(ch);
        }
    }
    
    curs_set(0);
}

void draw_status_line(Txe *txe) {
    attron(A_REVERSE);
    move(0, 0);
    
    fore(i, 0, txe->width) {
        addch(' ');
    }
    
    move(0, 0);
    printw("some_file.c L#12 C#32");
    attroff(A_REVERSE);
}

void redraw(Txe *txe) {
    clear();
    draw_status_line(txe);
    redraw_text(txe);
}

void load_settings(Txe *txe) {
    txe->tab_width = 4;
}

int main() {
    __log_file = fopen("out.log", "w");
    
    Buffer *buffer = buffer_create(32);
    buffer_insert_string(buffer, "Hello, world!\n");
    buffer_move(buffer, -5);
    buffer_insert_string(buffer, "\nlol");
    buffer_seek_end(buffer);
    
    Txe txe = {0};
    init_curses(&txe);
    load_settings(&txe);
    txe.current_buffer = buffer;
    
    redraw(&txe);
    
    txe.running = true;
    while(txe.running) {
        int ch = 0;
        int ret = get_wch(&ch);
        
        if(ret == KEY_CODE_YES) {
            switch(ch) {
                case KEY_LEFT: {
                    buffer_move_left(txe.current_buffer);
                } break;
                case KEY_RIGHT: {
                    buffer_move_right(txe.current_buffer);
                } break;
                case KEY_BACKSPACE: {
                    buffer_remove(txe.current_buffer);
                } break;
                case KEY_RESIZE: {
                    screen_resize(&txe, COLS, LINES);
                } break;
            }
        } else if(ret == OK) {
            if (ch == 'Q') {
                txe.running = false;
            } else {
                buffer_insert(txe.current_buffer, ch);
            }
        }
        
        redraw(&txe);
    }
    
    endwin();
    
    return 0;
}