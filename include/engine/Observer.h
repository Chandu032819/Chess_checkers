#pragma once
#include <string>
#include "Move.h"
#include "Color.h"

namespace engine {

class Board;

// Observer pattern: the engine announces what happened; it never knows
// or cares how that gets rendered. A console UI, a future GUI (SFML/Qt),
// a move-logger, or a network broadcaster can all subscribe without a
// single line of engine or game code changing.
class GameObserver {
public:
    virtual ~GameObserver() = default;
    virtual void onMoveMade(const Move& /*move*/, const Board& /*board*/) {}
    virtual void onInvalidMove(const Move& /*attempted*/) {}
    virtual void onGameEnd(const std::string& /*result*/) {}
};

} // namespace engine
