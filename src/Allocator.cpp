#include "../include/Allocator.h"
#include <limits>
#include <iomanip>

Allocator::Allocator(size_t size) 
    : tot_size(size), curr_strat(FIRST_FIT), next_alloc_id(1) {
    // Initialize memory as one giant free block
    blk_lst.emplace_back(0, 0, size, 0, true);
}

size_t Allocator::align(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

void Allocator::coalesce_free_blocks() {
    if (blk_lst.empty()) return;
    auto it = blk_lst.begin();
    while (it != blk_lst.end()) {
        auto next_it = std::next(it);
        if (next_it == blk_lst.end()) break;

        if (it->is_free && next_it->is_free) {
            it->size += next_it->size;
            blk_lst.erase(next_it);
            // Do not increment 'it' so we can check the new merged block against the next neighbor
        } else {
            ++it;
        }
    }
}

void Allocator::setStrategy(Alloc_Strat strategy) {
    curr_strat = strategy;
    std::cout << "Allocator strategy updated.\n";
}

void Allocator::malloc_mem(size_t raw_size) {
    size_t aligned_size = align(raw_size);
    auto target_block = blk_lst.end();

    // Strategy Selection Logic
    if (curr_strat == FIRST_FIT) {
        for (auto it = blk_lst.begin(); it != blk_lst.end(); ++it) {
            if (it->is_free && it->size >= aligned_size) {
                target_block = it;
                break;
            }
        }
    } else if (curr_strat == BEST_FIT) {
        size_t min_diff = std::numeric_limits<size_t>::max();
        for (auto it = blk_lst.begin(); it != blk_lst.end(); ++it) {
            if (it->is_free && it->size >= aligned_size) {
                size_t diff = it->size - aligned_size;
                if (diff < min_diff) {
                    min_diff = diff;
                    target_block = it;
                }
            }
        }
    } else if (curr_strat == WORST_FIT) {
        size_t max_diff = 0;
        bool found = false;
        for (auto it = blk_lst.begin(); it != blk_lst.end(); ++it) {
            if (it->is_free && it->size >= aligned_size) {
                size_t diff = it->size - aligned_size;
                if (diff >= max_diff || !found) {
                    max_diff = diff;
                    target_block = it;
                    found = true;
                }
            }
        }
    }

    if (target_block == blk_lst.end()) {
        std::cout << "Allocation Failed: Not enough memory.\n";
        return;
    }

    // Splitting Logic
    if (target_block->size > aligned_size) {
        size_t remaining = target_block->size - aligned_size;
        size_t new_start = target_block->st_addr + aligned_size;
        
        target_block->size = aligned_size;
        target_block->requs_size = raw_size;
        target_block->is_free = false;
        target_block->id = next_alloc_id;
        
        blk_lst.insert(std::next(target_block), Mem_Block(0, new_start, remaining, 0, true));
    } else {
        target_block->is_free = false;
        target_block->requs_size = raw_size;
        target_block->id = next_alloc_id;
    }
    
    std::cout << "Allocated Block ID " << next_alloc_id++ << " at 0x" << std::hex << target_block->st_addr << std::dec << "\n";
}

void Allocator::free_mem(int id) {
    bool found = false;
    for (auto& block : blk_lst) {
        if (!block.is_free && block.id == id) {
            block.is_free = true;
            block.requs_size = 0;
            found = true;
            break;
        }
    }
    if (found) {
        std::cout << "Freed Block ID " << id << ".\n";
        coalesce_free_blocks();
    } else {
        std::cout << "Block ID " << id << " not found.\n";
    }
}

bool Allocator::is_valid_access(int virtual_addr) {
    for (const auto& block : blk_lst) {
        if (virtual_addr >= (int)block.st_addr && virtual_addr < (int)(block.st_addr + block.size)) {
            return !block.is_free;
        }
    }
    return false;
}

void Allocator::print_stats() {
    size_t used = 0, free_mem = 0, internal_frag = 0, max_free = 0;
    for(const auto& b : blk_lst) {
        if(b.is_free) {
            free_mem += b.size;
            if(b.size > max_free) max_free = b.size;
        } else {
            used += b.size;
            if(b.requs_size > 0) internal_frag += (b.size - b.requs_size);
        }
    }
    double ext_frag = (free_mem > 0) ? (1.0 - ((double)max_free / free_mem)) * 100.0 : 0.0;
    
    std::cout << "--- Allocator Stats (Total: " << tot_size << ") ---\n";
    std::cout << "Used: " << used << " | Free: " << free_mem << "\n";
    std::cout << "Internal Frag: " << internal_frag << "B | External Frag: " << std::fixed << std::setprecision(2) << ext_frag << "%\n";
}