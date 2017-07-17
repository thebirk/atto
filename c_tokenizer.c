/* 

Split the system into dynamic system loading the parser at runtime
load a struct containing standard functions

*/

typedef struct {
    char *str;
    int len;
    int fg;
    int bg;
} HLString;

typedef enum {
    LAST_NONE = 0,
    LAST_PREPROC,
} TokenizerLast;

typedef struct {
    char *buffer;
    int offset;
    
    TokenizerLast last;
} TokenizerState;

bool is_upper(int c) {
    return (c >= 'A') && (c <= 'Z');
}

bool is_lower(int c) {
    return (c >= 'a') && (c <= 'z');
}

bool is_alpha(int c) {
    return is_upper(c) || is_lower(c);
}

bool is_num(int c) {
    return (c >= '0') && (c <= '9');
}

bool is_alnum(int c) {
    return is_alpha(c) || is_num(c);
}

bool is_ident(int c) {
    return is_alnum(c) || c == '_';
}

void init_c_tokenizer(TokenizerState *state, char *buffer) {
    state->buffer = buffer;
    state->offset = 0;
    state->last = LAST_NONE;
}

bool compare_hl_string(HLString *a, char *b) {
    int b_len = strlen(b);
    if(a->len != b_len) return false;
    for(int i = 0; i < b_len; i++) {
        if(a->str[i] != b[i]) return false;
    }
    return true;
}

void check_ident(HLString *str) {
    if(compare_hl_string(str, "int")) {
        str->fg = TB_GREEN;
        str->bg = TB_DEFAULT;
        return;
    }
#define ISTYPE(_str) else if(compare_hl_string(str, _str)) { str->fg = TB_GREEN; str->bg = TB_DEFAULT; return; }
    ISTYPE("char")
        ISTYPE("short")
        ISTYPE("long")
        ISTYPE("bool")
#undef ISTYPE
#define ISKEYWORD(_str) else if(compare_hl_string(str, _str)) { str->fg = TB_WHITE; str->bg = TB_DEFAULT; return; }
    ISKEYWORD("return")
        ISKEYWORD("if")
        ISKEYWORD("else")
        ISKEYWORD("switch")
        ISKEYWORD("break")
        ISKEYWORD("continue")
        ISKEYWORD("extern")
        ISKEYWORD("auto")
        ISKEYWORD("do")
        ISKEYWORD("while")
        ISKEYWORD("for")
        ISKEYWORD("static")
        ISKEYWORD("volatile")
        ISKEYWORD("const")
        ISKEYWORD("enum")
        ISKEYWORD("struct")
        ISKEYWORD("typedef")
        ISKEYWORD("goto")
        ISKEYWORD("register")
        ISKEYWORD("signed")
        ISKEYWORD("unsigned")
        ISKEYWORD("sizeof")
        ISKEYWORD("union")
        ISKEYWORD("inline")
        ISKEYWORD("asm")
        ISKEYWORD("__asm")
        ISKEYWORD("typeof")
#undef ISKEYWORD
    
    // TODO: true/false
    str->fg = TB_DEFAULT;
    str->bg = TB_DEFAULT;
}

bool c_tokenize(Txe *txe, TokenizerState *state, HLString *str) {
    int start = state->offset;
    
    char ch = state->buffer[state->offset];
    if(ch == 0) return false;
    
    if(state->last == LAST_PREPROC) {
        if(ch == '<') {
            state->offset++;
            // keep last
            str->str = state->buffer + state->offset - 1;
            str->len = 1;
            str->fg = TB_YELLOW;
            str->bg = TB_DEFAULT;
            
            return true;
        }
        if(ch == '>') {
            state->offset++;
            // dont keep last
            state->last = LAST_NONE;
            str->str = state->buffer + state->offset - 1;
            str->len = 1;
            str->fg = TB_YELLOW;
            str->bg = TB_DEFAULT;
            
            return true;
        }
    }
    
    if(ch == '"') {
        state->offset++;
        while(is_alpha(state->buffer[state->offset]) || (state->buffer[state->offset] == '"')) {
            state->offset++;
            if(state->buffer[state->offset-1] == '"') break;
        }
        
        state->last = LAST_NONE;
        str->str = state->buffer+state->offset;
        str->len = state->offset - start;
        str->fg = TB_RED;
        str->bg = TB_DEFAULT;
        
        return true;
    }
    
    if(ch == '#') {
        state->offset++;
        while(is_alpha(state->buffer[state->offset])) {
            state->offset++;
        }
        
        state->last = LAST_PREPROC;
        str->str = state->buffer+state->offset-1;
        str->len = state->offset - start;
        str->fg = TB_MAGENTA;
        str->bg = TB_DEFAULT;
        
        return true;
    }
    
    if(ch == '/' && state->buffer[state->offset+1]) {
        state->offset += 2;
        
        while(state->buffer[state->offset] != '\n') {
            state->offset++;
        }
        
        state->last = LAST_NONE;
        str->str = state->buffer + state->offset-1;
        str->len = state->offset - start;
        str->fg = TB_YELLOW;
        str->bg = TB_DEFAULT;
        
        return true;
    }
    
    if(is_alpha(ch)) {
        state->offset++;
        while(is_ident(state->buffer[state->offset])) {
            state->offset++;
        }
        
        state->last = LAST_NONE;
        str->str = state->buffer+state->offset-1;
        str->len = state->offset - start;
        
        check_ident(str);
        
        return true;
    }
    
    str->fg = TB_DEFAULT;
    str->bg = TB_DEFAULT;
    return false;
}