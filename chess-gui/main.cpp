#include "MemoryLeakDetector.h"

#include "chess-simulator.h"
#include "chess.hpp"

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "extras/IconsFontAwesome6.h"

#include "PieceSvg.h"
#include <map>
#include <chrono>
#include "magic_enum/magic_enum.hpp"

enum class SimulationState {
    PAUSED,
    RUNNING,
};

// ui settings
auto simulationState = SimulationState::PAUSED;
std::chrono::nanoseconds timeSpentOnMoves = std::chrono::nanoseconds::zero();
std::chrono::nanoseconds timeSpentLastMove = std::chrono::milliseconds::zero();
int64_t accumulatedBytesLeaks = 0;
string gameResult;
vector<string> moves;

void move(chess::Board& board) {
    if(board.isHalfMoveDraw()){
        auto result = board.getHalfMoveDrawType();
        gameResult = std::string(magic_enum::enum_name(result.second)) + " " + std::string(magic_enum::enum_name(result.first));
        return;
    }
    auto result = board.isGameOver();
    gameResult = std::string(magic_enum::enum_name(result.second)) + " " + std::string(magic_enum::enum_name(result.first));
    if(result.second != chess::GameResult::NONE) {
        simulationState = SimulationState::PAUSED;
        return;
    }

    std::string turn(magic_enum::enum_name(board.sideToMove().internal()));

    auto beforeAllocCount = memory_stats::get()->allocCount;
    auto beforeAllocSize = memory_stats::get()->allocSize;
    auto beforeTime = std::chrono::high_resolution_clock::now();

    memory_stats::get()->enabled = true;
    auto moveStr = ChessSimulator::Move(board.getFen(true));
    memory_stats::get()->enabled = false;

    auto afterTime = std::chrono::high_resolution_clock::now();
    auto afterAllocSize = memory_stats::get()->allocSize;
    auto afterAllocCount = memory_stats::get()->allocCount;

    auto move = chess::uci::uciToMove(board, moveStr);

    if(afterAllocSize!=beforeAllocSize) {
        std::cerr << "Memory leak detected: " << afterAllocCount - beforeAllocCount << " allocations not freed; total: " << afterAllocSize - beforeAllocSize << std::endl;
    }
    board.makeMove(move);

    // update stats
    accumulatedBytesLeaks += afterAllocSize - beforeAllocSize;
    timeSpentOnMoves += afterTime - beforeTime;
    timeSpentLastMove = afterTime - beforeTime;
    moves.push_back(std::to_string(board.fullMoveNumber()) + " " + turn +": " + moveStr);
}

auto SvgStringToTexture(std::string svgString){
    auto document = lunasvg::Document::loadFromData(svgString);
    auto bitmap = document->renderToBitmap(100, 100);
    bitmap.convertToRGBA();
    auto img = GenImageColor(100,100, WHITE);
    img.data = bitmap.data();
    auto tex = LoadTextureFromImage(img);
    return tex;
}

auto loadPiecesTextures(){
    std::map<char, Texture> map;

    map['P'] = SvgStringToTexture(PawnWhiteSvgString);
    map['p'] = SvgStringToTexture(PawnBlackSvgString);
    map['R'] = SvgStringToTexture(RookWhiteSvgString);
    map['r'] = SvgStringToTexture(RookBlackSvgString);
    map['N'] = SvgStringToTexture(KnightWhiteSvgString);
    map['n'] = SvgStringToTexture(KnightBlackSvgString);
    map['B'] = SvgStringToTexture(BishopWhiteSvgString);
    map['b'] = SvgStringToTexture(BishopBlackSvgString);
    map['Q'] = SvgStringToTexture(QueenWhiteSvgString);
    map['q'] = SvgStringToTexture(QueenBlackSvgString);
    map['K'] = SvgStringToTexture(KingWhiteSvgString);
    map['k'] = SvgStringToTexture(KingBlackSvgString);

    return map;
}

int main(int argc, char* argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 800;
    int minScreenSide;
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Chess AI Simulator");
    SetTargetFPS(144);
    rlImGuiSetup(true);


    // load data
    auto piecesTextures = loadPiecesTextures();

    chess::Board board;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if(simulationState == SimulationState::RUNNING)
            move(board);

        BeginDrawing();
        ClearBackground(DARKGRAY);
        // game logic

        // draw the chess board
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        minScreenSide = screenWidth < screenHeight ? screenWidth : screenHeight;

        auto displacementX = screenWidth/2 - minScreenSide/2;
        auto displacementY = screenHeight/2 - minScreenSide/2;

        Rectangle pieceRectSource {0,0,100,100};
        Vector2 origin = {0,0};

        for(int row=0;row<8;row++){
            for(int col=0;col<8;col++){
                Rectangle location = {displacementX + col*minScreenSide/8.f, screenHeight - displacementY - (row+1)*minScreenSide/8.f, minScreenSide/8.f, minScreenSide/8.f};

                if((row+col)%2==0)
                    DrawRectangleRec(location, WHITE);
                else
                    DrawRectangleRec(location, LIGHTGRAY);

                switch(board.at(chess::Square(row*8+col)).internal()){
                    case chess::Piece::underlying::WHITEPAWN:
                        DrawTexturePro(piecesTextures['P'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::BLACKPAWN:
                        DrawTexturePro(piecesTextures['p'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::WHITEROOK:
                        DrawTexturePro(piecesTextures['R'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::BLACKROOK:
                        DrawTexturePro(piecesTextures['r'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::WHITEKNIGHT:
                        DrawTexturePro(piecesTextures['N'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::BLACKKNIGHT:
                        DrawTexturePro(piecesTextures['n'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::WHITEBISHOP:
                        DrawTexturePro(piecesTextures['B'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::BLACKBISHOP:
                        DrawTexturePro(piecesTextures['b'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::WHITEQUEEN:
                        DrawTexturePro(piecesTextures['Q'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::BLACKQUEEN:
                        DrawTexturePro(piecesTextures['q'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::WHITEKING:
                        DrawTexturePro(piecesTextures['K'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    case chess::Piece::underlying::BLACKKING:
                        DrawTexturePro(piecesTextures['k'], pieceRectSource, location, origin, 0, WHITE);
                        break;
                    default:
                        break;
                }
            }
        }

        // start ImGui Conent
        rlImGuiBegin();

        ImGui::Begin("Settings", nullptr);
        ImGui::Text("%.1fms %.0fFPS | AVG: %.2fms %.1fFPS", ImGui::GetIO().DeltaTime * 1000, 1.0f / ImGui::GetIO().DeltaTime,
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Separator();
        if (ImGui::Button(ICON_FA_PLAY)) {
            simulationState = SimulationState::RUNNING;
        };
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PAUSE)) {
            simulationState = SimulationState::PAUSED;
        };
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLAY ICON_FA_PAUSE)){
            simulationState = SimulationState::PAUSED;
            move(board);
        }
        ImGui::Separator();
        // statistics
        ImGui::Text("Acc Time spent: %.2fms", timeSpentOnMoves.count() / 1000000.0);
        ImGui::Text("Last move dur:  %.2fms", timeSpentLastMove.count() / 1000000.0);
        ImGui::Text("Bytes leaked: %lld", accumulatedBytesLeaks);
        ImGui::Text("Game result: %s", gameResult.c_str());
        // moves
        ImGui::Separator();
        ImGui::BeginChild("Moves", ImVec2(0, 0), true);
        // print moves in reverse order
        for(auto it = moves.rbegin(); it != moves.rend(); ++it){
            ImGui::Text("%s", it->c_str());
        }
        ImGui::EndChild();

        ImGui::End();  // end settings
        // end ImGui Content
        rlImGuiEnd();

        // end game logic

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    rlImGuiShutdown();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}