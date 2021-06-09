#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <assert.h>

#include "include/config.h"

void PrintUsage();
void ReadInstructionsFromDisk(std::ifstream &rom_src, std::vector<uint8_t> &address_space);

int main(void)
{
    // Initialize internal data structures
    std::vector<uint8_t> address_space(config::memory_size, 0);

    // Testing on the IBM logo for now
    const std::string rom_path = "roms/ibm.ch8";

    // Open the ROM file
    std::ifstream rom_src;
    rom_src.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        rom_src.open(rom_path, std::ios::in | std::ios::binary);

        ReadInstructionsFromDisk(rom_src, address_space);

        rom_src.close();
    }
    catch(std::ios_base::failure &e)
    {
        std::cerr << "ERR: Unable to read file \"" << rom_path << "\". Exiting." << std::endl;
        PrintUsage();

        return -1;
    }

    return 0;
}

// Reads the program instructions byte-by-byte from disk and writes them to the CHIP-8 address space.
void ReadInstructionsFromDisk(std::ifstream &rom_src, std::vector<uint8_t> &address_space)
{
    uint8_t byte_buffer = 0;
    rom_src.read(reinterpret_cast<char *>(&byte_buffer), 1);

    int byte_count = 0;

    try
    {
        // Read and write each byte of the instruction code into program memory
        do
        {
            int byte_to_write = config::program_start + byte_count++;
            assert(byte_to_write < config::memory_size && byte_to_write >= 0);

            address_space[byte_to_write] = byte_buffer;
        } 
        while (rom_src.read(reinterpret_cast<char *>(&byte_buffer), 1));
    }
    catch (std::ios_base::failure&)
    {
        // ifstreams throw exceptions on EOF - check this exception is relevant
        // to caller before allowing it to propagate
        if (!rom_src.eof())
        {
            throw;
        }
    }
}

void PrintUsage()
{
    std::cout << "Usage: chip8.exe" << std::endl;
}