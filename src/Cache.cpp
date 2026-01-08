#include "../include/Cache.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <iomanip>

CacheLvl::CacheLvl(std::string _name, int _size, int _blockSize, int _assoc)
    : name(_name), tot_size(_size), blk_size(_blockSize), associativity(_assoc), hit(0), miss(0) {
    num_set = tot_size / (blk_size * associativity);
    sets.resize(num_set, std::vector<CacheL>(associativity));
}

void CacheLvl::reset() {
    hit = 0; miss = 0;
    for(auto& s : sets) for(auto& l : s) l = CacheL();
}

bool CacheLvl::access(int physical_addr, int current_time) {
    int offset_bits = std::log2(blk_size);
    int set_index = (physical_addr >> offset_bits) & (num_set - 1);
    unsigned int tag = physical_addr >> (offset_bits + (int)std::log2(num_set));

    for (auto& line : sets[set_index]) {
        if (line.valid && line.tag == tag) {
            line.lst_acs_time = current_time;
            hit++;
            return true;
        }
    }
    miss++;
    return false;
}

void CacheLvl::install(int physical_addr, int current_time) {
    int offset_bits = std::log2(blk_size);
    int set_index = (physical_addr >> offset_bits) & (num_set - 1);
    unsigned int tag = physical_addr >> (offset_bits + (int)std::log2(num_set));

    // 1. Try to find empty slot
    for (auto& line : sets[set_index]) {
        if (!line.valid) {
            line.valid = true; line.tag = tag; line.lst_acs_time = current_time;
            return;
        }
    }
    // 2. LRU Eviction
    int lru_idx = 0, min_t = std::numeric_limits<int>::max();
    for (int i = 0; i < associativity; ++i) {
        if (sets[set_index][i].lst_acs_time < min_t) {
            min_t = sets[set_index][i].lst_acs_time;
            lru_idx = i;
        }
    }
    sets[set_index][lru_idx].valid = true;
    sets[set_index][lru_idx].tag = tag;
    sets[set_index][lru_idx].lst_acs_time = current_time;
}

void CacheLvl::flush_frame(int frame_start_addr, int frame_size) {
    for(int offset = 0; offset < frame_size; offset += blk_size) {
        int addr = frame_start_addr + offset;
        int set_index = (addr >> (int)std::log2(blk_size)) & (num_set - 1);
        unsigned int tag = addr >> ((int)std::log2(blk_size) + (int)std::log2(num_set));
        
        for (auto& line : sets[set_index]) {
            if (line.valid && line.tag == tag) line.valid = false;
        }
    }
}

void CacheLvl::print_stats() {
    std::cout << name << " Hits: " << hit << " Misses: " << miss;
    if (hit + miss > 0) 
        std::cout << " Rate: " << std::fixed << std::setprecision(2) << ((double)hit/(hit+miss)*100.0) << "%";
    std::cout << "\n";
}