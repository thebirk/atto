#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "termbox/src/termbox.h"
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdarg.h>

#include "array.c"
#include "buffer.c"

#define STATUS_BAR_SIZE 1

FILE *__log_file;
#define LOG(...) fprintf(__log_file, __VA_ARGS__)

typedef struct TxeBuffer TxeBuffer;
struct TxeBuffer {
    Buffer *buffer;
    char *name;
    TxeBuffer *next;
    TxeBuffer *prev;
};

typedef struct {
    int tab_width;
    int newline_visual;
    int tab_visual;
    
    bool draw_line_numbers;
    char line_numbers_character;
    
    bool show_special_characters;
    int current_newline_visual;
    int current_tab_visual;
} TxeSettings;

typedef struct {
    int terminal_height;
    int height;
    int width;
    
    bool running;
    
    TxeSettings settings;
    
    TxeBuffer *current_buffer;
    int cursor_x;
    int cursor_y;
} Txe;

void screen_resize(Txe *txe, int w, int h) {
    txe->terminal_height = h;
    txe->height = h - STATUS_BAR_SIZE;
    txe->width = w;
}

void init_termbox(Txe *txe) {
    tb_init();
    tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
    screen_resize(txe, tb_width(), tb_height());
}

void update_cursor(Txe *txe, int x, int y) {
    txe->cursor_x = x;
    txe->cursor_y = y;
}

int draw_string(int x, int y, int fg, int bg, char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[2048];
    vsnprintf(buffer, 2048, format, args);
    
    int written = 0;
    char *ptr = buffer;
    while(*ptr) {
        tb_change_cell(x, y, *ptr, fg, bg);
        ptr++;
        x++;
        written++;
    }
    
    va_end(args);
    return written;
}

int draw_line_numbers(Txe *txe) {
    if(!txe->settings.draw_line_numbers) {
        return 0;
    }
    
    int total_lines = buffer_get_line_count(txe->current_buffer->buffer);
    int width = 0;
    int yy = total_lines;
    while(yy) {
        yy /= 10;
        width++;
    }
    
    for(int y = STATUS_BAR_SIZE; y < total_lines; y++) {
        int len = draw_string(0, y, TB_GREEN, TB_DEFAULT, "%*d", width, y+1-STATUS_BAR_SIZE);
        if(txe->settings.line_numbers_character != 0) {
            draw_string(len, y, TB_GREEN, TB_DEFAULT, "%c", txe->settings.line_numbers_character);
        }
    }
    
    if(txe->settings.line_numbers_character != 0) width++;
    return width;
}

typedef struct {
    int fg;
    int bg;
} ColorPair;

#include "c_tokenizer.c"

void redraw_text(Txe *txe) {
    char *buff = buffer_stringify(txe->current_buffer->buffer);
    
    int lineno_width = draw_line_numbers(txe);
    
    int x = 0;
    int y = STATUS_BAR_SIZE;
    TokenizerState state = {0};
    init_c_tokenizer(&state, buff);
    HLString str = {0};
    
    // store string outside in the same manner as the current step
    // method, print each character and just check if there is characters
    // remaing in the hlstring buffer
    // this way we keep the for loop rendering and get proper
    // highlighting
    
    // OR
    // change to the old state machine way
    // use the buffer itself todo the lookahead stupid
    // add a printout state which just keeps printing out a prevoisly
    // lexed token, easy, profit
    
    int step = 0;
    for(int i = 0; buff[i]; i++) {
        if(step > 0) {
            if(i == txe->current_buffer->buffer->pre) {
                if((x+lineno_width < 0 || x+lineno_width >= txe->width || y < 0 || y >= txe->terminal_height)) {
                    update_cursor(txe, x+lineno_width, y);
                    tb_set_cursor(-1, -1);
                } else {
                    update_cursor(txe, x, y);
                    tb_set_cursor(x+lineno_width, y);
                }
            }
            x++;
            step--;
            continue;
        }
        
        if(i == txe->current_buffer->buffer->pre) {
            if((x+lineno_width < 0 || x+lineno_width >= txe->width || y < 0 || y >= txe->terminal_height)) {
                update_cursor(txe, x+lineno_width, y);
                tb_set_cursor(-1, -1);
            } else {
                update_cursor(txe, x, y);
                tb_set_cursor(x+lineno_width, y);
            }
        }
        
        if(c_tokenize(txe, &state, &str)) {
            step = str.len-1;
            draw_string(x+lineno_width, y, str.fg, str.bg, "%.*s", str.len, str.str);
            x++;
        } else {
            // print char
            // normal increment
            if(buff[i] == '\n') {
                tb_change_cell(x+lineno_width, y, txe->settings.current_newline_visual, TB_DEFAULT, TB_DEFAULT);
                y++;
                x = 0;
            } else if(buff[i] == '\t') {
                tb_change_cell(x+lineno_width, y,   txe->settings.current_tab_visual, TB_DEFAULT, TB_DEFAULT);
                fore(i, 1, txe->settings.tab_width) {
                    tb_change_cell(x+i+lineno_width, y, txe->settings.current_tab_visual, TB_DEFAULT, TB_DEFAULT);
                }
                x += txe->settings.tab_width;
            } else {
                tb_change_cell(x+lineno_width, y, buff[i], str.fg, str.bg);
                x++;
            }
            
        }
    }
    
    //LexerState lexer_state = LEXER_NONE;
    /*int x = 0;
    int y = STATUS_BAR_SIZE;
    for(int i = 0; buff[i]; i++) {
        if(i == txe->current_buffer->buffer->pre) {
            if((x+lineno_width < 0 || x+lineno_width >= txe->width || y < 0 || y >= txe->terminal_height)) {
                update_cursor(txe, x+lineno_width, y);
                tb_set_cursor(-1, -1);
            } else {
                update_cursor(txe, x, y);
                tb_set_cursor(x+lineno_width, y);
            }
        }
        
        ColorPair color;
        color.fg = TB_DEFAULT;
        color.bg = TB_DEFAULT;
        // c_lexer(&lexer_state, buff[i], &color);
        
        if(buff[i] == '\n') {
            tb_change_cell(x+lineno_width, y, txe->settings.current_newline_visual, TB_DEFAULT, TB_DEFAULT);
            y++;
            x = 0;
        } else if(buff[i] == '\t') {
            tb_change_cell(x+lineno_width, y,   txe->settings.current_tab_visual, TB_DEFAULT, TB_DEFAULT);
            fore(i, 1, txe->settings.tab_width) {
                tb_change_cell(x+i+lineno_width, y, txe->settings.current_tab_visual, TB_DEFAULT, TB_DEFAULT);
            }
            x += txe->settings.tab_width;
        } else {
            tb_change_cell(x+lineno_width, y, buff[i], color.fg, color.bg);
            x++;
        }
    }*/
}

void draw_status_line(Txe *txe) {
    char buffer[1024] = {0};
    snprintf(buffer, 1024, "%s - L#%d C#%d", txe->current_buffer->name ? txe->current_buffer->name : "NULL", txe->cursor_y, txe->cursor_x+1);
    
    time_t rawtime;
    time(&rawtime);
    struct tm *tim = localtime(&rawtime);
    
    int width = strlen(buffer);
    int remaining_width = txe->width - width;
    char time_buffer[64] = {0};
    snprintf(time_buffer, 64, "%02d:%02d:%02d", tim->tm_hour, tim->tm_min, tim->tm_sec);
    
    snprintf(buffer+width, 1024-width, "%*s", txe->width - width, time_buffer);
    
    fore(i, 0, txe->width) {
        if(buffer[i]) {
            tb_change_cell(i, 0, buffer[i], TB_DEFAULT|TB_REVERSE, TB_DEFAULT|TB_REVERSE);
        } else {
            tb_change_cell(i, 0, ' ', TB_DEFAULT|TB_REVERSE, TB_DEFAULT|TB_REVERSE);
        }
    }
}

void redraw(Txe *txe) {
    tb_set_clear_attributes(TB_DEFAULT, TB_DEFAULT);
    tb_clear();
    redraw_text(txe);
    draw_status_line(txe);
    tb_present();
}

void toggle_show_special_characters(Txe *txe) {
    txe->settings.show_special_characters = !txe->settings.show_special_characters;
    
    if(txe->settings.show_special_characters) {
        txe->settings.current_tab_visual = txe->settings.tab_visual;
        txe->settings.current_newline_visual = txe->settings.newline_visual;
    } else {
        txe->settings.current_tab_visual = ' ';
        txe->settings.current_newline_visual = ' ';
    }
}

void load_settings(Txe *txe) {
    txe->settings.tab_width = 4;
    
    txe->settings.newline_visual = '@';
    txe->settings.tab_visual = '$';
    txe->settings.show_special_characters = true;
    toggle_show_special_characters(txe);
    
    txe->settings.draw_line_numbers = true;
    txe->settings.line_numbers_character = '|';
}

void init_txe(Txe *txe) {
    load_settings(txe);
    
    txe->current_buffer = malloc(sizeof(TxeBuffer));
    txe->current_buffer->buffer = buffer_create(2);
    txe->current_buffer->name = 0;
    txe->current_buffer->next = txe->current_buffer;
    txe->current_buffer->prev = txe->current_buffer;
}

void open_empty_buffer(Txe *txe, char *name) {
    TxeBuffer *b = malloc(sizeof(TxeBuffer));
    b->buffer = buffer_create(2);
    b->name = strdup(name);
    TxeBuffer *other = txe->current_buffer;
    
    b->next = other->next;
    other->next->prev = b;
    other->next = b;
    b->prev = other;
    
    txe->current_buffer = b;
}

void open_file(Txe *txe, char *path) {
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    rewind(f);
    char *file = malloc(size+1);
    file[size] = 0;
    fread(file, 1, size, f);
    fclose(f);
    Buffer *buffer = buffer_create(size);
    buffer_insert_string(buffer, file);
    buffer_seek(buffer, 0);
    
    TxeBuffer *b = malloc(sizeof(TxeBuffer));
    b->buffer = buffer;;
    b->name = strdup(path);
    TxeBuffer *other = txe->current_buffer;
    
    b->next = other->next;
    other->next->prev = b;
    other->next = b;
    b->prev = other;
    
    txe->current_buffer = b;
}

/* 
NOTES:
- When implementing saving remeber that TxeBuffers with a NULL name
exists at you will have to prompt the user for a filename

TODO:

- Switch buffer.c to use uint32_t's
- Add a queary/prompt bar for user input
Needs to have a preceeding title and the ability to be prefilled
ex: "Name: <user input here>"
*/

int main() {
    __log_file = fopen("out.log", "w");
    
    Txe txe = {0};
    init_txe(&txe);
    init_termbox(&txe);
    
    /*
    signal(SIGALRM, signal_handler);
    alarm(1);
    */
    
    open_file(&txe, "txe.c");
    
    redraw(&txe);
    
    txe.running = true;
    while(txe.running) {
        struct tb_event e = {0};
        tb_poll_event(&e);
        switch(e.type) {
            case TB_EVENT_KEY: {
                if(e.ch != 0) {
                    buffer_insert(txe.current_buffer->buffer, (char)e.ch);
                } else if(e.key != 0) {
                    switch(e.key) {
                        case TB_KEY_ARROW_UP: {
                            buffer_seek_up(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_ARROW_DOWN: {
                            buffer_seek_down(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_HOME: {
                            buffer_seek_home(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_END: {
                            buffer_seek_end(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_CTRL_W: {
                            toggle_show_special_characters(&txe);
                        } break;
                        case TB_KEY_CTRL_A: {
                            txe.current_buffer = txe.current_buffer->prev;
                        } break;
                        case TB_KEY_CTRL_D: {
                            txe.current_buffer = txe.current_buffer->next;
                        } break;
                        case TB_KEY_SPACE: {
                            buffer_insert(txe.current_buffer->buffer, ' ');
                        } break;
                        case TB_KEY_ENTER: {
                            buffer_insert(txe.current_buffer->buffer, '\n');
                        } break;
                        case TB_KEY_TAB: {
                            buffer_insert(txe.current_buffer->buffer, '\t');
                        } break;
                        case TB_KEY_ARROW_LEFT: {
                            buffer_move_left(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_ARROW_RIGHT: {
                            buffer_move_right(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_BACKSPACE2:
                        case TB_KEY_BACKSPACE: {
                            buffer_remove(txe.current_buffer->buffer);
                        } break;
                        case TB_KEY_CTRL_Q: {
                            txe.running = false;
                        } break;
                    }
                    
                }
            } break;
            case TB_EVENT_RESIZE: {
                screen_resize(&txe, e.w, e.h);
            } break;
            case TB_EVENT_MOUSE: {
                
            } break;
            case TB_EVENT_TIMER: {
                LOG("TIMER\n");
            } break;
        }
        redraw(&txe);
    }
    
    tb_shutdown();
    
    return 0;
}