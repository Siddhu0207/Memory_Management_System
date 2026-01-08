#include "../include/VirtualMemory.h"
#include <iostream>

Vrt_Mem::Vrt_Mem(size_t total_ram_size) : pg_faults(0), pg_hits(0) {
    num_frames = total_ram_size / PAGE_SIZE;
    phy_frames.assign(num_frames, -1);
}

int Vrt_Mem::translate(int virtual_addr, CacheLvl& l1, CacheLvl& l2) {
    int vpn = virtual_addr / PAGE_SIZE;
    int offset = virtual_addr % PAGE_SIZE;

    // Check Page Table (TLB simulation)
    if (pg_tab.find(vpn) != pg_tab.end() && pg_tab[vpn].valid) {
        pg_hits++;
        std::cout << "  [MMU] TLB Hit.\n";
        return (pg_tab[vpn].f_num * PAGE_SIZE) + offset;
    }

    // Page Fault Handling
    pg_faults++;
    std::cout << "  [MMU] Page Fault!\n";
    // --- ADD THIS BLOCK FOR LATENCY SIMULATION ---
    std::cout << "  [Disk] Reading page from disk... (Simulated 10ms delay)\n";
    // Optional: #include <thread> and uncomment below for real delay
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // ---------------------------------------------
    int target_frame = -1;
    // Find free frame
    for(int i=0; i<num_frames; ++i) { if(phy_frames[i] == -1) { target_frame = i; break; } }

    // FIFO Replacement if full
    if (target_frame == -1) {
        if (fr_fifo_que.empty()) return -1; 
        target_frame = fr_fifo_que.front();
        fr_fifo_que.pop_front();
        
        int victim_vpn = phy_frames[target_frame];
        if (victim_vpn != -1) pg_tab[victim_vpn].valid = false;

        // Flush caches for the evicted frame to ensure coherency
        int frame_addr = target_frame * PAGE_SIZE;
        std::cout << "  [OS] Flushing Caches for Frame " << target_frame << "\n";
        l1.flush_frame(frame_addr, PAGE_SIZE);
        l2.flush_frame(frame_addr, PAGE_SIZE);
    }

    phy_frames[target_frame] = vpn;
    fr_fifo_que.push_back(target_frame);

    PT_Entry pte;
    pte.valid = true;
    pte.f_num = target_frame;
    pg_tab[vpn] = pte;

    return (target_frame * PAGE_SIZE) + offset;
}

void Vrt_Mem::print_stats() {
    std::cout << "--- VM Stats ---\n";
    std::cout << "Page Faults: " << pg_faults << " | Page Hits: " << pg_hits << "\n";
}