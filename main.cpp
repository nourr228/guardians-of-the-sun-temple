#include <exception>
#include <iostream>

#include "core/Game.h"

int main() {
    try {
        Game game;
        return game.run();
    } catch (const std::exception& error) {
        std::cerr << "Application error: " << error.what() << '\n';
        return 1;
    }
}
