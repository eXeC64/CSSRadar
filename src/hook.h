#pragma once

#include <stdint.h>
#include <stdbool.h>


#define TEAM_NONE 0
#define TEAM_SPECTATE 1
#define TEAM_T 2
#define TEAM_CT 3

typedef struct player_s {
    uint8_t     padding1[40];
    uint32_t    index;
    uint8_t     padding2[12];
    uint8_t     name[32];
    uint32_t    team;
    uint32_t    health;
    float       x, y, z;
    float       pitch, yaw;
    uint8_t     padding3[204];
} player_t;


uint32_t find_dylib(uint32_t pid, const char* dylib, bool find_end);
void mem_read(uint32_t pid, uint32_t remote_addr, uint32_t* buf, size_t buf_size);
uint32_t find_pattern(uint32_t address, uint32_t len, uint8_t* byte_mask, char* str_mask);
