#pragma once
#include "engine/Piece.h"
#include "engine/Board.h"
#include <vector>
#include <memory>

namespace chess {

using engine::Piece;
using engine::Board;
using engine::Color;
using engine::Position;
using engine::Move;
using engine::opposite;

// Is `square` attacked by any piece of `byColor`? Used for check
// detection and for King's castling-safety checks.
bool isSquareAttacked(const Board& board, Position square, Color byColor);

class Pawn : public Piece {
public:
    Pawn(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    std::vector<Position> getAttackSquares(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'P' : 'p'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<Pawn>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

class Knight : public Piece {
public:
    Knight(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'N' : 'n'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<Knight>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

class Bishop : public Piece {
public:
    Bishop(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'B' : 'b'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<Bishop>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

class Rook : public Piece {
public:
    Rook(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'R' : 'r'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<Rook>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

class Queen : public Piece {
public:
    Queen(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'Q' : 'q'; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<Queen>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

class King : public Piece {
public:
    King(Color color, Position pos) : Piece(color, pos) {}
    std::vector<Move> getValidMoves(const Board& board) const override;
    std::vector<Position> getAttackSquares(const Board& board) const override;
    char getSymbol() const override { return color_ == Color::WHITE ? 'K' : 'k'; }
    bool isKing() const override { return true; }
    std::unique_ptr<Piece> clone() const override {
        auto p = std::make_unique<King>(color_, position_);
        p->setMovedFlag(hasMoved_);
        return p;
    }
};

// Factory pattern: builds a chess piece from its algebraic letter
// (K/Q/R/B/N/P). Used for the standard board setup and for promotion,
// so both places share one source of truth for "letter -> piece type".
class ChessPieceFactory {
public:
    std::unique_ptr<Piece> createPiece(char symbol, Color color, Position pos) const;
};

} // namespace chess
