#pragma once

namespace engine {

// A single board square. Used by every game the engine hosts -- rows/cols
// are just integers, so an 8x8 chess/checkers board or some other size
// board later on both work without any changes here.
struct Position {
    int row;
    int col;

    Position() : row(-1), col(-1) {}
    Position(int r, int c) : row(r), col(c) {}

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const Position& other) const { return !(*this == other); }
};

} // namespace engine
