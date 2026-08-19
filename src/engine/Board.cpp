#include "engine/Board.h"

namespace engine {

Board::Board(int rows, int cols) : rows_(rows), cols_(cols) {
    grid_.resize(rows_);
    for (auto& row : grid_) row.resize(cols_);
}

bool Board::inBounds(Position p) const {
    return p.row >= 0 && p.row < rows_ && p.col >= 0 && p.col < cols_;
}

Piece* Board::getPieceAt(Position p) const {
    if (!inBounds(p)) return nullptr;
    return grid_[p.row][p.col].get();
}

bool Board::isEmpty(Position p) const {
    return getPieceAt(p) == nullptr;
}

void Board::placePiece(std::unique_ptr<Piece> piece) {
    if (!piece) return;
    Position p = piece->getPosition();
    if (!inBounds(p)) return;
    grid_[p.row][p.col] = std::move(piece);
}

std::unique_ptr<Piece> Board::removePiece(Position p) {
    if (!inBounds(p)) return nullptr;
    std::unique_ptr<Piece> taken = std::move(grid_[p.row][p.col]);
    grid_[p.row][p.col].reset();
    return taken;
}

void Board::movePieceRaw(Position from, Position to) {
    if (!inBounds(from) || !inBounds(to)) return;
    grid_[to.row][to.col] = std::move(grid_[from.row][from.col]);
    grid_[from.row][from.col].reset();
    if (grid_[to.row][to.col]) {
        grid_[to.row][to.col]->setPosition(to);
    }
}

std::vector<Piece*> Board::getPiecesByColor(Color c) const {
    std::vector<Piece*> result;
    for (int r = 0; r < rows_; ++r)
        for (int col = 0; col < cols_; ++col)
            if (grid_[r][col] && grid_[r][col]->getColor() == c)
                result.push_back(grid_[r][col].get());
    return result;
}

std::unique_ptr<Board> Board::clone() const {
    auto copy = std::make_unique<Board>(rows_, cols_);
    for (int r = 0; r < rows_; ++r)
        for (int c = 0; c < cols_; ++c)
            if (grid_[r][c])
                copy->placePiece(grid_[r][c]->clone());
    copy->setSpecialTargetSquare(specialTarget_);
    return copy;
}

} // namespace engine
