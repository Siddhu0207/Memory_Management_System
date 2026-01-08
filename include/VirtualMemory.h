#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <map>
#include <vector>
#include <deque>
#include "Cache.h"
#include "Config.h"

struct PT_Entry {
    bool valid;
    int f_num;
    PT_Entry() : valid(false), f_num(-1) {}
};

class Vrt_Mem {
private:
    std::map<int, PT_Entry> pg_tab;
    std::deque<int> fr_fifo_que;
    std::vector<int> phy_frames; // Maps Frame -> VPN
    int num_frames;
    int pg_faults;
    int pg_hits;

public:
    Vrt_Mem(size_t tot_ram_size);
    // Returns Physical Address, triggers cache flush on eviction
    int translate(int vrt_addr, CacheLvl& l1, CacheLvl& l2);
    void print_stats();
};

#endif