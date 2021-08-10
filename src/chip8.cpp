#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <assert.h>
#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>

#include "interpreter.h"
#include "config.h"

void PrintUsage();
void DrawRectangleAt(sf::RenderWindow &window, const int native_x, const int native_y);
int StartGame(Chip8::Interpreter &game, sf::RenderWindow &window);
void DrawDispayData(sf::RenderWindow &window, const std::vector<std::vector<bool>> display);

int main(void)
{
    // Testing on the IBM logo for now
    const std::string rom_path = "../roms/ibm.ch8";

    Chip8::Interpreter game(config::display_width, config::display_height);

    try
    {
        game.ReadInstructionsFromDisk(rom_path);
    }
    catch (const std::ios_base::failure &e)
    {
        std::cerr << "ERR: Unable to read file \"" << rom_path << "\". Exiting." << std::endl;
        PrintUsage();

        return 1;
    }
    catch (...)
    {
        // DEBUG ONLY
        std::cerr << "exception in main()\n";
    }

    sf::RenderWindow window(
        sf::VideoMode(
            config::display_width * config::display_scale, 
            config::display_height * config::display_scale
        ),
        "CHIP-8: " + rom_path
    );

    return StartGame(game, window);
}

int StartGame(Chip8::Interpreter &game, sf::RenderWindow &window)
{
    while (window.isOpen() && game.StepOneCycle())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        if (game.ShouldUpdateDisplay())
        {
            window.clear();

            const std::vector<std::vector<bool>> display = game.GetDisplayData();
            DrawDispayData(window, display);
            
            window.display();

            game.SetDisplayUpdated();
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1429));
    }

    return 0;
}

void DrawDispayData(sf::RenderWindow &window, const std::vector<std::vector<bool>> display)
{
    for (int i = 0; i < config::display_height; ++i)
    {
        for (int j = 0; j < config::display_width; ++j)
        {
            if (display[i][j])
            {
                DrawRectangleAt(window, j, i);
            }
        }
    }
}

void DrawRectangleAt(sf::RenderWindow &window, const int native_x, const int native_y)
{
    int scaled_x = native_x * config::display_scale;
    int scaled_y = native_y * config::display_scale;

    sf::RectangleShape rectangle(sf::Vector2f(config::display_scale, config::display_scale));
    rectangle.setPosition(scaled_x, scaled_y);

    window.draw(rectangle);
}

void PrintUsage()
{
    std::cout << "Usage: chip8.exe" << std::endl;
}