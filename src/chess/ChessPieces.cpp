#include "chess/ChessPieces.h"
#include <cctype>
#include <utility>

namespace chess {

namespace {

// Shared sliding-move generator used by Bishop, Rook, and Queen: walk
// each direction until the edge of the board or a piece is hit.
std::vector<Move> slideMoves(const Board& board, Position from, Color color,
                              const std::vector<std::pair<int, int>>& dirs) {
    std::vector<Move> moves;
    for (auto& d : dirs) {
        Position cur(from.row + d.first, from.col + d.second);
        while (board.inBounds(cur)) {
            Piece* occ = board.getPieceAt(cur);
            if (!occ) {
                moves.emplace_back(from, cur);
            } else {
                if (occ->getColor() != color) {
                    Move m(from, cur);
                    m.isCapture = true;
                    m.capturedPos = cur;
                    moves.push_back(m);
                }
                break; // blocked either way -- own piece or a captured enemy
            }
            cur = Position(cur.row + d.first, cur.col + d.second);
        }
    }
    return moves;
}

} // anonymous namespace

bool isSquareAttacked(const Board& board, Position square, Color byColor) {
    for (Piece* p : board.getPiecesByColor(byColor)) {
        for (auto& atk : p->getAttackSquares(board))
            if (atk == square) return true;
    }
    return false;
}

// ---------------------------------------------------------------- Pawn
std::vector<Move> Pawn::getValidMoves(const Board& board) const {
    std::vector<Move> moves;
    int dir = (color_ == Color::WHITE) ? -1 : 1;
    int startRow = (color_ == Color::WHITE) ? 6 : 1;
    int promoRow = (color_ == Color::WHITE) ? 0 : 7;

    Position oneStep(position_.row + dir, position_.col);
    if (board.inBounds(oneStep) && board.isEmpty(oneStep)) {
        Move m(position_, oneStep);
        if (oneStep.row == promoRow) m.isPromotion = true;
        moves.push_back(m);

        if (position_.row == startRow) {
            Position twoStep(position_.row + 2 * dir, position_.col);
            if (board.isEmpty(twoStep)) {
                Move m2(position_, twoStep);
                m2.isDoubleStep = true;
                moves.push_back(m2);
            }
        }
    }

    for (int dc : {-1, 1}) {
        Position diag(position_.row + dir, position_.col + dc);
        if (!board.inBounds(diag)) continue;
        Piece* occ = board.getPieceAt(diag);
        if (occ && occ->getColor() != color_) {
            Move m(position_, diag);
            m.isCapture = true;
            m.capturedPos = diag;
            if (diag.row == promoRow) m.isPromotion = true;
            moves.push_back(m);
        } else if (!occ && diag == board.getSpecialTargetSquare()) {
            // En passant: the square is empty, but it's the recorded
            // target from the opponent's pawn having just double-stepped
            // past it -- so the actual captured pawn sits beside us, not
            // on the destination square.
            Move m(position_, diag);
            m.isCapture = true;
            m.capturedPos = Position(position_.row, diag.col);
            moves.push_back(m);
        }
    }
    return moves;
}

std::vector<Position> Pawn::getAttackSquares(const Board& board) const {
    std::vector<Position> squares;
    int dir = (color_ == Color::WHITE) ? -1 : 1;
    for (int dc : {-1, 1}) {
        Position p(position_.row + dir, position_.col + dc);
        if (board.inBounds(p)) squares.push_back(p);
    }
    return squares;
}

// -------------------------------------------------------------- Knight
std::vector<Move> Knight::getValidMoves(const Board& board) const {
    std::vector<Move> moves;
    static const int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
    static const int dc[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
    for (int i = 0; i < 8; ++i) {
        Position to(position_.row + dr[i], position_.col + dc[i]);
        if (!board.inBounds(to)) continue;
        Piece* occ = board.getPieceAt(to);
        if (!occ) {
            moves.emplace_back(position_, to);
        } else if (occ->getColor() != color_) {
            Move m(position_, to);
            m.isCapture = true;
            m.capturedPos = to;
            moves.push_back(m);
        }
    }
    return moves;
}

// -------------------------------------------------------------- Bishop
std::vector<Move> Bishop::getValidMoves(const Board& board) const {
    return slideMoves(board, position_, color_, {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
}

// ---------------------------------------------------------------- Rook
std::vector<Move> Rook::getValidMoves(const Board& board) const {
    return slideMoves(board, position_, color_, {{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
}

// --------------------------------------------------------------- Queen
std::vector<Move> Queen::getValidMoves(const Board& board) const {
    return slideMoves(board, position_, color_,
                       {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}, {-1, 0}, {1, 0}, {0, -1}, {0, 1}});
}

// ---------------------------------------------------------------- King
std::vector<Move> King::getValidMoves(const Board& board) const {
    std::vector<Move> moves;
    static const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    for (int i = 0; i < 8; ++i) {
        Position to(position_.row + dr[i], position_.col + dc[i]);
        if (!board.inBounds(to)) continue;
        Piece* occ = board.getPieceAt(to);
        if (!occ) {
            moves.emplace_back(position_, to);
        } else if (occ->getColor() != color_) {
            Move m(position_, to);
            m.isCapture = true;
            m.capturedPos = to;
            moves.push_back(m);
        }
    }

    // Castling: king hasn't moved, isn't currently in check, the rook on
    // that side hasn't moved, the squares between are empty, and the
    // king doesn't pass through or land on an attacked square.
    if (!hasMoved_ && !isSquareAttacked(board, position_, opposite(color_))) {
        int row = position_.row;

        Piece* rookK = board.getPieceAt(Position(row, 7));
        if (rookK && !rookK->hasMoved() && rookK->getColor() == color_) {
            Position sq1(row, 5), sq2(row, 6);
            if (board.isEmpty(sq1) && board.isEmpty(sq2) &&
                !isSquareAttacked(board, sq1, opposite(color_)) &&
                !isSquareAttacked(board, sq2, opposite(color_))) {
                Move m(position_, sq2);
                m.isCastle = true;
                m.rookFrom = Position(row, 7);
                m.rookTo = Position(row, 5);
                moves.push_back(m);
            }
        }

        Piece* rookQ = board.getPieceAt(Position(row, 0));
        if (rookQ && !rookQ->hasMoved() && rookQ->getColor() == color_) {
            Position sq1(row, 1), sq2(row, 2), sq3(row, 3);
            if (board.isEmpty(sq1) && board.isEmpty(sq2) && board.isEmpty(sq3) &&
                !isSquareAttacked(board, sq2, opposite(color_)) &&
                !isSquareAttacked(board, sq3, opposite(color_))) {
                Move m(position_, sq2);
                m.isCastle = true;
                m.rookFrom = Position(row, 0);
                m.rookTo = Position(row, 3);
                moves.push_back(m);
            }
        }
    }
    return moves;
}

std::vector<Position> King::getAttackSquares(const Board& board) const {
    std::vector<Position> squares;
    static const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    for (int i = 0; i < 8; ++i) {
        Position p(position_.row + dr[i], position_.col + dc[i]);
        if (board.inBounds(p)) squares.push_back(p);
    }
    return squares;
}

// ------------------------------------------------------------- Factory
std::unique_ptr<Piece> ChessPieceFactory::createPiece(char symbol, Color color, Position pos) const {
    switch (std::toupper(static_cast<unsigned char>(symbol))) {
        case 'K': return std::make_unique<King>(color, pos);
        case 'Q': return std::make_unique<Queen>(color, pos);
        case 'R': return std::make_unique<Rook>(color, pos);
        case 'B': return std::make_unique<Bishop>(color, pos);
        case 'N': return std::make_unique<Knight>(color, pos);
        case 'P': return std::make_unique<Pawn>(color, pos);
        default: return nullptr;
    }
}

} // namespace chess
