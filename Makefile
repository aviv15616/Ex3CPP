# Anksilae@gmail.com

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g
INCLUDES = -Iinclude -Iinclude/gui -Iinclude/roles

# קבצי מקור
SRC_CORE = src/Game.cpp src/Player.cpp
SRC_ROLES = \
    src/roles/Governor.cpp \
    src/roles/Spy.cpp \
    src/roles/Baron.cpp \
    src/roles/General.cpp \
    src/roles/Judge.cpp \
    src/roles/Merchant.cpp
SRC_GUI = $(wildcard src/gui/*.cpp)

# קובץ main
MAIN = Main.cpp
TARGET = build/Main

# קומפילציה והרצה של המשחק
Main: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC_CORE) $(SRC_ROLES) $(SRC_GUI) $(MAIN)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ -lsfml-graphics -lsfml-window -lsfml-system

# ================
# טסטים (מופרדים)
# ================

build/test_game: $(SRC_CORE) $(SRC_ROLES) tests/test_game.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

build/test_player: $(SRC_CORE) $(SRC_ROLES) tests/test_player.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

build/test_roles: $(SRC_CORE) $(SRC_ROLES) tests/test_roles.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

test_game: build/test_game
	./build/test_game

test_player: build/test_player
	./build/test_player

test_roles: build/test_roles
	./build/test_roles

# ==========
# כל הטסטים
# ==========
test: test_game test_player test_roles

valgrind: build/test_game build/test_player build/test_roles
	valgrind --leak-check=full --track-origins=yes  ./build/test_game
	valgrind --leak-check=full --track-origins=yes  ./build/test_player
	valgrind --leak-check=full --track-origins=yes  ./build/test_roles




# ===============
# ניקוי
# ===============
clean:
	rm -rf build/* *.gcno *.gcda *.gcov
