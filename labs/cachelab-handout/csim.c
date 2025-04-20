#include "csim.h"
#include "cachelab.h"

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int hit_count = 0, miss_count = 0, eviction_count = 0;
bool verbose = false;
int set_bit = -1, associtivity = -1, block_bit = -1;
char *trace_filename = NULL;

void init_cache(cache_line *cache) {
    size_t set_size = 1 << set_bit;
    for (size_t i = 0; i < set_size; i++) {
        int LRU = 0;
        for (size_t j = 0; j < associtivity; j++) {
            int index = i * associtivity + j;
            cache[index].valid = 0;
            cache[index].LRU_bits = LRU;
            LRU++; // from 0 to associtivity - 1
        }
    }
}

void update_LRU(int block_index, cache_line *set_select) {
    for (size_t j = 0; j < associtivity; j++) {
        int new_LRU_bits = set_select[j].LRU_bits;
        if (j == block_index) {
            new_LRU_bits = 0;
        } else if (new_LRU_bits != associtivity - 1) {
            new_LRU_bits += 1;
        } // else, left unchaged
        assert(new_LRU_bits < associtivity && new_LRU_bits >= 0);
        set_select[j].LRU_bits = new_LRU_bits;
    }
}

void replace_cache_line(unsigned long tag, cache_line *set_select) {
    eviction_count += 1;
    for (size_t j = 0; j < associtivity; j++) {
        if (set_select[j].LRU_bits != associtivity - 1) {
            continue;
        }
        if (verbose) {
            printf(" eviction");
        }
        set_select[j].tag = tag;
        update_LRU(j, set_select);
        return;
    }
}

bool load_to_empty(unsigned long tag, cache_line *set_select) {
    miss_count += 1;
    if (verbose) {
        printf(" miss");
    }
    for (size_t j = 0; j < associtivity; j++) {
        if (set_select[j].valid != 0) {
            continue;
        }
        set_select[j].valid = 1;
        set_select[j].tag = tag;
        update_LRU(j, set_select);
        return true;
    }
    return false;
}

bool find_target(unsigned long tag, cache_line *set_select) {
    for (size_t j = 0; j < associtivity; j++) {
        if (set_select[j].valid == 0) {
            continue;
        }
        if (tag == set_select[j].tag) {
            hit_count += 1;
            update_LRU(j, set_select);
            if (verbose) {
                printf(" hit");
            }
            return true;
        }
    }
    return false;
}

void access_cache(unsigned long address, cache_line *cache) {
    /* hit count, miss count, eviction count
        we need to know current lines in Cache
        use TIO to compare (the layout of TIO is set by argument options)
    */
    // parse its address - get T, I (O is not important)
    // go to Ith-set, compare each "valid" lines with their Tags
    unsigned long mask_I = ((1 << set_bit) - 1) << block_bit;
    unsigned long I = (address & mask_I) >> block_bit;
    unsigned long T = address >> (block_bit + set_bit);
    cache_line *set_select = cache + I * associtivity;
    if (find_target(T, set_select)) {
        return;
    }

    if (load_to_empty(T, set_select)) {
        return;
    }

    replace_cache_line(T, set_select);
    return;
}

int main(int argc, char **argv) {
    // handle cli args (options and file)
    int opt;

    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        // printf("opt = %c, optarg = %s\n", opt, optarg);
        switch (opt) {
        case 'v':
            verbose = true;
            break;
        case 's':
            set_bit = atoi(optarg);
            break;
        case 'E':
            associtivity = atoi(optarg);
            break;
        case 'b':
            block_bit = atoi(optarg);
            break;
        case 't':
            trace_filename = optarg;
            break;
        default: // TODO seems to be handled by getopt()
            fprintf(stderr, "Unknown option: -%c\n", optopt);
            return 1;
        }
    }
    // check if all necessary option is set
    if (set_bit == -1) {
        fprintf(stderr, "Please include -s option\n");
        return 1;
    }
    if (associtivity == -1) {
        fprintf(stderr, "Please include -E option\n");
        return 1;
    }
    if (block_bit == -1) {
        fprintf(stderr, "Please include -b option\n");
        return 1;
    }
    if (trace_filename == NULL) {
        fprintf(stderr, "Please include -t option\n");
        return 1;
    }

    size_t set_size = 1 << set_bit;
    size_t block_size = 1 << block_bit;

    if (verbose) {
        printf("number of set = %ld\n", set_size);
        printf("number of lines in a set = %d\n", associtivity);
        printf("size of a block = %ld\n", block_size);
        printf("trace_filename= %s\n", trace_filename);
    }
    // set up DS for Cache
    cache_line *cache =
        (cache_line *)malloc(set_size * associtivity * sizeof(cache_line));
    init_cache(cache);

    /* parse trace file (action and address)
        M = access 2
        L and S = access 1
        address - hex format, turn to binary
    */
    FILE *file = fopen(trace_filename, "r");
    if (!file) {
        printf("Error opening file");
        return 1;
    }
    char buffer[20];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != ' ') {
            continue; // ignore "I" (without " ")
        }
        char type;
        unsigned long address;
        int size;
        assert(sscanf(buffer, " %c %lx,%d\n", &type, &address, &size) == 3);
        if (verbose) {
            printf("%c %lx,%d", type, address, size);
        }
        switch (type) {
        case 'M':
            access_cache(address, cache);
            access_cache(address, cache);
            break;
        case 'L':
        case 'S':
            access_cache(address, cache);
            break;
        default: // error
            printf("Error type\n");
            exit(1);
        }
        if (verbose) {
            printf("\n");
        }
    }

    // Always check fclose for write errors
    if (fclose(file) != 0) {
        perror("Error closing file");
        return 1;
    }

    printSummary(hit_count, miss_count, eviction_count);
    free(cache);
    return 0;
}
