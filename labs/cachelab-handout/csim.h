typedef struct { // TODO can only use Tag, not entire address!
    unsigned long address;
    unsigned int LRU_bits;
    bool valid; // TODO change code to use bool instead (use int by now)
} cache_line;

typedef struct { // TODO not useful
    unsigned long address;
    char type;
} access;
