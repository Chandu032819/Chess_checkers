cmake_minimum_required(VERSION 3.10)
project(ChessCheckersEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(game_engine
    src/engine/Board.cpp
    src/engine/Command.cpp
    src/engine/Game.cpp
    src/chess/ChessPieces.cpp
    src/chess/ChessGame.cpp
    src/checkers/CheckersPieces.cpp
    src/checkers/CheckersGame.cpp
)

add_executable(play src/main.cpp)
target_link_libraries(play game_engine)

add_executable(run_tests tests/EngineTests.cpp)
target_link_libraries(run_tests game_engine)
