#pragma once
#include <vector>
#include <memory>
#include "Position.h"
#include "Color.h"
#include "Move.h"

namespace engine {

class Board;

// The engine's only contract with a "piece". Chess pieces and Checkers
// pieces both derive from this and plug in their own movement rules by
// overriding getValidMoves(). Nothing else in the engine (Board, Game,
// Command, Observer) needs to know a single rule of either game.
class Piece {
public:
    Piece(Color color, Position pos) : color_(color), position_(pos) {}
    virtual ~Piece() = default;

    // Pseudo-legal moves for this piece given the current board. "Pseudo"
    // because it's purely about this piece's own movement pattern -- the
    // owning Game layers on any board-wide legality (e.g. "does this
    // leave my own king in check", or "captures are mandatory this turn").
    virtual std::vector<Move> getValidMoves(const Board& board) const = 0;

    virtual char getSymbol() const = 0;                    // display glyph
    virtual std::unique_ptr<Piece> clone() const = 0;       // Prototype pattern, needed for board snapshots

    // Default: a square is "attacked" by this piece if it's a destination
    // of one of its own pseudo-legal moves. That's correct for every
    // piece except Pawn (forward move isn't an attack) and King (castling
    // isn't an attack) -- those two override it explicitly.
    virtual std::vector<Position> getAttackSquares(const Board& board) const {
        std::vector<Position> squares;
        for (const auto& m : getValidMoves(board)) squares.push_back(m.to);
        return squares;
    }

    // Generic "is this the game's royal piece" hook, defaulted false.
    // Chess's King overrides it true so check-detection can find it
    // without the engine knowing anything about chess.
    virtual bool isKing() const { return false; }

    Color getColor() const { return color_; }
    Position getPosition() const { return position_; }
    void setPosition(Position p) { position_ = p; }

    bool hasMoved() const { return hasMoved_; }
    void markMoved() { hasMoved_ = true; }
    void setMovedFlag(bool v) { hasMoved_ = v; } // used by MoveCommand::undo

protected:
    Color color_;
    Position position_;
    bool hasMoved_ = false;
};

} // namespace engine
