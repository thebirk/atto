        typedef struct {
        int fg;
        int bg;
    } ColorPair;
    
    enum {
        COLOR_NORMAL,
        COLOR_KEYWORD,
        COLOR_COMMENT,
        COLOR_STRING,
        COLOR_NUMBER,
        COLOR_PREPROC,
        COLOR_OPERATOR,
    };
    
    // Has to match with the above
    ColorPair c_colors[] = {
        {TB_DEFAULT, TB_DEFAULT},
        {TB_GREEN, TB_DEFAULT},
        {TB_YELLOW, TB_DEFAULT},
        {TB_YELLOW, TB_DEFAULT},
        {TB_RED, TB_DEFAULT},
        {TB_MAGENTA, TB_DEFAULT},
        {TB_BLUE, TB_DEFAULT},
    };
    
    
    typedef enum {
        LEXER_NONE = 0,
        LEXER_COMMENT,
        LEXER_KEYWORD,
        LEXER_OPERATOR,
        LEXER_STRING,
        LEXER_PREPROC,
        LEXER_NUMBER,
        LEXER_SLASH_OR_COMMENT,
    } LexerState;
    
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
    
    void c_lexer(LexerState *state, char c, ColorPair *color) {
        switch(*state) {
            case LEXER_COMMENT: {
                
            } break;
            case LEXER_KEYWORD: {
                
            } break;
            case LEXER_STRING: {
                if(c == '"') {
                    *state = LEXER_NONE;
                }
                *color = c_colors[COLOR_STRING];
            } break;
            case LEXER_OPERATOR: {
                
            } break;
            case LEXER_PREPROC: {
                if(!is_alpha(c)) {
                    *state = LEXER_NONE;
                    *color = c_colors[COLOR_NORMAL];
                    return;
                }
                *color = c_colors[COLOR_PREPROC];
            } break;
            case LEXER_NUMBER: {
                if(!is_num(c)) {
                    *state = LEXER_NONE;
                    *color = c_colors[COLOR_NORMAL];
                    return;
                }
                *color = c_colors[COLOR_NUMBER];
            } break;
            
            case LEXER_NONE:
            default: {
                switch(c) {
                    case '"': {
                        *state = LEXER_STRING;
                        *color = c_colors[COLOR_STRING];
                        return;
                    } break;
                    case '#': {
                        *state = LEXER_PREPROC;
                        *color = c_colors[COLOR_PREPROC];
                        return;
                    } break;
                }
                
                if(is_num(c)) {
                    *state = LEXER_NUMBER;
                    *color = c_colors[COLOR_NUMBER];
                    return;
                }
                
                *color = c_colors[COLOR_NORMAL];
            } break;
        }
    }
