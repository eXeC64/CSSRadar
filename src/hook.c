#include <stdio.h>
#include <sys/ptrace.h>
#include <stdint.h>

#include "hook.h"

uint32_t find_dylib(uint32_t pid, const char* dylib, bool find_end) {
    char* cmd[256];
    if(!find_end) {
        snprintf(cmd, 256, "grep \"%s\" /proc/%i/maps | head -n 1 | cut -d \"-\" -f1", dylib, pid);
    } else {
        snprintf(cmd, 256, "grep \"%s\" /proc/%i/maps | tail -n 1 | cut -d \"-\" -f2", dylib, pid);
    }

    FILE* maps = popen(cmd, "r");

    uint32_t ptr = 0;

    if(maps) {
        fscanf(maps, "%08lx", &ptr);
    }

    pclose(maps);

    return ptr;
}

void mem_read(uint32_t pid, uint32_t remote_addr, uint32_t* buf, size_t buf_size) {

    if(buf_size % 4) {
        return;
    }

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    waitpid(pid, NULL, 0);

    for(int i = 0; i < buf_size/4; ++i) {
        buf[i] = ptrace(PTRACE_PEEKDATA, pid, remote_addr + (4*i), NULL);
    }

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

bool data_compare(uint8_t* data, uint8_t* byte_mask, char* str_mask) {
    for(; *str_mask; ++str_mask, ++data, ++byte_mask) {
        if(str_mask == 'x' && *data != *byte_mask) {
            return false;
        }
    }
    return (str_mask == NULL);
}

uint32_t find_pattern(uint32_t address, uint32_t len, uint8_t* byte_mask, char* str_mask) {
    for(uint32_t i = 0; i < len; ++i) {
        if(data_compare((uint8_t*)(address+i), byte_mask, str_mask)) {
            return (uint32_t)(address+i);
        }
    }
    return 0;
}
