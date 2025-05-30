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
    Setup,   // Player setup screen
    Playing  // Active game screen
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
    // --- Constructor and Run Loop ---
    GUI(Game &game);     ///< Initializes GUI with reference to game
    void run();          ///< Main event/render loop

private:
    // --- Game & SFML References ---
    Game &game;                            ///< Reference to game logic
    sf::RenderWindow window;              ///< Main SFML window
    sf::Font font;                        ///< Loaded font
    sf::RectangleShape input_box;         ///< Input field rectangle

    // --- Game State Tracking ---
    GUIState state = GUIState::Setup;
    PendingTargetAction pending_target_action = PendingTargetAction::None;
    sf::FloatRect persistent_new_game_button;

    // --- Input State ---
    std::string name_input;               ///< Current name being typed
    std::string selected_role;            ///< Role selected for new player
    std::string error_message;            ///< Last error message to display
    std::string info_message;             ///< Info message to show user

    // --- UI Button Interaction ---
    std::vector<std::pair<sf::FloatRect, std::string>> action_buttons_bounds; // Action buttons with labels
    std::vector<sf::FloatRect> target_button_bounds;                          // Target button hitboxes
    std::vector<std::string> current_target_names;                            // Names of current target candidates

    struct SpecialButtonInfo {
        sf::FloatRect bounds;
        std::string player_name;
        std::string role;
    };
    std::vector<SpecialButtonInfo> special_buttons_positions;                 // e.g., Undo Tax buttons

    // =============================
    // === RENDERING FUNCTIONS ===
    // =============================

    void render();                                             ///< Main rendering logic
    void drawSetupScreen();                                    ///< Render setup screen (name + role)
    void drawPlayerPanel(const std::shared_ptr<Player>);       ///< Info panel for current player
    void drawActionButtons(const std::shared_ptr<Player>);     ///< Draw gather/tax/bribe/etc buttons
    void drawSpecialButtonsPanel();                            ///< Draw spy/governor/judge action buttons
    void drawSpecialActionButton(const std::shared_ptr<Player> player); ///< (Unused but declared)
    void drawTurnInfo();                                       ///< Show info/error boxes
    void drawTargetSelectionButtons();                         ///< Draw buttons for selecting target

    // =============================
    // === EVENT & LOGIC HANDLERS ===
    // =============================

    void handleEvents();                                       ///< Poll and handle all SFML events
    void handleSetupInput(const sf::Event &event);             ///< Handle typing and clicks during setup
    bool handleGlobalButtons(const sf::Vector2i &mouse);       ///< Check global buttons (e.g., New Game)
    bool handleSpecialButtonClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current); ///< Spy/Governor etc
    bool handleTargetActionClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current);  ///< Coup/Arrest/Sanction
    bool handleBasicActionClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current);   ///< Gather/Tax/etc

    // =============================
    // === GAME STATE HELPERS ===
    // =============================

    bool tryCreateAndAddPlayer(const std::string &name, const std::string &role); ///< Attempts to add a new player
    void handle_gui_exception(const std::exception &e);                           ///< Convert exception to message

    // =============================
    // === POPUPS / MODALS ===
    // =============================

    static void show_peek_result_popup(const std::string &role, int coins); ///< Spy peek popup
    static std::shared_ptr<Player> show_selection_popup(
        const std::vector<std::shared_ptr<Player>> &targets,
        const std::string &title,
        const sf::Color &button_color); ///< Show list of player buttons

    // =============================
    // === UTILITY DRAWING ===
    // =============================

    void drawText(const std::string &str, float x, float y, unsigned size = 20, sf::Color color = sf::Color::White); ///< Draw text
    static sf::RectangleShape createButton(float x, float y, float w, float h, const sf::Color &color);             ///< Make button
    static bool isMouseOver(const sf::RectangleShape &rect, sf::Vector2i mousePos);                                 ///< Check hit
};

} // namespace coup
