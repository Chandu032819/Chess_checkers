# Turn-Based Strategy Game Engine (Chess + Checkers)

A single, generic game engine that hosts two different games -- Chess
and Checkers -- by sharing all the board, turn, and undo/redo machinery
and letting each game plug in only what actually differs: piece
movement rules and win conditions.

The point of building it this way instead of "just Chess" is that
anyone can hardcode one game's rules. Sharing 80%+ of the engine across
two structurally different games (Chess: sliding pieces, check/mate,
castling, promotion — Checkers: forced captures, mandatory multi-jump,
king-crowning) is what actually demonstrates OOP design rather than a
one-off script.

## Quick start

```bash
# Option A: CMake
mkdir build && cd build && cmake .. && make

# Option B: no CMake needed
./build.sh

./build/play         # interactive demo -- pick Chess or Checkers
./build/run_tests    # automated correctness tests
```

Moves are entered as `from to`, e.g. `e2 e4`. Type `undo` to take back
the last move, `quit` to exit.

## Architecture

```
include/engine/   -- generic engine, knows nothing about chess or checkers
  Position.h        board coordinate
  Color.h           WHITE / BLACK
  Move.h            from/to + optional flags (capture, castle, promotion...)
  Piece.h           abstract base class -- the core polymorphism seam
  Board.h           owns the grid, encapsulates all access to it
  Command.h         MoveCommand -- execute()/undo() a move
  Observer.h        GameObserver interface
  Game.h            Game -- the Template Method (playTurn)

include/chess/    -- plugs chess rules into the engine
  ChessPieces.h     Pawn, Knight, Bishop, Rook, Queen, King, ChessPieceFactory
  ChessGame.h       check/checkmate/stalemate, castling, promotion

include/checkers/ -- plugs checkers rules into the engine
  CheckersPieces.h  Man, CheckersKing, CheckersPieceFactory
  CheckersGame.h    forced capture, multi-jump, king-crowning

include/ConsoleObserver.h  -- text UI, built entirely on GameObserver
src/main.cpp                -- interactive driver
tests/EngineTests.cpp       -- assert-based correctness tests
```

Neither `Board` nor `Game` includes a single `#include` from `chess/`
or `checkers/`. All the sharing happens through the `Piece` interface
and the `Game` hooks below -- that's what makes the engine "generic"
rather than "chess with a checkers mode bolted on".

## Design patterns, and where to find them

**Template Method** (`engine/Game.h`) — `Game::playTurn()` is the one
fixed algorithm every game follows: validate → apply → notify → check
end → switch turn. Concrete games never override `playTurn()` itself;
they customize behavior only through four protected hooks:

| Hook | Default | ChessGame | CheckersGame |
|---|---|---|---|
| `getLegalMoves()` | piece's own moves | + reject moves that leave own king in check | + forced-capture filter |
| `applyMove()` | move/capture via Command | + promotion swap | + king-crowning on back row |
| `checkGameEnd()` | *(pure virtual)* | checkmate / stalemate | no pieces / no moves left |
| `continuesTurn()` | `false` (always alternate) | *(unused)* | mandatory multi-jump |

**Polymorphism / Strategy** (`Piece::getValidMoves`) — every piece
type owns its own movement rule as an override of one virtual method.
Adding a new piece or an entirely new game variant means writing one
new `Piece` subclass; the engine, `Board`, and `Game` don't change.

**Factory** (`ChessPieceFactory`, `CheckersPieceFactory`) — builds a
piece from a single character (`'Q'`, `'m'`, ...). Used both for the
initial board layout and for promotion, so there's one source of truth
for "letter → piece type" instead of two.

**Command** (`MoveCommand`) — every applied move is wrapped in a
command object that knows how to reverse itself (`execute()` /
`undo()`), including the harder cases: captures, castling (extra rook
move), and promotion (piece-type swap). `Game` keeps a stack of these,
which is what gives undo for free.

**Observer** (`GameObserver`, `ConsoleObserver`) — the engine
announces `onMoveMade` / `onInvalidMove` / `onGameEnd`; it has no idea
who's listening. `ConsoleObserver` is the only place that knows how to
print a board. A GUI (SFML/Qt) would be a second `GameObserver`
implementation and nothing else in the codebase would change.

**Prototype** (`Piece::clone()`, `Board::clone()`) — deep-copies a
board so the engine can simulate "what if I made this move" (used by
chess's check detection) without touching the real game state.

## Testing

`tests/EngineTests.cpp` is a small assert-based suite (no external
framework) covering:
- basic piece movement and turn alternation
- rejecting an illegal move (turn must not advance)
- checkmate detection (Fool's Mate, chosen because it's forced and
  short — a good regression check for check/checkmate logic)
- castling (both king and rook relocate correctly)
- undo (board and turn both roll back)
- en passant, including the capture itself and that the window closes
  after one ply if not taken
- checkers capture mechanics at the `Piece`/`Board`/`Command` level
- the forced-capture rule end-to-end through a real game, including a
  verified capture → mandatory recapture sequence

Run with `./build/run_tests`.

## Known limitations (documented on purpose)

Building something and *knowing exactly where you cut corners* is more
useful on a CV than pretending there aren't any:

- **Checkers uses simple mandatory capture, not "must capture the
  most pieces" (majority-capture / international draughts rule).**
  When multiple captures are available, the player may pick any of
  them.
- **Undo doesn't perfectly reverse every edge case** — e.g. a rook's
  `hasMoved` flag isn't rolled back after undoing a castle. `undoLastMove()`
  also doesn't reconstruct multi-move-per-turn state precisely for
  checkers multi-jumps (it flips the turn back after every single
  undo, which is correct for single moves but not for the middle of a
  jump chain).
- **`Board` has one small piece of game-specific state** —
  `specialTargetSquare`, used by Chess for en passant. Every other
  part of `Board`/`Game` is fully generic; this one flag was a
  deliberate compromise rather than adding a `GameContext` parameter
  to the entire `Piece` interface for a single feature only Chess
  uses. It's documented in `Board.h` and Checkers never touches it.

## En passant

Implemented. A pawn's double-step records the square it passed over
on the board (`Board::setSpecialTargetSquare`); the opposing pawn can
capture into that square on its very next move only — `ChessGame::applyMove`
clears the flag before applying whatever move was actually chosen, so
the window closes automatically one ply later. Covered by
`testChessEnPassantCapture` and `testChessEnPassantExpires`.

## Possible extensions

- Swap `ConsoleObserver` for an SFML/Qt `GameObserver` — no engine
  changes required.
- AI opponent: `Board::clone()` already supports simulating moves, so
  a minimax/alpha-beta search can reuse it directly.
- Load custom starting positions from a config file through the
  existing `PieceFactory` interfaces.
- Majority-capture (must-capture-the-most) rule for Checkers.
