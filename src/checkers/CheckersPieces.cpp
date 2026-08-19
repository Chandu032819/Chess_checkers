#include "checkers/CheckersPieces.h"
#include <cctype>

namespace checkers {

std::vector<Move> Man::getValidMoves(const Board& board) const {
    std::vector<Move> moves;
    int dir = (color_ == Color::WHITE) ? -1 : 1; // white moves toward row 0, black toward the last row

    for (int dc : {-1, 1}) {
        Position step(position_.row + dir, position_.col + dc);
        if (board.inBounds(step) && board.isEmpty(step)) {
            moves.emplace_back(position_, step);
        }

        Position land(position_.row + 2 * dir, position_.col + 2 * dc);
        if (board.inBounds(land) && board.isEmpty(land)) {
            Piece* mid = board.getPieceAt(step);
            if (mid && mid->getColor() != color_) {
                Move m(position_, land);
                m.isCapture = true;
                m.capturedPos = step;
                moves.push_back(m);
            }
        }
    }
    return moves;
}

std::vector<Move> CheckersKing::getValidMoves(const Board& board) const {
    std::vector<Move> moves;
    static const int dr[4] = {-1, -1, 1, 1};
    static const int dc[4] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        Position step(position_.row + dr[i], position_.col + dc[i]);
        if (board.inBounds(step) && board.isEmpty(step)) {
            moves.emplace_back(position_, step);
        }

        Position land(position_.row + 2 * dr[i], position_.col + 2 * dc[i]);
        if (board.inBounds(land) && board.isEmpty(land)) {
            Piece* mid = board.getPieceAt(step);
            if (mid && mid->getColor() != color_) {
                Move m(position_, land);
                m.isCapture = true;
                m.capturedPos = step;
                moves.push_back(m);
            }
        }
    }
    return moves;
}

std::unique_ptr<Piece> CheckersPieceFactory::createPiece(char symbol, Color color, Position pos) const {
    switch (std::tolower(static_cast<unsigned char>(symbol))) {
        case 'm': return std::make_unique<Man>(color, pos);
        case 'k': return std::make_unique<CheckersKing>(color, pos);
        default: return nullptr;
    }
}

} // namespace checkers
