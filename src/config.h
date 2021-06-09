#ifndef CONFIG_H
#define CONFIG_H

namespace config
{
    // the size of the CHIP-8 memory, in bytes (default: 4KiB)
    extern int memory_size;

    // the starting address of the program instructions (default: 0x200)
    extern int program_start;
}

#endif