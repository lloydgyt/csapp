#include "csim.h"

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cachelab.h"
// use #if to toggle verbose?
int hit_count = 0, miss_count = 0, eviction_count = 0;
bool verbose = false;

void init_cache(cache_line *cache, int set_size, int associtivity) {
    for (size_t i = 0; i < set_size; i++) { // TODO weird conversion!!
        int LRU = 0;
        for (size_t j = 0; j < associtivity; j++) {
            int index = i * associtivity + j;
            cache[index].valid = 0;
            cache[index].LRU_bits = LRU; // TODO how to minimize memory access?
            LRU++;                       // from 0 to associtivity - 1
        }
    }
}

void update_LRU(int block_index, int set_index, cache_line *cache, int set_bit,
                int associtivity) {
    int base_index = set_index * associtivity;
    for (size_t j = 0; j < associtivity; j++) {
        int new_LRU_bits = cache[base_index + j].LRU_bits;
        if (j == block_index) {
            new_LRU_bits = 0;
        } else if (new_LRU_bits != associtivity - 1) {
            new_LRU_bits += 1;
        }
        assert(new_LRU_bits < associtivity && new_LRU_bits >= 0);
        cache[base_index + j].LRU_bits = new_LRU_bits;
    }
}

void replace_cache_line(unsigned long address, int set_index, cache_line *cache,
                        int set_bit, int associtivity) {
    int base_index = set_index * associtivity;
    for (size_t j = 0; j < associtivity; j++) {
        if (cache[base_index + j].LRU_bits != associtivity - 1)
            continue;
        cache[base_index + j].address = address;
        update_LRU(j, set_index, cache, set_bit, associtivity);
        return;
    }
}

void access_cache(unsigned long address, cache_line *cache, int set_bit,
                  int associtivity, int block_bit) {
    /* hit count, miss count, eviction count
        we need to know current lines in Cache
        use TIO to compare (the layout of TIO is set by argument options)
    */
    // parse its address - get T, I (O is not important)
    // go to Ith-set, compare each "valid" lines with their Tags
    unsigned long mask_T = (-1) << (set_bit + block_bit);
    unsigned long mask_I = ((1 << set_bit) - 1) << block_bit;
    unsigned long I = address & mask_I;
    int base_index = (I >> block_bit) * associtivity;
    bool success = false;
    for (size_t j = 0; j < associtivity; j++) {
        if (cache[base_index + j].valid == 0)
            continue;
        if ((address & mask_T) == (cache[base_index + j].address & mask_T)) {
            success = true;
            hit_count += 1;
            // update LRU
            update_LRU(j, I >> block_bit, cache, set_bit, associtivity);
            if (verbose) {
                printf(" hit");
            }

            return;
        }
    }
    assert(!success);
    if (verbose) {
        printf(" miss");
    }
    miss_count += 1;
    // find a place
    bool has_space = false;
    for (size_t j = 0; j < associtivity; j++) {
        if (cache[base_index + j].valid != 0)
            continue;
        cache[base_index + j].valid = 1;
        cache[base_index + j].address = address;
        // TODO update LRU
        update_LRU(j, I >> block_bit, cache, set_bit, associtivity);
        // TODO verbose?
        return;
    }
    assert(!has_space);
    if (verbose) {
        printf(" eviction");
    }
    eviction_count += 1;
    // TODO implement later!
    replace_cache_line(address, I >> block_bit, cache, set_bit, associtivity);
    return;
}

int main(int argc, char **argv) {
    // handle cli args (options and file)
    int opt;
    char *trace_filename = NULL;
    int set_bit = -1, associtivity = -1, block_bit = -1;

    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        // printf("opt = %c, optarg = %s\n", opt, optarg);
        switch (opt) {
        case 'v':
            verbose = true;
            break;
        case 's':
            set_bit = atoi(optarg);
            assert(set_bit != -1);
            break;
        case 'E':
            associtivity = atoi(optarg);
            assert(associtivity != -1);
            break;
        case 'b':
            block_bit = atoi(optarg);
            assert(block_bit != -1);
            break;
        case 't':
            trace_filename = optarg;
            assert(trace_filename != NULL);
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
    } // TODO how to refactor

    int set_size = 2 << set_bit;
    int block_size = 2 << block_bit;

    if (verbose) {
        printf("number of set = %d\n", set_size);
        printf("number of lines in a set = %d\n", associtivity);
        printf("size of a block = %d\n", block_size);
        printf("trace_filename= %s\n", trace_filename);
    }
    /*
        set up DS for Cache
    */
    cache_line *cache =
        (cache_line *)malloc(set_size * associtivity * sizeof(cache_line));
    init_cache(cache, set_size, associtivity);

    /* TODO parse trace file (action and address)
        M = access 2
        L and S = access 1
        address - hex format, turn to binary
    */
    FILE *file = fopen(trace_filename, "r");
    if (!file) {
        printf("Error opening file"); // TODO what is this perror()?
        return 1;
    }
    char buffer[20];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != ' ')
            continue; // ignore "I" (without " ")

        char type;
        unsigned long address;
        int size;
        int match = sscanf(buffer, " %c %lx,%d\n", &type, &address, &size);
        assert(match == 3);
        if (verbose) {
            switch (type) {
            case 'M':
                printf("M %lx,%d", address, size);
                access_cache(address, cache, set_bit, associtivity, block_bit);
                access_cache(address, cache, set_bit, associtivity, block_bit);
                printf("\n");
                break;
            case 'L':
                printf("L %lx,%d", address, size);
                access_cache(address, cache, set_bit, associtivity, block_bit);
                printf("\n");
                break;
            case 'S':
                printf("S %lx,%d", address, size);
                access_cache(address, cache, set_bit, associtivity, block_bit);
                printf("\n");
                break;
            default:
                // TODO error
                printf("Error type\n");
                break;
            }
        } else {
            switch (type) {
            case 'M':
                access_cache(address, cache, set_bit, associtivity, block_bit);
                access_cache(address, cache, set_bit, associtivity, block_bit);
                break;
            case 'L':
                access_cache(address, cache, set_bit, associtivity, block_bit);
                break;
            case 'S':
                access_cache(address, cache, set_bit, associtivity, block_bit);
                break;
            default:
                // TODO error
                printf("Error type\n");
                break;
            }
        }
    }

    printSummary(hit_count, miss_count, eviction_count);
    free(cache); // TODO when to free?
    return 0;
}
