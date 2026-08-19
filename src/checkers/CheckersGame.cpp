#include "checkers/CheckersGame.h"
#include "engine/Command.h"

namespace checkers {

CheckersGame::CheckersGame() : Game(std::make_unique<Board>(8, 8)) {
    setupBoard();
}

void CheckersGame::setupBoard() {
    // Standard American checkers: pieces sit only on dark squares
    // ((row + col) is odd), three rows deep for each side.
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 8; ++c)
            if ((r + c) % 2 == 1)
                board_->placePiece(factory_.createPiece('m', Color::BLACK, Position(r, c)));

    for (int r = 5; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if ((r + c) % 2 == 1)
                board_->placePiece(factory_.createPiece('m', Color::WHITE, Position(r, c)));
}

bool CheckersGame::anyCaptureAvailable(Color c) const {
    for (Piece* p : board_->getPiecesByColor(c))
        for (auto& m : p->getValidMoves(*board_))
            if (m.isCapture) return true;
    return false;
}

std::vector<Move> CheckersGame::getLegalMoves(Position from) const {
    Piece* p = board_->getPieceAt(from);
    if (!p || p->getColor() != currentTurn_) return {};

    auto candidates = p->getValidMoves(*board_);

    // Forced-capture rule: if ANY piece belonging to the current player
    // has a capture available anywhere on the board, only capture moves
    // are legal this turn -- even for a piece that itself has a quiet
    // move available.
    if (anyCaptureAvailable(currentTurn_)) {
        std::vector<Move> onlyCaptures;
        for (auto& m : candidates)
            if (m.isCapture) onlyCaptures.push_back(m);
        return onlyCaptures;
    }
    return candidates;
}

void CheckersGame::applyMove(const Move& move) {
    Game::applyMove(move); // base handles the move + removing the jumped piece

    Piece* landed = board_->getPieceAt(move.to);
    if (landed && !dynamic_cast<CheckersKing*>(landed)) {
        int backRow = (landed->getColor() == Color::WHITE) ? 0 : board_->getRows() - 1;
        if (move.to.row == backRow) {
            auto original = board_->removePiece(move.to);
            auto promoted = factory_.createPiece('k', original->getColor(), move.to);
            board_->placePiece(std::move(promoted));
            history_.back()->attachPromotion(std::move(original));
        }
    }
}

bool CheckersGame::continuesTurn(const Move& lastMove) const {
    // Mandatory multi-jump: after a capture, if the SAME piece has
    // another capture available from its new square, it must keep
    // jumping instead of passing the turn.
    if (!lastMove.isCapture) return false;
    Piece* p = board_->getPieceAt(lastMove.to);
    if (!p) return false;
    for (auto& m : p->getValidMoves(*board_))
        if (m.isCapture) return true;
    return false;
}

bool CheckersGame::checkGameEnd() {
    Color opponent = opposite(currentTurn_);
    auto pieces = board_->getPiecesByColor(opponent);

    if (pieces.empty()) {
        lastResult_ = std::string(currentTurn_ == Color::WHITE ? "White" : "Black") +
                       " wins - opponent has no pieces left";
        return true;
    }

    for (Piece* p : pieces) {
        if (!p->getValidMoves(*board_).empty()) return false; // opponent can still move
    }

    lastResult_ = std::string(currentTurn_ == Color::WHITE ? "White" : "Black") +
                   " wins - opponent is blocked";
    return true;
}

} // namespace checkers
