CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g

# נתיבי include
INCLUDES = -Iinclude -Iinclude/gui -Iinclude/roles

# קבצי ליבה
SRC_CORE = src/Game.cpp src/Player.cpp

# קבצי תפקידים
SRC_ROLES = \
	src/roles/Governor.cpp \
	src/roles/Spy.cpp \
	src/roles/Baron.cpp \
	src/roles/General.cpp \
	src/roles/Judge.cpp \
	src/roles/Merchant.cpp

# קבצי GUI
SRC_GUI = src/gui/GUI.cpp

# קובץ main
MAIN = Main.cpp

# יעד בינארי רגיל
TARGET = build/Main

# יעד ברירת מחדל
Main: $(TARGET)
	./$(TARGET)

# קומפילציה רגילה
$(TARGET): $(SRC_CORE) $(SRC_ROLES) $(SRC_GUI) $(MAIN)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ -lsfml-graphics -lsfml-window -lsfml-system

# === יעד בדיקות ===
TEST = tests/all_tests.cpp
TEST_TARGET = build/all_tests

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(SRC_CORE) $(SRC_ROLES) $(TEST)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# ניקוי
clean:
	rm -rf build/*
