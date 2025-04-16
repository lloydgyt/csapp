#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <assert.h>
#include "csim.h"
// use #if to toggle verbose? 
int hit_count = 0, miss_count = 0, eviction_count = 0;

void access_cache(char type, unsigned long address, cache_line *cache) {
    hit_count += 1;
    // which policy? 
        // evicition policy? LRU
        // write policy? doesn't matter
    /* hit count, miss count, eviction count
        we need to know current lines in Cache
        use TIO to compare (the layout of TIO is set by argument options)
    */
    /*
        TODO need a DS to store valid bit, LRU bit, tags
        this DS is grouped into different set
    */
    /* TODO bit operation is involved
        for each line 
            get its action (special for M)
            parse its address - get T, I (O is not important)
            go to Ith-set, compare each "valid" lines with their Tags
            if hit:
                update hit_count and update LRU (policy bit)
            else:
                update miss count
                if full: evict first
                add that address, set valid bit and tags and LRU bits
    */

}

int main(int argc, char **argv)
{
    // handle cli args (options and file)
    int opt;
    char *trace_filename = NULL;
    int set_bit = -1, associtivity = -1, block_bit = -1;
    bool verbose = false; // TODO implement later!?

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
    cache_line *cache = (cache_line *)malloc(set_size * associtivity * sizeof(cache_line));

    /* TODO parse trace file (action and address)
        M = access 2
        L and S = access 1
        address - hex format, turn to binary
    */
    FILE *file = fopen(trace_filename, "r");
    if (!file) {
        printf("Error opening file"); // TODO what is this?
        return 1;
    }
    // TODO compute the length of the file and then malloc for access array
    // Or I can do operation inside the read
    char buffer[20];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != ' ') continue; // ignore "I" (without " ")

        char type;
        unsigned long address;
        int match = sscanf(buffer, " %c %lx,%*d\n", &type, &address);
        assert(match == 2);
        switch (type) {
            case 'M':
                access_cache(type, address, cache);
                access_cache(type, address, cache);
                break;
            case 'L':
            case 'S':
                access_cache(type, address, cache);
                break;
            default:
                // TODO error
                printf("Error type\n");
                break;
        }
    }
    
    printSummary(hit_count, miss_count, eviction_count);
    free(cache); // TODO when to free?
    return 0;
}
