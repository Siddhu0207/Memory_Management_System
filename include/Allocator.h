#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <list>
#include <cstddef>
#include <iostream>
#include "Config.h"

enum Alloc_Strat{ FIRST_FIT, BEST_FIT, WORST_FIT };

struct Mem_Block {
    int id;             
    size_t st_addr;  
    size_t size;           
    size_t requs_size; 
    bool is_free;       

    Mem_Block(int _id, size_t _start, size_t _size, size_t _req_size, bool _free)
        : id(_id), st_addr(_start), size(_size), requs_size(_req_size), is_free(_free) {}
};

class Allocator {
private:
    std::list<Mem_Block> blk_lst;
    size_t tot_size;
    Alloc_Strat curr_strat;
    int next_alloc_id;

    void coalesce_free_blocks();
    size_t align(size_t size);

public:
    Allocator(size_t size);
    void setStrategy(Alloc_Strat strat);
    void malloc_mem(size_t raw_size);
    void free_mem(int id);
    bool is_valid_access(int vrt_addr);
    void print_stats();
};

#endif