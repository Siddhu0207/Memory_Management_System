**MEMORY MANAGEMENT SYSTEM**

**1\. Memory Layout and Assumptions**

The system simulates a single-process environment with a configurable amount of Physical RAM. It manages memory through a combination of a contiguous physical byte array and a logical block list.

* **Physical Memory (RAM):**  
  * Represented as a linear vector of bytes (`std::vector<uint8_t>`).  
  * **Page Size:** Fixed at **64 Bytes**.  
  * **Alignment:** All memory allocations are aligned to **4 Bytes** boundaries.  
* **Logical Memory (Heap):**  
  * Managed as a linked list (`std::list<MemoryBlock>`) where each node represents a contiguous chunk of virtual address space.  
  * Each block tracks its `start_addr`, `size`, `requested_size` (for internal fragmentation tracking), and `is_free` status.  
* **Assumptions:**  
  * **Addressing:** The simulator uses `int` and `size_t` for addressing.  
  * **Data Storage:** While `physical_memory` exists as a vector, the simulation primarily tracks *state* (validity, tags, allocation status) rather than actual data values.  
  * **Global Time:** A logical clock (`global_time`) is used to manage LRU replacement policies in the cache.

## **2\. Allocation Strategy Implementations**

The allocator manages the heap using an **Explicit Free List** maintained as a linked list of memory blocks. The system supports three dynamic allocation strategies, switchable at runtime.

### **Strategies**

1. **First Fit:**  
   * Iterates through the block list from the beginning.  
   * Selects the **first** free block where `block.size >= aligned_request_size`.  
   * *Implementation:* Linear scan (`O(N)`).  
2. **Best Fit:**  
   * Iterates through the entire list.  
   * Selects the free block that produces the **smallest** remaining fragment (min `block.size - aligned_request_size`).  
   * *Goal:* Minimize wasted space in the specific block.  
3. **Worst Fit:**  
   * Iterates through the entire list.  
   * Selects the free block that produces the **largest** remaining fragment.  
   * *Goal:* Leave large enough gaps for future allocations.

### **Internal Mechanisms**

* **Splitting:** If a selected free block is larger than the requested size, it is split into two:  
  * The allocated block (Used).  
  * A new free block containing the remaining space (inserted immediately after the used block).  
* **Coalescing (Defragmentation):**  
  * Triggered immediately upon `free_mem()`.  
  * The allocator scans the list and merges adjacent blocks if **both** are marked `is_free`.  
  * This reduces external fragmentation by combining small free chunks into larger usable blocks.

## **3\. Cache Hierarchy and Replacement Policy**

The system implements a two-level inclusive cache hierarchy. Both levels use an **LRU (Least Recently Used)**replacement policy.

### **L1 Cache Specification**

* **Total Size:** 256 Bytes.  
* **Block (Line) Size:** 16 Bytes.  
* **Associativity:** **2-Way Set Associative**.  
* **Sets:** Calculated as 256/(16×2)=8 Sets.

### **L2 Cache Specification**

* **Total Size:** 512 Bytes.  
* **Block (Line) Size:** 16 Bytes.  
* **Associativity:** **4-Way Set Associative**.  
* **Sets:** Calculated as 512/(16×4)=8 Sets.

### **Replacement & Coherency**

* **LRU Policy:**  
  * Every cache line stores a `last_access_time`.  
  * On a set conflict (Set is full), the line with the smallest `last_access_time` is evicted.  
* **Address Mapping:**  
  * **Index:** Extracted using `(Addr / BlockSize) % NumSets`.  
  * **Tag:** Extracted using `Addr / (BlockSize * NumSets)`.  
* **Cache Flushing:**  
  * When the MMU evicts a physical page (Page Fault), `flush_frame` is called.  
  * This iterates through all cache sets and invalidates any lines belonging to the evicted physical frame to ensure data consistency.

## **4\. Virtual Memory Model**

The Virtual Memory model maps Virtual Page Numbers (VPN) to Physical Frame Numbers (PFN) using a single-level paging scheme.

* **Page Table:**  
  * Implemented as a `std::map<int, PageTableEntry>`.  
  * Maps **VPN** → **Frame Number**.  
  * Tracks `valid` bit to indicate if the page is currently in physical RAM.  
* **Frame Management:**  
  * `physical_frames`: A vector tracking which VPN currently occupies which physical frame index.  
* **Page Replacement Policy:** **FIFO (First-In, First-Out)**.  
  * A `std::deque<int>` (`frame_fifo_queue`) maintains the order of frame allocation.  
  * When a Page Fault occurs and memory is full, the frame at the front of the queue is victimized (evicted)

## **5\. Address Translation Flow**

When the CPU requests access to a Virtual Address (VA), the following sequence occurs in `access_virtual_address`:

1. **Segmentation Check:**  
   * The system first verifies if the VA belongs to a valid allocated `MemoryBlock`.  
   * If the address is unmapped or belongs to a free block → **Segmentation Fault**.  
2. **Address Decomposition:**  
   * VPN=VA/64  
   * Offset=VA(mod64)  
3. **MMU Lookup:**  
   * Check `page_table` for the VPN.  
   * **Hit:** Retrieve PFN immediately.  
   * **Miss (Page Fault):**  
     * Select victim frame using FIFO.  
     * **Flush Caches:** Invalidate L1/L2 lines corresponding to the victim frame.  
     * Update Page Table and Frame Queue.  
     * Return new PFN.  
4. **Physical Address Generation:**  
   * PA=(PFN×64)+Offset  
5. **Cache Access:**  
   * **L1 Lookup:** Check L1 with PA. If Hit, return.  
   * **L2 Lookup:** If L1 Miss, check L2.  
     * If L2 Hit: Promote to L1.  
   * **Main Memory:** If L2 Miss, fetch from RAM, install in L2, then install in L1.

## **6\. Limitations and Simplifications**

1. **Simulation Scale:** The defined sizes (256B L1, 64B Page) are extremely small compared to real hardware to make the simulation output readable.  
2. **Instantaneous Memory:** RAM latency is simulated only by log messages; there is no actual cycle-accurate delay penalty logic.  
3. **Unified Cache:** The simulator treats all accesses as generic data accesses; it does not distinguish between Instruction Cache (I-Cache) and Data Cache (D-Cache).  
4. **No "Real" TLB:** The code logs "TLB Hit," but implementation-wise, it accesses the `page_table` map directly. There is no separate small fixed-size TLB structure separate from the main page table.  
5. **Simplified Page Faults:** The simulation assumes "Infinite backing store" (disk). When a page is evicted, it simply vanishes; when loaded, it is assumed to be ready instantly.

