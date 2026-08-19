#include "engine/Command.h"
#include "engine/Board.h"

namespace engine {

MoveCommand::MoveCommand(Board& board, Move move) : board_(board), move_(move) {}

void MoveCommand::execute() {
    Piece* movingPiece = board_.getPieceAt(move_.from);
    if (!movingPiece) return;
    movedPieceHadMovedBefore_ = movingPiece->hasMoved();

    if (move_.isCapture) {
        capturedPiece_ = board_.removePiece(move_.capturedPos);
    }

    board_.movePieceRaw(move_.from, move_.to);

    if (move_.isCastle) {
        board_.movePieceRaw(move_.rookFrom, move_.rookTo);
    }

    if (Piece* landed = board_.getPieceAt(move_.to)) {
        landed->markMoved();
    }
}

void MoveCommand::attachPromotion(std::unique_ptr<Piece> originalPiece) {
    prePromotionPiece_ = std::move(originalPiece);
}

void MoveCommand::undo() {
    // If this move ended in a promotion, swap the promoted piece back
    // for the original BEFORE moving anything back to `from` -- otherwise
    // we'd relocate the promoted piece instead of the original.
    if (prePromotionPiece_) {
        board_.removePiece(move_.to);
        prePromotionPiece_->setPosition(move_.to);
        board_.placePiece(std::move(prePromotionPiece_));
    }

    board_.movePieceRaw(move_.to, move_.from);
    if (Piece* restored = board_.getPieceAt(move_.from)) {
        restored->setMovedFlag(movedPieceHadMovedBefore_);
    }

    if (move_.isCastle) {
        board_.movePieceRaw(move_.rookTo, move_.rookFrom);
    }

    if (move_.isCapture && capturedPiece_) {
        board_.placePiece(std::move(capturedPiece_));
    }
}

} // namespace engine
