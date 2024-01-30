#include "MemoryLeakDetector.h"

#include "chess-simulator.h"
#include "chess.hpp"

//#include "IconsFontAwesome6.h"
#include "IconsFontAwesome4.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"

#include "PieceSvg.h"
#include "magic_enum/magic_enum.hpp"
#include <chrono>
#include <map>

enum class SimulationState {
  PAUSED,
  RUNNING,
};

// ui settings
auto simulationState = SimulationState::PAUSED;
std::chrono::nanoseconds timeSpentOnMoves = std::chrono::nanoseconds::zero();
std::chrono::nanoseconds timeSpentLastMove = std::chrono::milliseconds::zero();
int64_t allocBalance = 0;
string gameResult;
vector<string> moves;

void reset(chess::Board &board){
  board = chess::Board();
  simulationState = SimulationState::PAUSED;
  timeSpentOnMoves = std::chrono::nanoseconds::zero();
  timeSpentLastMove = std::chrono::milliseconds::zero();
  allocBalance = 0;
  gameResult = "";
  moves.clear();
}

void move(chess::Board &board) {
  if (board.isHalfMoveDraw()) {
    auto result = board.getHalfMoveDrawType();
    gameResult = std::string(magic_enum::enum_name(result.second)) + " " +
                 std::string(magic_enum::enum_name(result.first));
    return;
  }
  auto result = board.isGameOver();
  if (result.second != chess::GameResult::NONE ||
      result.first != chess::GameResultReason::NONE) {
    gameResult = std::string(magic_enum::enum_name(result.second)) + " " +
                 std::string(magic_enum::enum_name(result.first));
    simulationState = SimulationState::PAUSED;
    return;
  }

  std::string turn(magic_enum::enum_name(board.sideToMove().internal()));
  // get stats
  auto beforeAllocCount = memory_stats::get()->allocCount;
  auto beforeTime = std::chrono::high_resolution_clock::now();

  // run!
  auto moveStr = ChessSimulator::Move(board.getFen(true));
  // get stats
  auto afterTime = std::chrono::high_resolution_clock::now();
  auto afterAllocCount = memory_stats::get()->allocCount;
  // apply move
  auto move = chess::uci::uciToMove(board, moveStr);
  // check against memory leaks
  if (afterAllocCount != beforeAllocCount) {
    std::cerr << "Memory leak detected: " << afterAllocCount - beforeAllocCount
              << " allocations not freed" << std::endl;
  }
  board.makeMove(move);

  // update stats
  timeSpentOnMoves += afterTime - beforeTime;
  timeSpentLastMove = afterTime - beforeTime;
  moves.push_back(std::to_string(board.fullMoveNumber()) + " " + turn + ": " +
                  moveStr);
}

auto SvgStringToTexture(std::string svgString) {
  auto document = lunasvg::Document::loadFromData(svgString);
  auto bitmap = document->renderToBitmap(100, 100);
  bitmap.convertToRGBA();
  auto img = GenImageColor(100, 100, WHITE);
  img.data = bitmap.data();
  auto tex = LoadTextureFromImage(img);
  return tex;
}

auto loadPiecesTextures() {
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

int main(int argc, char *argv[]) {
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
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    if (simulationState == SimulationState::RUNNING)
      move(board);

    BeginDrawing();
    ClearBackground(DARKGRAY);
    // game logic

    // draw the chess board
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    minScreenSide = screenWidth < screenHeight ? screenWidth : screenHeight;

    auto displacementX = screenWidth / 2 - minScreenSide / 2;
    auto displacementY = screenHeight / 2 - minScreenSide / 2;

    Rectangle pieceRectSource{0, 0, 100, 100};
    Vector2 origin = {0, 0};

    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        Rectangle location = {displacementX + col * minScreenSide / 8.f,
                              screenHeight - displacementY -
                                  (row + 1) * minScreenSide / 8.f,
                              minScreenSide / 8.f, minScreenSide / 8.f};

        if ((row + col) % 2 == 0)
          DrawRectangleRec(location, WHITE);
        else
          DrawRectangleRec(location, LIGHTGRAY);

        switch (board.at(chess::Square(row * 8 + col)).internal()) {
        case chess::Piece::underlying::WHITEPAWN:
          DrawTexturePro(piecesTextures['P'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::BLACKPAWN:
          DrawTexturePro(piecesTextures['p'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::WHITEROOK:
          DrawTexturePro(piecesTextures['R'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::BLACKROOK:
          DrawTexturePro(piecesTextures['r'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::WHITEKNIGHT:
          DrawTexturePro(piecesTextures['N'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::BLACKKNIGHT:
          DrawTexturePro(piecesTextures['n'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::WHITEBISHOP:
          DrawTexturePro(piecesTextures['B'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::BLACKBISHOP:
          DrawTexturePro(piecesTextures['b'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::WHITEQUEEN:
          DrawTexturePro(piecesTextures['Q'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::BLACKQUEEN:
          DrawTexturePro(piecesTextures['q'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::WHITEKING:
          DrawTexturePro(piecesTextures['K'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        case chess::Piece::underlying::BLACKKING:
          DrawTexturePro(piecesTextures['k'], pieceRectSource, location, origin,
                         0, WHITE);
          break;
        default:
          break;
        }
      }
    }

    // start ImGui Conent
    rlImGuiBegin();

    ImGui::Begin("Settings", nullptr);
    ImGui::Text("%.1fms %.0fFPS | AVG: %.2fms %.1fFPS",
                ImGui::GetIO().DeltaTime * 1000,
                1.0f / ImGui::GetIO().DeltaTime,
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::Separator();
    if (ImGui::Button(ICON_FA_REFRESH)) {
        simulationState = SimulationState::RUNNING;
        reset(board);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLAY)) {
      simulationState = SimulationState::RUNNING;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PAUSE)) {
      simulationState = SimulationState::PAUSED;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLAY ICON_FA_PAUSE)) {
      simulationState = SimulationState::PAUSED;
      move(board);
    }
    ImGui::Separator();
    // statistics
    ImGui::Text("Acc Time spent: %.3fms", timeSpentOnMoves.count() / 1000000.0);
    ImGui::Text("Last move dur:  %.3fms",
                timeSpentLastMove.count() / 1000000.0);
    ImGui::Text("Memory Leaks: %s",
                allocBalance == 0 ? "None" : to_string(allocBalance).c_str());

    ImGui::Text("Game result: %s", gameResult.c_str());
    // moves
    ImGui::Separator();
    ImGui::BeginChild("Moves", ImVec2(0, 0), true);
    // print moves in reverse order
    for (auto it = moves.rbegin(); it != moves.rend(); ++it)
      ImGui::Text("%s", it->c_str());
    ImGui::EndChild(); // end child moves
    ImGui::End(); // end settings
    // end ImGui Content
    rlImGuiEnd();

    // end game logic

    EndDrawing();
    //----------------------------------------------------------------------------------
  }
  rlImGuiShutdown();

  // De-Initialization
  CloseWindow();

  return 0;
}