#pragma once

namespace engine {

enum class Color { WHITE, BLACK, NONE };

inline Color opposite(Color c) {
    if (c == Color::WHITE) return Color::BLACK;
    if (c == Color::BLACK) return Color::WHITE;
    return Color::NONE;
}

} // namespace engine
