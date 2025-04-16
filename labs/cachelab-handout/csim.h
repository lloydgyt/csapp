typedef struct {
    unsigned long address;
    unsigned int LRU_bits;
    bool valid;
} cache_line;

typedef struct { // TODO not useful
    unsigned long address;
    char type;
} access;
