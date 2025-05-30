# Coup - Advanced C++ Game Simulation

### Author: Anksilae@gmail.com  
### Course: Software Programming – C++  
### Project: Ex3 – *Coup* Game Implementation with Roles, GUI and Full Test Coverage  

---

## 📌 Project Description

This project implements a full-featured simulation of the strategic board game **Coup**, written in modern C++ using **OOP principles**, **inheritance**, and **polymorphism**. The game includes both **game logic** and a **graphical user interface** (GUI), as well as **unit testing** and **memory validation via Valgrind**.

All player actions (tax, gather, bribe, arrest, sanction, coup, invest) are implemented according to specified rules, with support for multiple roles and custom exception handling. The GUI is built using SFML and enables interactive gameplay.

Additionally, the game supports out-of-turn actions, such as Spy's peek_and_disable, Governor's undo_tax, Judge's undo_bribe, and General's undo_coup, which can be triggered via dedicated GUI buttons. Each of these actions can be used once per round per associated player.

---

## 📁 Project Structure

```plaintext
.
├── assets/                        # Fonts and resources for GUI
├── build/                         # Output binaries (e.g., test_game, test_player)
├── include/
│   ├── gui/
│   │   └── GUI.hpp                # GUI class definition
│   ├── roles/                     # Header files for all special roles
│   │   ├── Baron.hpp
│   │   ├── General.hpp
│   │   ├── Governor.hpp
│   │   ├── Judge.hpp
│   │   ├── Merchant.hpp
│   │   └── Spy.hpp
│   ├── doctest.h                 # Testing framework
│   ├── Exceptions.hpp           # All game-related exceptions
│   ├── Game.hpp                 # Core game logic interface
│   └── Player.hpp               # Abstract base class for all players
│
├── src/
│   ├── gui/
│   │   ├── GUI.cpp              # Main GUI implementation
│   │   ├── GUI_Draw.cpp         # GUI rendering logic
│   │   ├── GUI_Events.cpp       # Input event handling
│   │   └── GUI_Utils.cpp        # Utility functions for GUI
│   ├── roles/
│   │   ├── Baron.cpp
│   │   ├── General.cpp
│   │   ├── Governor.cpp
│   │   ├── Judge.cpp
│   │   ├── Merchant.cpp
│   │   └── Spy.cpp
│   ├── Game.cpp
│   └── Player.cpp
│
├── tests/
│   ├── test_game.cpp            # Covers Game class logic
│   ├── test_player.cpp          # Covers Player class and behavior
│   └── test_roles.cpp           # Covers all special roles
│
├── Main.cpp                     # GUI entry point
└── Makefile                     # Compilation rules and valgrind target
```

---

## ✅ Features

- **Abstract Player class** with overridable methods
- **6 unique roles**: Governor, Spy, Judge, Baron, General, Merchant
- **Undo mechanisms**: Governor (undo tax), Judge (undo bribe), General (undo coup), Spy (disable arrest)
- **Sanctions and Arrests**: with full restrictions and rule enforcement
- **GUI**: Turn-based, visual role/action selection, SFML-based rendering
- **Test suite** with [doctest](https://github.com/doctest/doctest)
- **Memory-safe**: Fully validated using `valgrind`

---

## 🧪 Testing and Memory Validation

Unit tests are located in the `tests/` folder:

- `test_game.cpp` – covers `Game.cpp` (state transitions, turn logic, coup logic)
- `test_player.cpp` – covers `Player.cpp` and core gameplay actions
- `test_roles.cpp` – tests every special role’s behavior and edge cases

All tests are **fully covered** and **Valgrind-clean**.

### 🔍 Memory Check with Valgrind

The provided `valgrind` target in the Makefile only runs **on test binaries**, not on the GUI, to ensure accurate leak detection.

```make
valgrind: build/test_game build/test_player build/test_roles
	valgrind --leak-check=full --track-origins=yes ./build/test_game
	valgrind --leak-check=full --track-origins=yes ./build/test_player
	valgrind --leak-check=full --track-origins=yes ./build/test_roles
```

> **Note**: We deliberately avoid running Valgrind on the `Main` (GUI) executable because it pulls in system-level graphics and SFML libraries, which introduce unrelated memory traces and false positives. Only the test executables are covered since they include all logic without graphical side effects.

---

## 🛠️ Build & Run

### 🔧 Build

```bash
make          # Builds all sources and test targets
```

### ▶️ Run GUI

```bash
make Main
```

### 🧪 Run Tests

```bash
make test
```

### 🔬 Run Valgrind

```bash
make valgrind
```

---

## 📌 Notes

- All classes are documented with `@brief` using [Doxygen](https://www.doxygen.nl/).
- Code has been checked with `g++ -Wall -Wextra -pedantic -std=c++17`.

---

## 📚 License

This project is provided for academic purposes as part of a university assignment.  
No redistribution or use in commercial products is allowed without explicit permission from the author.

---

## 🧑‍💻 Author

- **Email:** Anksilae@gmail.com