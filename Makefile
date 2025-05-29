# Anksilae@gmail.com

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

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes ./build/Main


test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(SRC_CORE) $(SRC_ROLES) $(TEST)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
# === בדיקת קוד לא בשימוש עם cppcheck ===
check_unused:
	cppcheck --enable=all --inconclusive --std=c++17 --force \
	         -Iinclude -Iinclude/gui -Iinclude/roles \
	         src src/gui src/roles include include/gui include/roles \
	         2> cppcheck_report.txt
	@echo "🔍 דו\"ח cppcheck נשמר בקובץ: cppcheck_report.txt"
# ניקוי
clean:
	rm -rf build/*
