// Dear ImGui frontend for the chess/checkers engine.
//
// This file is the ENTIRE UI layer. It talks to the engine only through
// the same public surface main.cpp (the console driver) uses:
//   Game::playTurn(), Game::undoLastMove(), Game::getBoard(),
//   Game::getCurrentTurn(), Game::isGameOver(), Game::addObserver(),
//   and the new Game::getLegalMovesFor() wrapper (used to highlight a
//   selected piece's destination squares).
// engine/, chess/, checkers/ needed zero changes beyond that one
// wrapper -- proof that the Observer + Template Method design in the
// engine actually pays off when swapping in a real UI.

#include <memory>
#include <string>
#include <cstdio>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "engine/Game.h"
#include "chess/ChessGame.h"
#include "checkers/CheckersGame.h"
#include "GuiObserver.h"

using engine::Position;
using engine::Move;
using engine::Color;
using engine::Piece;

namespace {

// ---- App state -------------------------------------------------------
std::unique_ptr<engine::Game> game;
GuiObserver observer;
std::string activeGameName = "Chess";

Position selected;                 // Position() default == "none" (row=-1)
std::vector<Move> legalForSelected; // legal destinations for `selected`

struct PendingPromotion {
    bool active = false;
    Position from, to;
} pendingPromotion;

void startNewGame(const std::string& name) {
    if (name == "Chess") game = std::make_unique<chess::ChessGame>();
    else                  game = std::make_unique<checkers::CheckersGame>();
    game->addObserver(&observer);
    activeGameName = name;
    selected = Position();
    legalForSelected.clear();
    pendingPromotion.active = false;
    observer.reset();
}

bool isLegalDestination(Position p) {
    for (const auto& m : legalForSelected) if (m.to == p) return true;
    return false;
}

const Move* findLegalMove(Position to) {
    for (const auto& m : legalForSelected) if (m.to == to) return &m;
    return nullptr;
}

void clearSelection() {
    selected = Position();
    legalForSelected.clear();
}

// Click on square `p`. Handles select / reselect / attempt-move / promotion.
void handleSquareClick(Position p) {
    if (!game || game->isGameOver() || pendingPromotion.active) return;

    const engine::Board& board = game->getBoard();
    Piece* clickedPiece = board.getPieceAt(p);

    if (selected == Position()) {
        // Nothing selected yet -- select if there's a piece there.
        if (clickedPiece) {
            selected = p;
            legalForSelected = game->getLegalMovesFor(p); // empty if not this side's turn
        }
        return;
    }

    if (p == selected) { clearSelection(); return; }

    // Clicking another of your own pieces re-selects instead of attempting a move.
    Piece* selectedPiece = board.getPieceAt(selected);
    if (clickedPiece && selectedPiece && clickedPiece->getColor() == selectedPiece->getColor()) {
        selected = p;
        legalForSelected = game->getLegalMovesFor(p);
        return;
    }

    const Move* m = findLegalMove(p);
    if (m && m->isPromotion) {
        pendingPromotion = {true, selected, p};
        clearSelection();
        return;
    }

    game->playTurn(selected, p); // engine re-validates regardless; GuiObserver reports invalid moves
    clearSelection();
}

ImU32 col(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }

void drawBoard(float squareSize) {
    if (!game) return;
    const engine::Board& board = game->getBoard();
    int rows = board.getRows(), cols = board.getCols();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    const ImU32 light = col(235, 222, 195), dark = col(120, 96, 72);
    const ImU32 selectCol = col(255, 215, 90, 140);
    const ImU32 targetDot = col(60, 150, 60, 200);
    const ImU32 whiteChip = col(245, 245, 245), whiteChipEdge = col(60, 60, 60);
    const ImU32 blackChip = col(35, 35, 40), blackChipEdge = col(220, 220, 220);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            Position p(r, c);
            ImVec2 topLeft(origin.x + c * squareSize, origin.y + r * squareSize);
            ImVec2 botRight(topLeft.x + squareSize, topLeft.y + squareSize);

            dl->AddRectFilled(topLeft, botRight, ((r + c) % 2 == 0) ? light : dark);
            if (p == selected) dl->AddRectFilled(topLeft, botRight, selectCol);

            Piece* piece = board.getPieceAt(p);
            ImVec2 center(topLeft.x + squareSize * 0.5f, topLeft.y + squareSize * 0.5f);

            if (piece) {
                bool white = piece->getColor() == Color::WHITE;
                float radius = squareSize * 0.38f;
                dl->AddCircleFilled(center, radius, white ? whiteChip : blackChip, 32);
                dl->AddCircle(center, radius, white ? whiteChipEdge : blackChipEdge, 32, 2.0f);

                char sym[2] = { piece->getSymbol(), '\0' };
                ImVec2 textSize = ImGui::CalcTextSize(sym);
                ImVec2 textPos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);
                dl->AddText(textPos, white ? whiteChipEdge : blackChipEdge, sym);
            } else if (isLegalDestination(p)) {
                dl->AddCircleFilled(center, squareSize * 0.12f, targetDot, 24);
            }

            // Invisible button placed on top to capture the click.
            ImGui::SetCursorScreenPos(topLeft);
            char id[32];
            std::snprintf(id, sizeof(id), "##sq%d_%d", r, c);
            ImGui::InvisibleButton(id, ImVec2(squareSize, squareSize));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) handleSquareClick(p);
        }
    }
    ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + rows * squareSize + 8));
}

void drawSidebar() {
    ImGui::Text("Game: %s", activeGameName.c_str());
    ImGui::Spacing();

    if (ImGui::Button("New Chess Game", ImVec2(-1, 0))) startNewGame("Chess");
    if (ImGui::Button("New Checkers Game", ImVec2(-1, 0))) startNewGame("Checkers");
    if (ImGui::Button("Undo Last Move", ImVec2(-1, 0)) && game) {
        game->undoLastMove();
        clearSelection();
    }

    ImGui::Separator();

    if (game) {
        if (game->isGameOver()) {
            ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.1f, 1.0f), "Game over");
        } else {
            bool white = game->getCurrentTurn() == Color::WHITE;
            ImGui::Text("Turn: %s", white ? "White" : "Black");
        }
    }

    ImGui::Spacing();
    ImVec4 statusColor = observer.invalidFlashTimer > 0.0f
        ? ImVec4(0.9f, 0.3f, 0.3f, 1.0f) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
    ImGui::TextColored(statusColor, "%s", observer.statusMessage.c_str());

    ImGui::Separator();
    ImGui::Text("Move log");
    ImGui::BeginChild("MoveLog", ImVec2(0, 0), true);
    for (size_t i = 0; i < observer.moveLog.size(); ++i) {
        ImGui::Text("%2zu. %s", i + 1, observer.moveLog[i].c_str());
    }
    if (!observer.moveLog.empty()) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void drawPromotionPopup() {
    if (pendingPromotion.active && !ImGui::IsPopupOpen("Promote pawn"))
        ImGui::OpenPopup("Promote pawn");

    if (ImGui::BeginPopupModal("Promote pawn", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Choose promotion piece:");
        auto choose = [&](char c, const char* label) {
            if (ImGui::Button(label, ImVec2(80, 0))) {
                game->playTurn(pendingPromotion.from, pendingPromotion.to, c);
                pendingPromotion.active = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        };
        choose('Q', "Queen");
        choose('R', "Rook");
        choose('B', "Bishop");
        choose('N', "Knight");
        ImGui::NewLine();
        ImGui::EndPopup();
    }
}

} // namespace

int main(int argc, char** argv) {
    int testFrames = 0; // if >0, auto-exit after this many frames (headless smoke test)
    for (int i = 1; i < argc; ++i) {
        int n = 0;
        if (std::sscanf(argv[i], "--test-frames=%d", &n) == 1) testFrames = n;
    }

    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1100, 720, "Chess & Checkers", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "glfwCreateWindow failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    startNewGame("Chess");

    int framesRun = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (observer.invalidFlashTimer > 0.0f) observer.invalidFlashTimer -= io.DeltaTime;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Root", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        float boardArea = ImGui::GetContentRegionAvail().y - 16.0f;
        int dim = game ? game->getBoard().getRows() : 8;
        float squareSize = boardArea / dim;

        ImGui::BeginChild("BoardPanel", ImVec2(squareSize * dim + 16, 0), true);
        ImGui::Dummy(ImVec2(4, 4));
        ImGui::SameLine();
        drawBoard(squareSize);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Sidebar", ImVec2(0, 0), true);
        drawSidebar();
        ImGui::EndChild();

        drawPromotionPopup();

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.11f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (testFrames > 0 && ++framesRun >= testFrames) {
            // Headless smoke-test hook: dump the framebuffer to a PPM so a
            // build script (or CI) can confirm real pixels were drawn.
            std::vector<unsigned char> pixels(display_w * display_h * 3);
            glReadPixels(0, 0, display_w, display_h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
            FILE* f = std::fopen("gui_test_screenshot.ppm", "wb");
            if (f) {
                std::fprintf(f, "P6\n%d %d\n255\n", display_w, display_h);
                for (int y = display_h - 1; y >= 0; --y) // flip: GL origin is bottom-left
                    std::fwrite(pixels.data() + y * display_w * 3, 1, display_w * 3, f);
                std::fclose(f);
            }
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
