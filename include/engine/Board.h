#pragma once
#include <vector>
#include <memory>
#include "Piece.h"

namespace engine {

// Owns the grid of pieces. This is the encapsulation boundary: nothing
// outside Board ever touches the raw vector<vector<unique_ptr<Piece>>>,
// so "one piece per square" and ownership are invariants that simply
// can't be violated from outside this class.
class Board {
public:
    Board(int rows, int cols);

    int getRows() const { return rows_; }
    int getCols() const { return cols_; }
    bool inBounds(Position p) const;

    Piece* getPieceAt(Position p) const;   // nullptr if empty or out of bounds
    bool isEmpty(Position p) const;

    void placePiece(std::unique_ptr<Piece> piece);   // takes ownership, uses piece->getPosition()
    std::unique_ptr<Piece> removePiece(Position p);    // returns ownership (nullptr if empty)
    void movePieceRaw(Position from, Position to);     // low-level relocate, no rule checking

    std::vector<Piece*> getPiecesByColor(Color c) const;

    std::unique_ptr<Board> clone() const; // deep copy, used to simulate "what if I made this move"

    // A single generic square of "special state" a game can attach to the
    // board across one move -- used by Chess for en passant (the square a
    // capturing pawn would land on). This is the one deliberate exception
    // to "Board knows nothing about any specific game": modeling it as a
    // full turn-history / game-context object would be more "pure" but is
    // a lot of extra plumbing for one flag that only Chess ever reads.
    // Checkers never touches it. Defaults to Position() (row -1 = "none").
    void setSpecialTargetSquare(Position p) { specialTarget_ = p; }
    Position getSpecialTargetSquare() const { return specialTarget_; }

private:
    int rows_, cols_;
    std::vector<std::vector<std::unique_ptr<Piece>>> grid_;
    Position specialTarget_;
};

} // namespace engine
