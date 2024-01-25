#include "chess-simulator.h"
#include "chess.hpp"

using namespace ChessSimulator;

std::string ChessSimulator::Move(std::string fen){
    // create your board based on the board string following the FEN notation
    // search for the best move using minimax / monte carlo tree search / alpha-beta pruning / ...
    // try to use nice heuristics to speed up the search and have better results
    // return the best move in UCI notation
    // you will gain extra points if you create your own board/move representation instead of using the one provided by the library

    // here goes a random movement
    chess::Board board(fen);
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // get random move
    auto move = moves[std::rand() % moves.size()];
    return chess::uci::moveToUci(move);
}