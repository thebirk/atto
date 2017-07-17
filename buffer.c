// Internal structures
typedef struct {
    char *data;
    int pre;
    int post;
    int size;
    
    char *string;
    bool dirty;
} Buffer;

// Internal functions
void buffer_expand(Buffer *b)
{
    int nsize = b->size * 2;
    char *ndata = malloc(nsize);
    
    for(int i = 0; i < b->pre; i++) {
        ndata[i] = b->data[i];
    }
    
    for(int i = 0; i < b->post; i++) {
        ndata[nsize-i-1] = b->data[b->size-i-1];
    }
    
    b->size = nsize;
    free(b->data);
    b->data = ndata;
    b->dirty = true;
}

// Public functions

Buffer* buffer_create(int size)
{
    assert(size > 0);
    
    Buffer *b = malloc(sizeof(Buffer));
    if(b == 0) {
        return 0;
    }
    
    b->size = size;
    b->dirty = true;
    b->string = 0;
    b->data = malloc(size);
    if(b->data == 0) {
        free(b);
        return 0;
    }
    b->post = 0;
    b->pre = 0;
    
    return b;
}

void buffer_destroy(Buffer* handle)
{
    Buffer *b = handle;
    
    free(b->string);
    free(b->data);
    free(b);
}

int buffer_get_offset(Buffer* handle)
{
    return ((Buffer*) handle)->pre;
}

void buffer_insert(Buffer* handle, char c)
{
    Buffer *b = handle;
    
    if(b->pre + b->post == b->size) {
        buffer_expand(b);
    }
    b->data[b->pre] = c;
    b->pre++;
    b->dirty = true;
}

void buffer_insert_string(Buffer* handle, const char *str)
{
    assert(str);
    while(*str) {
        buffer_insert(handle, *str++);
    }
    ((Buffer*)handle)->dirty = true;
}

void buffer_remove(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->pre > 0) {
        b->pre--;
        b->dirty = true;
    }
}

void buffer_move_left(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->pre > 0) {
        char c = b->data[b->pre-1];
        b->data[b->size - b->post - 1] = c;
        b->pre--;
        b->post++;
        b->dirty = true;
    }
}

void buffer_move_right(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->post > 0) {
        b->data[b->pre] = b->data[b->size - b->post];
        b->pre++;
        b->post--;
        b->dirty = true;
    }
}

void buffer_move(Buffer* handle, int distance)
{
    if(distance > 0) {
        for(int i = 0; i < distance; i++) {
            buffer_move_right(handle);
        }
        ((Buffer*)handle)->dirty = true;
    } else {
        distance = -distance;
        for(int i = 0; i < distance; i++) {
            buffer_move_left(handle);
        }
        ((Buffer*)handle)->dirty = true;
    }
}

void buffer_seek(Buffer* handle, int pos)
{
    Buffer *b = handle;
    buffer_move(handle, pos - b->pre);
    b->dirty = true;
}

void buffer_seek_home(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->pre > 0) {
        char c = b->data[b->pre-1];
        if(c == '\n') {
            return; // Already home
        }
        
        while(b->data[b->pre-1] != '\n') {
            if(b->pre == 0) break;
            buffer_move_left(b);
        }
    }
} 

void buffer_seek_end(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->post > 0) {
        char c = b->data[b->size-b->post];
        if(c == '\n') {
            return; // Already at the end
        }
        
        while(b->data[b->size-b->post] != '\n') {
            if(b->post == 0) break;
            buffer_move_right(b);
        }
    }
}

void buffer_seek_up(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->pre > 0) {
        char c = b->data[b->pre-1];
        if(c == '\n') {
            buffer_move_left(b);
            while(b->data[b->pre-1] != '\n') {
                if(b->pre == 0) return;
                buffer_move_left(b);
            }
        } else {
            
        }
    }
}

void buffer_seek_down(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->post > 0) {
        
        int offset = 0;
        while(b->data[b->size-b->post+offset] != '\n') {
            if(b->post == 0) break;
            offset++;
        }
        printf("offset: %d\n", offset);
        
        if(offset == 0) {
            buffer_move_right(b);
            while(b->data[b->size-b->post] != '\n') {
                buffer_move_right(b);
            }
        }
        
    } else {
        
    }
}

char* buffer_stringify(Buffer* handle)
{
    Buffer *b = handle;
    
    if(b->dirty) {
        int size = b->pre + b->post;
        char *str = malloc(size+1);
        
        for(int i = 0; i < b->pre; i++) {
            str[i] = b->data[i];
        }
        
        for(int i = 0; i < b->post; i++) {
            str[size-i-1] = b->data[b->size-i-1];
        }
        
        str[size] = 0;
        free(b->string);
        b->string = str;
        b->dirty = false;
        return b->string;
    } else {
        return b->string;
    }
}

int buffer_get_line_count(Buffer *buffer) {
    char *str = buffer_stringify(buffer);
    int result = 1;
    char c;
    while((c = *str)) {
        if(c == '\n') result++;
        str++;
    }
    
    return result;
}