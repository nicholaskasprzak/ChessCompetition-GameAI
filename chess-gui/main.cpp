#include "MemoryLeakDetector.h"

#include "chess-simulator.h"
#include "chess.hpp"

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "extras/IconsFontAwesome6.h"

#include "PieceSvg.h"
#include <map>

void move(chess::Board& board) {
    auto moveStr = ChessSimulator::Move(board.getFen(true));
    auto move = chess::uci::uciToMove(board, moveStr);
    board.makeMove(move);
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

enum class SimulationState {
    PAUSED,
    RUNNING,
};

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

    // ui settings
    bool aiEnabled = false;
    auto aiColor = chess::Color("w");
    auto simulationState = SimulationState::PAUSED;

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

        ImGui::End();  // end settings

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

        // end game logic

        // end ImGui Content
        rlImGuiEnd();

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