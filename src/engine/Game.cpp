#include "engine/Game.h"
#include <algorithm>

namespace engine {

Game::Game(std::unique_ptr<Board> board, Color startingTurn)
    : board_(std::move(board)), currentTurn_(startingTurn) {}

std::vector<Move> Game::getLegalMoves(Position from) const {
    Piece* p = board_->getPieceAt(from);
    if (!p || p->getColor() != currentTurn_) return {};
    return p->getValidMoves(*board_);
}

void Game::applyMove(const Move& move) {
    auto cmd = std::make_unique<MoveCommand>(*board_, move);
    cmd->execute();
    history_.push_back(std::move(cmd));
}

bool Game::playTurn(Position from, Position to, char promotionChoice) {
    if (gameOver_) return false;

    Piece* mover = board_->getPieceAt(from);
    if (!mover || mover->getColor() != currentTurn_) {
        notifyInvalidMove(Move(from, to));
        return false;
    }

    std::vector<Move> legal = getLegalMoves(from);
    auto it = std::find_if(legal.begin(), legal.end(),
                            [&](const Move& m) { return m.to == to; });
    if (it == legal.end()) {
        notifyInvalidMove(Move(from, to));
        return false;
    }

    Move chosen = *it;
    if (chosen.isPromotion) chosen.promotionSymbol = promotionChoice;

    applyMove(chosen);
    notifyMoveMade(chosen);

    if (checkGameEnd()) {
        gameOver_ = true;
        notifyGameEnd(lastResult_);
        return true;
    }

    if (!continuesTurn(chosen)) {
        currentTurn_ = opposite(currentTurn_);
    }
    return true;
}

bool Game::undoLastMove() {
    if (history_.empty()) return false;
    history_.back()->undo();
    history_.pop_back();
    currentTurn_ = opposite(currentTurn_);
    gameOver_ = false;
    return true;
}

void Game::addObserver(GameObserver* obs) { observers_.push_back(obs); }

void Game::notifyMoveMade(const Move& move) {
    for (auto* obs : observers_) obs->onMoveMade(move, *board_);
}
void Game::notifyInvalidMove(const Move& attempted) {
    for (auto* obs : observers_) obs->onInvalidMove(attempted);
}
void Game::notifyGameEnd(const std::string& result) {
    for (auto* obs : observers_) obs->onGameEnd(result);
}

} // namespace engine
