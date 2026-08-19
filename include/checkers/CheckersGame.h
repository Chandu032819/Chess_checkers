#pragma once
#include "engine/Game.h"
#include "checkers/CheckersPieces.h"

namespace checkers {

using engine::Game;
using engine::Board;
using engine::Move;
using engine::Position;
using engine::Color;
using engine::Piece;
using engine::opposite;

class CheckersGame : public Game {
public:
    CheckersGame();

protected:
    std::vector<Move> getLegalMoves(Position from) const override;
    void applyMove(const Move& move) override;
    bool checkGameEnd() override;
    bool continuesTurn(const Move& lastMove) const override;

private:
    void setupBoard();
    bool anyCaptureAvailable(Color c) const;

    CheckersPieceFactory factory_;
};

} // namespace checkers
