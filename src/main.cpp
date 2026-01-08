#include <iostream>
#include <string>
#include "../include/Allocator.h"
#include "../include/Cache.h"
#include "../include/VirtualMemory.h"
#include "../include/Config.h"

// System Integration Class
class MemorySimulator {
    Allocator* allocator;
    Vrt_Mem* vm;
    CacheLvl l1;
    CacheLvl l2;
    int global_time;

public:
    MemorySimulator(size_t size) 
        : l1("L1", L1_SIZE, L1_BLOCK_SIZE, L1_ASSOC),
          l2("L2", L2_SIZE, L2_BLOCK_SIZE, L2_ASSOC),
          global_time(0) {
        allocator = new Allocator(size);
        vm = new Vrt_Mem(size);
    }
    
    ~MemorySimulator() { delete allocator; delete vm; }

    void setStrategy(Alloc_Strat s) { allocator->setStrategy(s); }
    void malloc(size_t size) { allocator->malloc_mem(size); }
    void free(int id) { allocator->free_mem(id); }

    void access(int va) {
        // 1. Segmentation Check (Protection)
        if (!allocator->is_valid_access(va)) {
            std::cout << "[CPU] SegFault: Invalid Access at 0x" << std::hex << va << std::dec << "\n";
            return;
        }
        global_time++;

        std::cout << "[CPU] Access VA: " << va << "\n";

        // 2. Address Translation (VM handles faults & cache flushing)
        int pa = vm->translate(va, l1, l2);

        std::cout << "[TRACER] VA:" << va << " -> PA:" << pa << "\n";

        
        // 3. Cache Access Hierarchy
        std::cout << "  [Cache] Checking L1...\n";
        if (l1.access(pa, global_time)) {
            std::cout << "    -> L1 Hit\n";
        } else {
            std::cout << "    -> L1 Miss. Checking L2...\n";
            if (l2.access(pa, global_time)) {
                std::cout << "    -> L2 Hit. Fill L1.\n";
                l1.install(pa, global_time);
            } else {
                std::cout << "    -> L2 Miss. Fetch RAM. Fill L2, L1.\n";
                l2.install(pa, global_time);
                l1.install(pa, global_time);
            }
        }
    }

    void stats() {
        allocator->print_stats();
        vm->print_stats();
        l1.print_stats();
        l2.print_stats();
    }
};

int main() {
    size_t mem_size = 0;
    std::cout << "Enter Init Memory Size: ";
    std::cin >> mem_size;
    
    MemorySimulator sim(mem_size);
    std::string cmd;
    
    std::cout << "Commands: malloc <size>, free <id>, access <addr>, stats, set allocator <mode>, exit\n";

    while(std::cin >> cmd && cmd != "exit") {
        if (cmd == "malloc") { size_t s; std::cin >> s; sim.malloc(s); }
        else if (cmd == "free") { int id; std::cin >> id; sim.free(id); }
        else if (cmd == "access") { int a; std::cin >> a; sim.access(a); }
        else if (cmd == "stats") { sim.stats(); }
        else if (cmd == "set") {
             std::string _, mode; std::cin >> _ >> mode; 
             if(mode == "first") sim.setStrategy(FIRST_FIT);
             else if(mode == "best") sim.setStrategy(BEST_FIT);
             else if(mode == "worst") sim.setStrategy(WORST_FIT);
        }
        std::cout << "> ";
    }
    return 0;
}