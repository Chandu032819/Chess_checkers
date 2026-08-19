#pragma once
#include "engine/Observer.h"
#include "engine/Board.h"
#include <iostream>
#include <string>

// A plain console renderer that plugs into the engine purely through
// the GameObserver interface. Swapping this for a GUI (SFML/Qt) later
// means writing a new class here -- zero changes to engine/, chess/, or
// checkers/.
class ConsoleObserver : public engine::GameObserver {
public:
    void onMoveMade(const engine::Move& move, const engine::Board& board) override {
        std::cout << "\nMove: " << toAlgebraic(move.from) << " -> " << toAlgebraic(move.to);
        if (move.isCapture) std::cout << "  (capture)";
        if (move.isCastle) std::cout << "  (castle)";
        if (move.isPromotion) std::cout << "  (promotion)";
        std::cout << "\n";
        printBoard(board);
    }

    void onInvalidMove(const engine::Move& attempted) override {
        std::cout << "Invalid move: " << toAlgebraic(attempted.from)
                   << " -> " << toAlgebraic(attempted.to) << ". Try again.\n";
    }

    void onGameEnd(const std::string& result) override {
        std::cout << "\n=== GAME OVER: " << result << " ===\n";
    }

    static std::string toAlgebraic(engine::Position p) {
        if (p.row < 0 || p.col < 0) return "??";
        std::string s;
        s += char('a' + p.col);
        s += char('0' + (8 - p.row));
        return s;
    }

    static void printBoard(const engine::Board& board) {
        std::cout << "\n   a b c d e f g h\n";
        for (int r = 0; r < board.getRows(); ++r) {
            std::cout << " " << (8 - r) << " ";
            for (int c = 0; c < board.getCols(); ++c) {
                engine::Piece* p = board.getPieceAt(engine::Position(r, c));
                std::cout << (p ? p->getSymbol() : '.') << " ";
            }
            std::cout << (8 - r) << "\n";
        }
        std::cout << "   a b c d e f g h\n\n";
    }
};
