// Anksilae@gmail.com

#pragma once

#include "../Game.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <optional>


const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 720;

namespace coup {

    // ====== GUI State Enums ======

    enum class GUIState {
        Setup,     // Player setup screen
        Playing    // Active game screen
    };

    enum class PendingTargetAction {
        None,
        Arrest,
        Sanction,
        Coup
    };

    // ====== GUI Class ======

    class GUI {
    public:
        GUI(Game &game, bool auto_start); // Constructor
        void run();                       // Main game loop

    private:
        // ====== References ======
        Game &game;                       // Reference to the game logic

        // ====== SFML Window and Core ======
        sf::RenderWindow window;         // Main render window
        sf::Clock render_clock; 
        sf::Font font;                   // Loaded font for text
        GUIState state = GUIState::Setup; // Current GUI state
        PendingTargetAction pending_target_action = PendingTargetAction::None; // Action waiting for target
        sf::FloatRect persistent_new_game_button; // Location of "New Game" button


        // ====== Flags ======
        bool debug_auto_start = false;   // Auto start for debug/demo
        bool debug_start_done = false;   // Has auto start already occurred

        // ====== Input and State ======
        std::string name_input;          // Input for player name
        std::string selected_role;       // Selected role for new player
        std::string error_message;       // Last error message
        std::string info_message;        // Info message to display
        std::string last_action_log;     // Log of last action (unused)

        sf::RectangleShape input_box;    // Name input field

        // ====== UI Interaction ======
        std::vector<std::pair<sf::FloatRect, std::string>> action_buttons_bounds; // Positions of action buttons
        std::vector<sf::FloatRect> target_button_bounds;                          // Positions of target buttons
        std::vector<std::string> current_target_names;                            // Current targetable player names

        struct SpecialButtonInfo {
            sf::FloatRect bounds;
            std::string player_name;
            std::string role;
        };
        std::vector<SpecialButtonInfo> special_buttons_positions;                 // Info buttons (e.g., undo tax)

        // ====== Rendering ======
        void render();                        // Draw everything to the window
        void drawSetupScreen();              // Draw name/role selection screen
        void drawPlayerPanel(std::shared_ptr<Player> player); // Top-left info of current player
        void drawActionButtons(std::shared_ptr<Player> player); // Action buttons
        void drawSpecialButtonsPanel();      // Right panel with Undo/Spy buttons
        void drawSpecialActionButton(std::shared_ptr<Player> player); // [Unused]
        void drawTurnInfo();                 // Show info/error boxes
        void drawTargetSelectionButtons();   // Show buttons for choosing a target

        // ====== GUI Events and Input ======
        void handleEvents();                 // Handle mouse/keyboard input
        bool tryCreateAndAddPlayer(const std::string &name, const std::string &role); // Try to add a player
        void handle_gui_exception(const std::exception &e); // Display exception in error_message

        // ====== Popups ======
        bool show_yes_no_popup(const std::string &question_text); // Not implemented in GUI.cpp yet
        void show_peek_result_popup(const std::string &role, int coins); // Show result of Spy peek
        std::shared_ptr<Player> show_selection_popup(
            const std::vector<std::shared_ptr<Player>> &targets,
            const std::string &title,
            const sf::Color &button_color); // Show popup to choose player from list

        // ====== Utility Drawing ======
        void drawText(const std::string &str, float x, float y, unsigned size = 20, sf::Color color = sf::Color::White); // Text
        static sf::RectangleShape createButton(float x, float y, float w, float h, const sf::Color &color);              // Button
        static bool isMouseOver(const sf::RectangleShape &rect, sf::Vector2i mousePos);
        void handleSetupInput(const sf::Event &event);
        bool handleGlobalButtons(const sf::Vector2i &mouse);
        bool handleSpecialButtonClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current);
        bool handleTargetActionClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current);
        bool handleBasicActionClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current);
        // Mouse-over test
    };

} // namespace coup
