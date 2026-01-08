#ifndef CONFIG_H
#define CONFIG_H

// Memory Configuration
const int PAGE_SIZE = 64; 
const int ALIGNMENT = 4;

// Cache Configuration
const int L1_SIZE = 256;      
const int L1_BLOCK_SIZE = 16; 
const int L1_ASSOC = 2;       

const int L2_SIZE = 512;      
const int L2_BLOCK_SIZE = 16; 
const int L2_ASSOC = 4;       

#endif