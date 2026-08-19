#pragma once
#include <memory>
#include "Move.h"
#include "Piece.h"

namespace engine {

class Board;

// Command pattern: wraps a single applied move together with exactly
// enough state to reverse it. Game keeps a stack of these, which is
// what gives undo/redo for free without Board or Game needing bespoke
// rollback logic per game.
class MoveCommand {
public:
    MoveCommand(Board& board, Move move);

    void execute();
    void undo();

    // Hook used by ChessGame/CheckersGame right after execute(), when a
    // move causes a piece to be swapped for a promoted one. Stores the
    // original piece so undo() can restore it before reversing the move.
    void attachPromotion(std::unique_ptr<Piece> originalPiece);

    const Move& getMove() const { return move_; }

private:
    Board& board_;
    Move move_;

    std::unique_ptr<Piece> capturedPiece_;
    std::unique_ptr<Piece> prePromotionPiece_;
    bool movedPieceHadMovedBefore_ = false;
};

} // namespace engine
