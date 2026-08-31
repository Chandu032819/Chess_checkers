#pragma once
#include <string>
#include <vector>
#include "engine/Observer.h"
#include "ConsoleObserver.h" // reuse toAlgebraic() for move-log text

// Same role as ConsoleObserver, but for a GUI: instead of printing to
// stdout immediately, it just remembers the latest event as plain state
// (strings/bools) that gui_main.cpp reads once per frame to draw status
// text and a move log. This is the entire adapter needed to plug the
// existing engine into an ImGui frontend -- engine/, chess/, checkers/
// are untouched.
class GuiObserver : public engine::GameObserver {
public:
    void onMoveMade(const engine::Move& move, const engine::Board& /*board*/) override {
        std::string entry = ConsoleObserver::toAlgebraic(move.from) + " -> " +
                             ConsoleObserver::toAlgebraic(move.to);
        if (move.isCapture) entry += " (capture)";
        if (move.isCastle) entry += " (castle)";
        if (move.isPromotion) entry += " (promotion)";
        moveLog.push_back(entry);
        statusMessage = "Move played: " + entry;
        invalidFlashTimer = 0.0f;
    }

    void onInvalidMove(const engine::Move& attempted) override {
        statusMessage = "Invalid move: " +
            ConsoleObserver::toAlgebraic(attempted.from) + " -> " +
            ConsoleObserver::toAlgebraic(attempted.to);
        invalidFlashTimer = 1.2f; // seconds the GUI should show it in red
    }

    void onGameEnd(const std::string& result) override {
        statusMessage = "GAME OVER: " + result;
        gameOver = true;
    }

    void reset() {
        moveLog.clear();
        statusMessage = "New game started. White to move.";
        gameOver = false;
        invalidFlashTimer = 0.0f;
    }

    std::vector<std::string> moveLog;
    std::string statusMessage = "New game started. White to move.";
    bool gameOver = false;
    float invalidFlashTimer = 0.0f; // >0 while the invalid-move message should render red
};
