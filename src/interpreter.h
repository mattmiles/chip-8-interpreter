#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <cstdint>
#include <array>
#include <vector>
#include <stack>
#include <string>

#include "config.h"

namespace Chip8
{
    constexpr int MAX_REGISTERS = 16;

    struct Instruction
    {
        int type;
        int X;
        int Y;
        int N;
        int NN;
        int NNN;
    };

    // Main class for the CHIP-8 interpreter. 
    class Interpreter
    {
    public:
        Interpreter(int display_width, int display_height);
        Interpreter(const std::string &filename, int display_width, int display_height);
        
        bool StepOneCycle();

        void ReadInstructionsFromDisk(const std::string &filename);

        bool ShouldUpdateDisplay();
        const std::vector<std::vector<bool>> GetDisplayData();
        void SetDisplayUpdated();

    private:
        // CHIP-8 memory
        std::vector<uint8_t> address_space_;

        // Registers & program counter
        uint16_t reg_PC_;
        uint16_t reg_I_;
        std::array<uint8_t, MAX_REGISTERS> reg_general_;

        // Stack
        std::stack<uint16_t> stack_;

        // Timers
        uint8_t timer_delay_;
        uint8_t timer_sound_;

        // display
        std::vector<std::vector<bool>> display_;

        // address of last instruction
        uint16_t program_end_;

        bool is_initialized_;

        int display_width_;
        int display_height_;
        bool display_modified_;

        inline uint16_t FetchNextInstruction();
        inline const Instruction DecodeInstruction(uint16_t &instruction);
        bool ExecuteInstruction(const Instruction &instruction);

        // Display/graphics
        void ClearDisplay();
        void DrawSprite(int x, int y, int height, uint16_t location);
    };
}

#endif