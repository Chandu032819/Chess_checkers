#pragma once
#include "Position.h"

namespace engine {

// A game-agnostic description of "move this piece from A to B", plus
// enough optional flags to cover the special cases both Chess and
// Checkers need (captures at a square other than `to`, castling,
// promotion, double-step pawn openings). A game that needs none of the
// extra flags just leaves them at their defaults.
struct Move {
    Position from;
    Position to;

    bool isCapture = false;
    Position capturedPos{-1, -1}; // may differ from `to` (checkers jump)

    bool isPromotion = false;
    char promotionSymbol = 'Q';

    bool isCastle = false;
    Position rookFrom{-1, -1};
    Position rookTo{-1, -1};

    bool isDoubleStep = false; // pawn's opening two-square move

    Move() = default;
    Move(Position f, Position t) : from(f), to(t) {}
};

} // namespace engine
