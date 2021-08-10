#include <vector>

#ifndef CONFIG_H
#define CONFIG_H

namespace config
{
    // The size of the CHIP-8 memory, in bytes (default: 4KiB)
    extern int memory_size;

    // The starting address of the program instructions (default: 0x200)
    extern int program_start;

    extern int display_width;
    extern int display_height;
    extern int display_scale;

    extern std::vector<int> font;
    extern uint16_t font_start;

    extern int sprite_width;
}

#endif