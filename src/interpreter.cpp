#include <fstream>
#include <assert.h>
#include <iostream>

#include "interpreter.h"

// throws std::ios_base::failure
Chip8::Interpreter::Interpreter(int display_width, int display_height) : address_space_(config::memory_size, 0), display_(display_height)
{
    reg_PC_ = config::program_start;
    reg_I_ = 0;
    
    for (int i = 0; i < MAX_REGISTERS; ++i)
    {
        reg_general_[i] = 0;
    }

    display_width_ = display_width;
    display_height_ = display_height;
    display_modified_ = false;

    for (int i = 0; i < display_height; ++i)
    {
        for (int j = 0; j < display_width; ++j)
        {
            display_[i].push_back(false);
        }
    }

    timer_delay_ = 0;
    timer_sound_ = 0;

    program_end_ = 0;

    is_initialized_ = false;
}

Chip8::Interpreter::Interpreter(const std::string &filename, int display_width, int display_height) : Interpreter(display_width, display_height)
{
    ReadInstructionsFromDisk(filename);
    is_initialized_ = true;    
}

void Chip8::Interpreter::ReadInstructionsFromDisk(const std::string &filename)
{
    std::ifstream rom_src;
    rom_src.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    rom_src.open(filename, std::ios::in | std::ios::binary);

    int byte_count = 0;

    try
    {
        uint8_t *address_space = address_space_.data();

        while (rom_src.read(reinterpret_cast<char *>(address_space + config::program_start + byte_count++), 1));
    }
    catch (const std::ios_base::failure &e)
    {
        // ifstreams throw exceptions on EOF - check this exception is relevant
        // to caller before rethrowing
        if (!rom_src.eof())
        {
            throw;
        }
    }

    rom_src.close();

    program_end_ = config::program_start + byte_count - 1;
    is_initialized_ = true;
}

bool Chip8::Interpreter::StepOneCycle()
{
    // make sure there's an instruction to process
    if (!is_initialized_ || reg_PC_ + 1 > program_end_)
    {
        return false;
    }

    uint16_t raw_instruction = FetchNextInstruction();
    reg_PC_ += 2;

    const Chip8::Instruction instruction = DecodeInstruction(raw_instruction);

    return ExecuteInstruction(instruction);
}

inline uint16_t Chip8::Interpreter::FetchNextInstruction()
{
    assert(reg_PC_ + 1 <= program_end_);

    uint16_t instruction = address_space_[reg_PC_];
    instruction = (instruction << 8) | address_space_[reg_PC_ + 1];

    return instruction;
}

inline const Chip8::Instruction Chip8::Interpreter::DecodeInstruction(uint16_t &instruction)
{
    Instruction retval;

    retval.type = (instruction >> 12) & 0xF;
    retval.X = (instruction >> 8) & 0xF;
    retval.Y = (instruction >> 4) & 0xF;
    retval.N = instruction & 0xF;
    retval.NN = instruction & 0xFF;
    retval.NNN = instruction & 0xFFF;

    return retval;
}

bool Chip8::Interpreter::ExecuteInstruction(const Chip8::Instruction &instruction)
{
    bool success_status = true;

    switch (instruction.type)
    {
        case 0x0:
            switch (instruction.NNN)
            {
                case 0x0E0:
                    // 00E0: Clear screen
                    ClearDisplay();

                    break;
                case 0x0EE:
                    // 00EE: Return from subroutine
                    break;
                default:
                    // 0NNN: Execute machine language routine -- not supported
                    break;
            }

            break;

        case 0x1:
            // 1NNN: Jump
            reg_PC_ = instruction.NNN;

            break;

        case 0x2:
            // 2NNN: Call subroutine
            break;

        case 0x3:
            // 3XNN: Skip if VX == NN
            break;

        case 0x4:
            // 4XNN: Skip if VX != NN
            break;

        case 0x5:
            // 5XY0: Skip if VX == VY
            break;

        case 0x6:
            // 6XNN: Set
            reg_general_[instruction.X] = instruction.NN;

            break;
        
        case 0x7:
            // 7XNN: Add
            reg_general_[instruction.X] += instruction.NN;

            break;

        case 0x8:
            break;

        case 0x9:
            // 9XY0: Skip if VX != VY
            break;

        case 0xA:
            reg_I_ = instruction.NNN;

            break;
        
        case 0xB:
            break;

        case 0xC:
            break;

        case 0xD:
        {
            // DXYN: Display
            int sprite_height = instruction.N;
            int sprite_x = reg_general_[instruction.X];
            int sprite_y = reg_general_[instruction.Y];
            uint16_t sprite_location = reg_I_;

            DrawSprite(sprite_x, sprite_y, sprite_height, sprite_location);

            break;
        }
        case 0xE:
            break;
        
        case 0xF:
            break;

        default:
            std::cerr << "Warning: instruction 0x" << std::hex << instruction.type << instruction.NNN << " is not valid." << std::endl;
            success_status = false;
            break;
    }

    return success_status;
}

bool Chip8::Interpreter::ShouldUpdateDisplay()
{
    return display_modified_;
}

void Chip8::Interpreter::SetDisplayUpdated()
{
    display_modified_ = false;
}

const std::vector<std::vector<bool>> Chip8::Interpreter::GetDisplayData()
{
    return display_;
}

void Chip8::Interpreter::ClearDisplay()
{
    for (int i = 0; i < display_height_; ++i)
    {
        for (int j = 0; j < display_width_; ++j)
        {
            display_[i][j] = false;
        }
    }

    display_modified_ = true;
}

void Chip8::Interpreter::DrawSprite(int x, int y, int height, uint16_t location)
{
    reg_general_[0xF] = 0;

    x %= display_width_;
    y %= display_height_;

    uint16_t current_location = location;

    for (int i = 0; i < height; ++i)
    {
        int target_y = y + i;

        if (target_y >= display_height_)
        {
            break;
        }

        uint8_t sprite_row = address_space_[current_location];

        for (int j = 0; j < config::sprite_width; ++j)
        {
            int target_x = x + j;

            if (target_x >= display_width_)
            {
                break;
            }

            // read each bit of the sprite row, left to right
            uint8_t pixel = (sprite_row >> config::sprite_width - j - 1) & 0x1;

            if (pixel)
            {
                bool result = !display_[target_y][target_x];

                display_[target_y][target_x] = result;
                display_modified_ = true;

                if (!result)
                {
                    reg_general_[0xF] = 1;
                }
            }
        }

        current_location += 2;
    }
}