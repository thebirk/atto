typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;

#define Array(_type) struct { \
    _type *data; \
    u64 size; \
    u64 cap; \
}

// Inclusive [_start, _end]
#define fori(_it, _start, _end) for(int _it = (_start); _it <= (_end); _it++)
// Exclusive [_start, _end}
#define fore(_it, _start, _end) for(int _it = (_start); _it < (_end); _it++)
// Exclusive with non-local variable
#define forev(_it, _start, _end) for((_it) = (_start); (_it) < (_end); (_it)++)

#define for_array(_arr, _val) for(int it_index = 0; (_val) = (_arr).data[it_index], it_index < (_arr).size; it_index++, (_val) = (_arr).data[it_index])

#define array_init(_arr, _cap) do { \
    assert((_arr).data == 0 && ("Called array_init with an array that is not zero!")); \
    assert((_cap) >= 0 && ("Called array_init with cap less than zero!")); \
    if((_cap) == 0) break; \
    (_arr).cap = (_cap); \
    (_arr).size = 0; \
    (_arr).data = malloc((_arr).cap * sizeof(*((_arr).data))); \
} while(0)

#define array_free(_arr) do { \
    free((_arr).data); \
    (_arr).data = 0; \
    (_arr).size = 0; \
    (_arr).cap = 0; \
} while(0)

#define array_clear(_arr) do { \
    (_arr).size = 0; \
} while(0)

#define array_add(_arr, _el) do { \
    (_arr).size++; \
    if((_arr).size <= (_arr).cap) { \
        (_arr).data[(_arr).size-1] = (_el); \
    } else { \
        if((_arr).cap >= 1024) { \
            (_arr).cap += 128; \
        } else { \
            if((_arr).cap == 0) { \
                (_arr).cap = 2; \
            } else { \
                (_arr).cap *= 2; \
            } \
        } \
        (_arr).data = realloc((_arr).data, (_arr).cap * sizeof(*((_arr).data))); \
        (_arr).data[(_arr).size-1] = (_el); \
    } \
} while(0)