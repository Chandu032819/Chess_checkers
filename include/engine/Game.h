#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Board.h"
#include "Move.h"
#include "Observer.h"
#include "Command.h"

namespace engine {

// Template Method pattern: playTurn() is the ONE fixed algorithm every
// concrete game -- Chess, Checkers, or some future variant -- follows.
// A concrete game customizes behavior ONLY through the four protected
// hooks below; it never overrides playTurn() itself. That's what lets
// ChessGame and CheckersGame share this class instead of each
// reimplementing "validate, apply, notify, check end, switch turn".
class Game {
public:
    explicit Game(std::unique_ptr<Board> board, Color startingTurn = Color::WHITE);
    virtual ~Game() = default;

    // The template method.
    bool playTurn(Position from, Position to, char promotionChoice = 'Q');

    bool undoLastMove();

    // Public read-only wrapper around the legal-move hook. Added for UI
    // consumers (e.g. a GUI highlighting a selected piece's destination
    // squares) that need to query legality without duplicating any rule
    // logic. Delegates entirely to the existing virtual hook -- no new
    // behavior, just visibility.
    std::vector<Move> getLegalMovesFor(Position from) const { return getLegalMoves(from); }

    void addObserver(GameObserver* obs);
    bool isGameOver() const { return gameOver_; }
    Color getCurrentTurn() const { return currentTurn_; }
    const Board& getBoard() const { return *board_; }

protected:
    // Hook 1 -- which moves are actually legal for the piece at `from`?
    // Default: the piece's own pseudo-legal moves, restricted to the
    // player whose turn it is. ChessGame adds a "does this leave my own
    // king in check" filter; CheckersGame adds the forced-capture rule.
    virtual std::vector<Move> getLegalMoves(Position from) const;

    // Hook 2 -- how does a chosen move actually mutate the board?
    // Default: wrap it in a MoveCommand (Command pattern) and push it
    // onto the undo history. ChessGame extends this for promotion;
    // CheckersGame extends it for checkers king-promotion.
    virtual void applyMove(const Move& move);

    // Hook 3 -- has the game ended, and how? Pure virtual: every
    // concrete game MUST define its own win/draw condition.
    virtual bool checkGameEnd() = 0;

    // Hook 4 -- does the SAME player move again? Default: no, always
    // alternate. CheckersGame overrides this for mandatory multi-jump
    // sequences.
    virtual bool continuesTurn(const Move& /*lastMove*/) const { return false; }

    void notifyMoveMade(const Move& move);
    void notifyInvalidMove(const Move& attempted);
    void notifyGameEnd(const std::string& result);

    std::unique_ptr<Board> board_;
    Color currentTurn_;
    bool gameOver_ = false;
    std::string lastResult_;
    std::vector<std::unique_ptr<MoveCommand>> history_;
    std::vector<GameObserver*> observers_;
};

} // namespace engine
