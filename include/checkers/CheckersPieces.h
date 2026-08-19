#pragma once
#include "engine/Piece.h"
#include "engine/Board.h"
#include <vector>
#include <memory>

namespace checkers {

using engine::Piece;
using engine::Board;
using engine::Color;
using engine::Position;
using engine::Move;

// A regular checkers piece: moves and captures diagonally forward only.
class Man : public Piece {
public:
    Man(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'o' : 'x'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<Man>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

// A crowned piece: moves and captures diagonally in all four directions.
class CheckersKing : public Piece {
public:
    CheckersKing(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'O' : 'X'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<CheckersKing>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

// Factory pattern: builds a checkers piece from 'm' (man) or 'k' (king).
// Same role as ChessPieceFactory -- one source of truth for "symbol ->
// piece type", used both by the initial setup and by king-promotion.
class CheckersPieceFactory {
public:
    std::unique_ptr<Piece> createPiece(char symbol, Color color, Position pos) const;
};

} // namespace checkers
