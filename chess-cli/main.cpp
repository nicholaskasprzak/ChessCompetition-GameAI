#include "chess-simulator.h"
#include "chess.hpp"
#include <string>

int main() {
    std::string fen;
    getline(std::cin, fen);
    auto move = ChessSimulator::Move(fen);
    std::cout << move << std::endl;
}