#pragma once
#include "engine/Game.h"
#include "chess/ChessPieces.h"

namespace chess {

using engine::Game;
using engine::Board;
using engine::Move;
using engine::Position;
using engine::Color;
using engine::Piece;
using engine::opposite;

class ChessGame : public Game {
public:
    ChessGame();

    bool isInCheck(Color c) const;

protected:
    std::vector<Move> getLegalMoves(Position from) const override;
    void applyMove(const Move& move) override;
    bool checkGameEnd() override;

private:
    // King-safety filter: from all of this piece's pseudo-legal moves,
    // keep only the ones that don't leave `asColor`'s own king in check.
    std::vector<Move> safeMovesForPiece(Position from, Color asColor) const;

    bool isKingInCheck(const Board& board, Color kingColor) const;
    Position findKing(const Board& board, Color c) const;
    void setupBoard();

    ChessPieceFactory factory_;
};

} // namespace chess
