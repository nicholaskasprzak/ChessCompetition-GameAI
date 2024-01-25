#include "chess-simulator.h"
#include "chess.hpp"

#include "raylib.h"
#include "raymath.h"

#include "imgui.h"
#include "rlImGui.h"

int main(int argc, char* argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 800;
    int minScreenSide = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Chess AI Simulator");
    SetTargetFPS(144);
    rlImGuiSetup(true);

    bool aiEnabled = false;
    chess::Color aiColor = chess::Color("b");

    chess::Board board;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // start ImGui Conent
        rlImGuiBegin();

        ImGui::Begin("Settings", nullptr);
        ImGui::Text("%.1fms %.0fFPS | AVG: %.2fms %.1fFPS", ImGui::GetIO().DeltaTime * 1000, 1.0f / ImGui::GetIO().DeltaTime,
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
//            state.Reset();
//            score = Heuristics::MaterialScore(&state);
        };
        ImGui::SameLine();
        if (ImGui::Button("Undo")){
//        && !previousStates.empty()) {
//            validMoves = {};
//            selected = {INT32_MIN, INT32_MIN};
//            state = previousStates.top();
//            previousStates.pop();
        }
        ImGui::Separator();

//        ImGui::LabelText("Score", "%.1f", score);
        ImGui::Separator();

        if (ImGui::Checkbox("AI Enabled", &aiEnabled))
            if (aiEnabled == true) aiColor = chess::Color("b");

        static bool aiIsBlackStatic = true;
        if (aiEnabled) {
            if (ImGui::Checkbox("AI is Black", &aiIsBlackStatic)) {
                if (aiIsBlackStatic)
                    aiColor = chess::Color("b");
                else
                    aiColor = chess::Color("w");
            }
        }
//        ImGui::LabelText(state.GetTurn() == PieceColor::White ? "White" : "Black", "Turn:");
        ImGui::End();  // end settings

        // game logic

        // draww the chess board
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        minScreenSide = screenWidth < screenHeight ? screenWidth : screenHeight;

        for(int row=0;row<8;row++){
            for(int col=0;col<8;col++){
                if((row+col)%2==0)
                    DrawRectangle(screenWidth/2 - minScreenSide/2 + col*minScreenSide/8, row*minScreenSide/8, minScreenSide/8, minScreenSide/8, WHITE);
                else
                    DrawRectangle(screenWidth/2 - minScreenSide/2 + col*minScreenSide/8, row*minScreenSide/8, minScreenSide/8, minScreenSide/8, BLACK);
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