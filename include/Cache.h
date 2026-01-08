#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>

struct CacheL {
    bool valid;
    unsigned int tag;
    int lst_acs_time;
    CacheL() : valid(false), tag(0), lst_acs_time(0) {}
};

class CacheLvl {
private:
    std::string name;
    int tot_size, blk_size, associativity, num_set;
    std::vector<std::vector<CacheL>> sets;
    int hit, miss;

public:
    CacheLvl(std::string _name, int _size, int _blk_Size, int _assoc);
    void reset();
    bool access(int phy_addr, int curr_time);
    void install(int phy_addr, int curr_time);
    void flush_frame(int fr_st_addr, int fr_size);
    void print_stats();
};

#endif