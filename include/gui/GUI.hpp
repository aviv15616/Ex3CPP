#pragma once
#include "../Game.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <optional>

namespace coup {

    enum class GUIState {
        Setup,
        Playing
    };

    enum class PendingTargetAction {
        None,
        Arrest,
        Sanction,
        Coup
    };

    class GUI {
    public:
        GUI(Game& game);
        void run();

    private:
        Game& game;
        sf::RenderWindow window;
        sf::Font font;
        GUIState state = GUIState::Setup;
        PendingTargetAction pending_target_action = PendingTargetAction::None;
        std::string last_action_log;
        sf::FloatRect persistent_new_game_button;
        std::vector<std::pair<sf::FloatRect, std::string>> action_buttons_bounds;
        std::vector<std::string> current_target_names;

        




        // רכיבי קלט
        sf::RectangleShape input_box;
        std::string name_input;
        std::string selected_role;
        std::string error_message;
        std::string info_message;


        // פעולות GUI
        void render();
        void handleEvents();
        std::shared_ptr<coup::Player> show_tax_target_selection_popup(const std::vector<std::shared_ptr<coup::Player>> &targets);
        void drawTargetSelectionButtons();
        bool show_yes_no_popup(const std::string &question_text);
        void drawSetupScreen();
        void drawPlayerPanel(std::shared_ptr<Player> player);
        void drawActionButtons(std::shared_ptr<Player> player);
        void drawSpecialActionButton(std::shared_ptr<Player> player);
        void drawTurnInfo();
        void drawSpecialButtonsPanel();
        void show_peek_result_popup(const std::string &role, int coins);
        std::shared_ptr<coup::Player> show_coup_target_selection_popup(const std::vector<std::shared_ptr<coup::Player>> &targets);
        std::shared_ptr<coup::Player> show_peek_target_selection_popup(const std::vector<std::shared_ptr<coup::Player>> &targets);

        // עזר
        void drawText(const std::string& str, float x, float y, unsigned size = 20, sf::Color color = sf::Color::White);
        sf::RectangleShape createButton(float x, float y, float w, float h, const sf::Color& color);
        bool isMouseOver(const sf::RectangleShape& rect, sf::Vector2i mousePos);
        bool tryCreateAndAddPlayer(const std::string& name, const std::string& role);
        void handle_gui_exception(const std::exception& e);
        struct SpecialButtonInfo {
            sf::FloatRect bounds;
            std::string player_name;
            std::string role;
        };

        std::vector<SpecialButtonInfo> special_buttons_positions;


        void handle_gui_info(const std::string &msg);
    };

} // namespace coup
