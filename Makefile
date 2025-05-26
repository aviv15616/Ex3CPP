CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g
INCLUDES = -Isrc -Isrc/core -Isrc/players -Iinclude

SRC_CORE = src/core/Game.cpp
SRC_PLAYERS = \
	src/players/Player.cpp \
	src/players/Governor.cpp \
	src/players/Spy.cpp \
	src/players/Baron.cpp \
	src/players/General.cpp \
	src/players/Judge.cpp \
	src/players/Merchant.cpp

DEMO = demo/Demo.cpp

TARGET = build/Main

Main: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC_CORE) $(SRC_PLAYERS) $(DEMO)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -f $(TARGET)
