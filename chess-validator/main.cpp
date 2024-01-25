#include <iostream>
#include <sstream>
#include "chess.hpp"
#include "nlohmann/json.hpp"
#include "magic_enum/magic_enum.hpp"

using json = nlohmann::json;

// reads UCI moves string from stdin until the end of the game
// prints after every move all the states of the board
int main() {
    // read the board
    std::string fen;
    getline(std::cin, fen);

    // create the board
    chess::Board board(fen);

    for(;;) {
        json j = json::object();
        std::pair<chess::GameResultReason, chess::GameResult> gameover;

        // read the move
        std::string uci;
        getline(std::cin, uci);

        // create the move
        auto move = chess::uci::uciToMove(board, uci);
        j["isCapture"] = board.isCapture(move);

        // make the move
        board.makeMove(move);

        // print the board
        fen = board.getFen(true);
        j["fen"] = board.getFen(true);
        j["inCheck"] = board.inCheck();
        j["sideToMove"] = magic_enum::enum_name(board.sideToMove().internal());
        j["halfMoveClock"] = board.halfMoveClock();
        j["fullMoveNumber"] = board.fullMoveNumber();
        j["isRepetition"] = board.isRepetition();

        // print whole board to easily render on frontend
        std::stringstream ss;
        ss << board;
        j["boardRendered"] = ss.str();

        // game over condition
        auto isHalfMoveDraw = board.isHalfMoveDraw();
        j["isHalfMoveDraw"] = isHalfMoveDraw;
        if(isHalfMoveDraw){
            gameover = board.getHalfMoveDrawType();
            j["gameResult"] = magic_enum::enum_name(gameover.second);
            j["gameResultReason"] = magic_enum::enum_name(gameover.first);
            if(gameover.second != chess::GameResult::NONE)
                break;
        }
        else
        {
            gameover = board.isGameOver();
            j["gameResult"] = magic_enum::enum_name(gameover.second);
            j["gameResultReason"] = magic_enum::enum_name(gameover.first);
            if(gameover.second != chess::GameResult::NONE)
                break;
        }

        std::cout << j.dump() << std::endl;
    }
}