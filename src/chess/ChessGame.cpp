#include "chess/ChessGame.h"
#include "engine/Command.h"

namespace chess {

ChessGame::ChessGame() : Game(std::make_unique<Board>(8, 8)) {
    setupBoard();
}

void ChessGame::setupBoard() {
    const std::string backRank = "RNBQKBNR";
    for (int c = 0; c < 8; ++c) {
        board_->placePiece(factory_.createPiece(backRank[c], Color::BLACK, Position(0, c)));
        board_->placePiece(factory_.createPiece('P', Color::BLACK, Position(1, c)));
        board_->placePiece(factory_.createPiece('P', Color::WHITE, Position(6, c)));
        board_->placePiece(factory_.createPiece(backRank[c], Color::WHITE, Position(7, c)));
    }
}

Position ChessGame::findKing(const Board& board, Color c) const {
    for (Piece* p : board.getPiecesByColor(c))
        if (p->isKing()) return p->getPosition();
    return Position(-1, -1);
}

bool ChessGame::isKingInCheck(const Board& board, Color kingColor) const {
    Position kp = findKing(board, kingColor);
    if (kp.row < 0) return false;
    return isSquareAttacked(board, kp, opposite(kingColor));
}

bool ChessGame::isInCheck(Color c) const {
    return isKingInCheck(*board_, c);
}

std::vector<Move> ChessGame::safeMovesForPiece(Position from, Color asColor) const {
    Piece* p = board_->getPieceAt(from);
    std::vector<Move> result;
    if (!p) return result;

    for (auto& m : p->getValidMoves(*board_)) {
        // Simulate the move on a scratch copy of the board and reject it
        // if it leaves asColor's own king in check.
        auto sim = board_->clone();
        engine::MoveCommand cmd(*sim, m);
        cmd.execute();
        if (!isKingInCheck(*sim, asColor)) result.push_back(m);
    }
    return result;
}

std::vector<Move> ChessGame::getLegalMoves(Position from) const {
    Piece* p = board_->getPieceAt(from);
    if (!p || p->getColor() != currentTurn_) return {};
    return safeMovesForPiece(from, currentTurn_);
}

void ChessGame::applyMove(const Move& move) {
    // Any en passant opportunity is only valid for the single move right
    // after a double-step, so clear it before applying whatever move was
    // actually chosen -- getLegalMoves() already read the old value while
    // building the legal-move list, so this is safe to do first.
    board_->setSpecialTargetSquare(Position());

    Game::applyMove(move); // base handles the actual move/capture/castle

    if (move.isDoubleStep) {
        Position target((move.from.row + move.to.row) / 2, move.from.col);
        board_->setSpecialTargetSquare(target);
    }

    if (move.isPromotion) {
        auto original = board_->removePiece(move.to);
        Color c = original->getColor();
        auto promoted = factory_.createPiece(move.promotionSymbol, c, move.to);
        board_->placePiece(std::move(promoted));
        history_.back()->attachPromotion(std::move(original));
    }
}

bool ChessGame::checkGameEnd() {
    Color opponent = opposite(currentTurn_);

    bool hasMoves = false;
    for (Piece* p : board_->getPiecesByColor(opponent)) {
        if (!safeMovesForPiece(p->getPosition(), opponent).empty()) {
            hasMoves = true;
            break;
        }
    }

    if (!hasMoves) {
        if (isKingInCheck(*board_, opponent)) {
            lastResult_ = std::string(currentTurn_ == Color::WHITE ? "White" : "Black") + " wins by checkmate";
        } else {
            lastResult_ = "Draw by stalemate";
        }
        return true;
    }
    return false;
}

} // namespace chess
