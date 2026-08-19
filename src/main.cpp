#include <iostream>
#include <string>
#include <memory>
#include "engine/Game.h"
#include "chess/ChessGame.h"
#include "checkers/CheckersGame.h"
#include "ConsoleObserver.h"

bool parseSquare(const std::string& s, engine::Position& out) {
    if (s.size() != 2) return false;
    char colC = s[0];
    char rowC = s[1];
    if (colC < 'a' || colC > 'h') return false;
    if (rowC < '1' || rowC > '8') return false;
    out.col = colC - 'a';
    out.row = 8 - (rowC - '0');
    return true;
}

int main() {
    std::cout << "=== Turn-Based Strategy Game Engine ===\n";
    std::cout << "One shared engine, two games plugged in: Chess and Checkers.\n\n";
    std::cout << "Choose a game:\n  1) Chess\n  2) Checkers\n> ";

    int choice = 1;
    std::cin >> choice;

    std::unique_ptr<engine::Game> game;
    if (choice == 2) {
        game = std::make_unique<checkers::CheckersGame>();
        std::cout << "\nStarting Checkers. 'o'/'O' = White man/king, 'x'/'X' = Black man/king.\n";
    } else {
        game = std::make_unique<chess::ChessGame>();
        std::cout << "\nStarting Chess. Uppercase = White, lowercase = Black.\n";
    }

    ConsoleObserver observer;
    game->addObserver(&observer);
    ConsoleObserver::printBoard(game->getBoard());

    std::cout << "Enter moves like: e2 e4   (also: 'undo', 'quit')\n\n";

    std::string fromStr, toStr;
    while (!game->isGameOver()) {
        std::cout << (game->getCurrentTurn() == engine::Color::WHITE ? "White" : "Black")
                   << " to move: ";
        if (!(std::cin >> fromStr)) break;
        if (fromStr == "quit") break;
        if (fromStr == "undo") {
            game->undoLastMove();
            ConsoleObserver::printBoard(game->getBoard());
            continue;
        }
        if (!(std::cin >> toStr)) break;

        engine::Position from, to;
        if (!parseSquare(fromStr, from) || !parseSquare(toStr, to)) {
            std::cout << "Could not parse that. Use squares like e2 e4.\n";
            continue;
        }

        game->playTurn(from, to);
    }

    std::cout << "\nThanks for playing!\n";
    return 0;
}
