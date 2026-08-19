// Lightweight assert-based tests -- no external test framework needed.
// Run the `run_tests` executable after building.
#include <cassert>
#include <iostream>
#include "engine/Board.h"
#include "engine/Command.h"
#include "chess/ChessGame.h"
#include "chess/ChessPieces.h"
#include "checkers/CheckersGame.h"
#include "checkers/CheckersPieces.h"

using namespace engine;

static void testChessBasicMoves() {
    chess::ChessGame game;
    assert(game.playTurn(Position(6, 4), Position(4, 4))); // e2-e4
    assert(game.playTurn(Position(1, 4), Position(3, 4))); // e7-e5
    assert(game.getBoard().getPieceAt(Position(4, 4)) != nullptr);
    assert(game.getBoard().getPieceAt(Position(6, 4)) == nullptr);
    assert(game.getCurrentTurn() == Color::WHITE);
    std::cout << "[PASS] testChessBasicMoves\n";
}

static void testChessRejectsMoveOntoOwnPiece() {
    chess::ChessGame game;
    bool result = game.playTurn(Position(7, 4), Position(6, 4)); // King onto own pawn's square
    assert(!result);
    assert(game.getCurrentTurn() == Color::WHITE); // turn must not advance on an invalid move
    std::cout << "[PASS] testChessRejectsMoveOntoOwnPiece\n";
}

static void testChessFoolsMateCheckmate() {
    // Classic 2-move checkmate: proves check/checkmate detection works,
    // not just piece movement.
    chess::ChessGame game;
    assert(game.playTurn(Position(6, 5), Position(5, 5))); // f2-f3
    assert(game.playTurn(Position(1, 4), Position(3, 4))); // e7-e5
    assert(game.playTurn(Position(6, 6), Position(4, 6))); // g2-g4
    assert(game.playTurn(Position(0, 3), Position(4, 7))); // Qd8-h4#
    assert(game.isGameOver());
    std::cout << "[PASS] testChessFoolsMateCheckmate\n";
}

static void testChessCastling() {
    // Clear the pieces between king and rook, then verify O-O is offered
    // and correctly relocates both king and rook.
    chess::ChessGame game;
    assert(game.playTurn(Position(6, 4), Position(4, 4))); // e2-e4
    assert(game.playTurn(Position(1, 4), Position(3, 4))); // e7-e5
    assert(game.playTurn(Position(7, 6), Position(5, 5))); // Ng1-f3
    assert(game.playTurn(Position(0, 6), Position(2, 5))); // Ng8-f6
    assert(game.playTurn(Position(7, 5), Position(4, 2))); // Bf1-c4
    assert(game.playTurn(Position(0, 5), Position(3, 2))); // Bf8-c5
    assert(game.playTurn(Position(7, 4), Position(7, 6))); // O-O (White kingside)

    assert(game.getBoard().getPieceAt(Position(7, 6)) != nullptr); // king landed
    assert(game.getBoard().getPieceAt(Position(7, 5)) != nullptr); // rook landed
    assert(game.getBoard().getPieceAt(Position(7, 4)) == nullptr); // king's old square empty
    assert(game.getBoard().getPieceAt(Position(7, 7)) == nullptr); // rook's old square empty
    std::cout << "[PASS] testChessCastling\n";
}

static void testChessUndo() {
    chess::ChessGame game;
    assert(game.playTurn(Position(6, 4), Position(4, 4))); // e2-e4
    assert(game.getBoard().getPieceAt(Position(4, 4)) != nullptr);
    assert(game.undoLastMove());
    assert(game.getBoard().getPieceAt(Position(4, 4)) == nullptr);
    assert(game.getBoard().getPieceAt(Position(6, 4)) != nullptr);
    assert(game.getCurrentTurn() == Color::WHITE);
    std::cout << "[PASS] testChessUndo\n";
}

static void testChessEnPassantCapture() {
    chess::ChessGame game;
    assert(game.playTurn(Position(6, 4), Position(4, 4))); // e2-e4 (double step)
    assert(game.playTurn(Position(1, 0), Position(2, 0))); // a7-a6 (filler)
    assert(game.playTurn(Position(4, 4), Position(3, 4))); // e4-e5
    assert(game.playTurn(Position(1, 3), Position(3, 3))); // d7-d5 (double step, lands beside White pawn)

    assert(game.playTurn(Position(3, 4), Position(2, 3))); // e5xd6 en passant
    assert(game.getBoard().getPieceAt(Position(3, 3)) == nullptr); // captured pawn removed
    assert(game.getBoard().getPieceAt(Position(2, 3)) != nullptr); // capturing pawn landed
    assert(game.getBoard().getPieceAt(Position(3, 4)) == nullptr); // old square empty

    std::cout << "[PASS] testChessEnPassantCapture\n";
}

static void testChessEnPassantExpires() {
    chess::ChessGame game;
    assert(game.playTurn(Position(6, 4), Position(4, 4))); // e2-e4
    assert(game.playTurn(Position(1, 0), Position(2, 0))); // a7-a6
    assert(game.playTurn(Position(4, 4), Position(3, 4))); // e4-e5
    assert(game.playTurn(Position(1, 3), Position(3, 3))); // d7-d5, opens the window

    // White doesn't take it immediately -- the window must close.
    assert(game.playTurn(Position(6, 0), Position(5, 0))); // a2-a3
    assert(game.playTurn(Position(2, 0), Position(3, 0))); // a6-a5

    bool lateEnPassant = game.playTurn(Position(3, 4), Position(2, 3));
    assert(!lateEnPassant);

    std::cout << "[PASS] testChessEnPassantExpires\n";
}

static void testCheckersCaptureMechanics() {
    // Unit-test the capture mechanics directly via Board + Piece +
    // MoveCommand, bypassing full-game setup for a precise scenario.
    Board board(8, 8);
    checkers::CheckersPieceFactory factory;

    board.placePiece(factory.createPiece('m', Color::WHITE, Position(4, 3)));
    board.placePiece(factory.createPiece('m', Color::BLACK, Position(3, 4)));
    assert(board.isEmpty(Position(2, 5)));

    Piece* whiteMan = board.getPieceAt(Position(4, 3));
    auto moves = whiteMan->getValidMoves(board);
    bool foundCapture = false;
    for (auto& m : moves)
        if (m.isCapture && m.to == Position(2, 5)) foundCapture = true;
    assert(foundCapture);

    Move captureMove(Position(4, 3), Position(2, 5));
    captureMove.isCapture = true;
    captureMove.capturedPos = Position(3, 4);

    MoveCommand cmd(board, captureMove);
    cmd.execute();
    assert(board.getPieceAt(Position(3, 4)) == nullptr); // captured piece removed
    assert(board.getPieceAt(Position(2, 5)) != nullptr);  // landed

    cmd.undo();
    assert(board.getPieceAt(Position(3, 4)) != nullptr);  // capture reversed
    assert(board.getPieceAt(Position(4, 3)) != nullptr);
    assert(board.getPieceAt(Position(2, 5)) == nullptr);

    std::cout << "[PASS] testCheckersCaptureMechanics\n";
}

static void testCheckersForcedCaptureRule() {
    checkers::CheckersGame game;

    assert(game.playTurn(Position(5, 0), Position(4, 1))); // White
    assert(game.playTurn(Position(2, 1), Position(3, 0))); // Black
    assert(game.playTurn(Position(5, 2), Position(4, 3))); // White

    // Black now has exactly one legal move: the forced capture.
    assert(game.playTurn(Position(3, 0), Position(5, 2))); // Black captures White's man
    assert(game.getBoard().getPieceAt(Position(4, 1)) == nullptr); // captured
    assert(game.getCurrentTurn() == Color::WHITE);

    // White is now forced to recapture -- a quiet move must be rejected
    // even though it would otherwise be legal.
    bool quietMoveAccepted = game.playTurn(Position(5, 4), Position(4, 5));
    assert(!quietMoveAccepted);
    assert(game.getCurrentTurn() == Color::WHITE);

    // The mandatory recapture must be accepted.
    assert(game.playTurn(Position(6, 3), Position(4, 1)));
    assert(game.getBoard().getPieceAt(Position(5, 2)) == nullptr); // black man captured
    assert(game.getBoard().getPieceAt(Position(4, 1)) != nullptr); // landed
    assert(game.getCurrentTurn() == Color::BLACK); // no further jump, turn passes

    std::cout << "[PASS] testCheckersForcedCaptureRule\n";
}

int main() {
    testChessBasicMoves();
    testChessRejectsMoveOntoOwnPiece();
    testChessFoolsMateCheckmate();
    testChessCastling();
    testChessUndo();
    testChessEnPassantCapture();
    testChessEnPassantExpires();
    testCheckersCaptureMechanics();
    testCheckersForcedCaptureRule();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
