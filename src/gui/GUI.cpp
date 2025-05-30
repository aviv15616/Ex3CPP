// Anksilae@gmail.com

#include "GUI.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Judge.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Merchant.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

namespace coup
{

    /**
     * @brief Constructs the GUI object, initializes the game window, font, and input box.
     * 
     * @param game Reference to the game object managed by this GUI.
     */
    GUI::GUI(Game &game) : game(game), window(sf::VideoMode(1024, 720), "Coup Game")
    {
        state = GUIState::Setup;

        if (!font.loadFromFile("assets/OpenSans.ttf"))
        {
            std::cerr << "Could not load font" << std::endl;
        }

        input_box.setSize(sf::Vector2f(200, 35));
        input_box.setFillColor(sf::Color(50, 50, 50));
        input_box.setOutlineColor(sf::Color(200, 200, 200));
        input_box.setOutlineThickness(2);
        input_box.setPosition(160, 25);
        sf::sleep(sf::milliseconds(100));
    }

    /**
     * @brief Runs the main GUI loop, handling events and rendering the screen repeatedly while the window is open.
     */
    void GUI::run()
    {
        while (window.isOpen())
        {
            handleEvents();
            render();
        }
    }

    /**
     * @brief Handles input and window events such as closing the window, clicking buttons, or triggering actions.
     */
    void GUI::handleEvents()
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            try
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    return;
                }

                if (event.type == sf::Event::MouseButtonPressed)
                {
                    error_message.clear(); // מנקה שגיאות קודמות בלחיצה
                }

                if (state == GUIState::Setup)
                {
                    handleSetupInput(event);
                }
                else if (state == GUIState::Playing && event.type == sf::Event::MouseButtonPressed)
                {
                    sf::Vector2i mouse = sf::Mouse::getPosition(window);

                    // 🟢 לחצן "New Game" או סיום משחק
                    if (handleGlobalButtons(mouse))
                        return;

                    auto current = game.get_player_by_name(game.turn());

                    // 🟢 כפתורים מיוחדים (Spy, Judge וכו’)
                    if (handleSpecialButtonClick(mouse, current))
                        return;

                    // 🟢 שלב ראשון – לחיצה על יעד
                    bool clicked_target = handleTargetActionClick(mouse, current);

                    // 🟢 שלב שני – לחיצה על פעולה רגילה
                    bool clicked_basic = handleBasicActionClick(mouse, current);

                    // 🟡 שלב שלישי – אם לא נלחץ יעד ולא פעולה, נניח שהוא לחץ על מקום ריק
                    if (pending_target_action != PendingTargetAction::None &&
                        !clicked_target && !clicked_basic)
                    {
                        std::cerr << "[INFO] Target action canceled due to click outside buttons.\n";
                        pending_target_action = PendingTargetAction::None;
                        info_message.clear();
                    }
                }
            }
            catch (const std::exception &e)
            {
                handle_gui_exception(e);
                pending_target_action = PendingTargetAction::None;
            }
        }
    }

    /**
     * @brief Renders all elements of the GUI including setup screen, gameplay UI, player status, 
     * error/info messages, and new game prompts.
     */
    void GUI::render()
    {
        window.clear(sf::Color(30, 30, 30));

        try
        {
            if (state == GUIState::Setup)
            {
                drawSetupScreen();
            }
            else if (state == GUIState::Playing)
            {
                if (game.is_game_over() && pending_target_action == PendingTargetAction::None)
                {
                    int window_width = window.getSize().x;
                    int window_height = window.getSize().y;
                    int button_width = 300;
                    int button_height = 50;

                    // Center the button
                    int button_x = (window_width - button_width) / 2;
                    int button_y = (window_height - button_height) / 2;

                    auto newGameBtn = createButton(button_x, button_y, button_width, button_height, sf::Color(100, 200, 100));
                    window.draw(newGameBtn);
                    window.draw(newGameBtn);
                    sf::FloatRect button_bounds = newGameBtn.getGlobalBounds();
                    float text_x = button_bounds.left + (button_bounds.width - 80) / 2;
                    float text_y = button_bounds.top + (button_bounds.height - 20) / 2;

                    drawText("Start New Game", text_x-35, text_y, 20);
                    persistent_new_game_button = newGameBtn.getGlobalBounds();

                    try
                    {
                        sf::Text winner_text("WINNER IS : " + game.winner() + " GAME OVER !", font, 22);
                        sf::FloatRect winner_text_rect = winner_text.getLocalBounds();
                        float winner_text_x = (window_width - winner_text_rect.width) / 2;
                        float winner_text_y = button_y - 60;

                        drawText("WINNER IS : " + game.winner() + " GAME OVER !", winner_text_x, winner_text_y, 22, sf::Color::Yellow);
                    }
                    catch (const std::exception &e)
                    {
                        sf::Text winner_text("GAME OVER - Error getting winner", font, 22);
                        sf::FloatRect winner_text_rect = winner_text.getLocalBounds();
                        float winner_text_x = (window_width - winner_text_rect.width) / 2;
                        float winner_text_y = button_y - 60;
                        drawText("GAME OVER - Error getting winner", winner_text_x, winner_text_y, 22, sf::Color::Red);
                    }

                    if (!error_message.empty())
                    {
                        sf::RectangleShape box(sf::Vector2f(500, 80));
                        box.setFillColor(sf::Color(80, 0, 0));
                        box.setOutlineColor(sf::Color::Red);
                        box.setOutlineThickness(2);
                        box.setPosition(30, 450);
                        window.draw(box);
                        drawText("Error:", 40, 460, 20, sf::Color::White);
                        drawText(error_message, 40, 490, 18, sf::Color(255, 180, 180));
                    }

                    window.display();
                    return;
                }

                std::string current_turn = game.turn();
                std::shared_ptr<Player> player = game.get_player_by_name(current_turn);

                drawPlayerPanel(player);
                drawActionButtons(player);
                drawSpecialButtonsPanel();
                drawTurnInfo();

                int button_width = 140;
                int button_height = 40;
                int margin = 30;

                int btn_x = window.getSize().x - button_width - margin;
                int btn_y = window.getSize().y - button_height - margin;

                auto alive_players = game.players();
                int row_height = 26;
                int box_width = 260;
                int padding = 10;
                int list_height = static_cast<int>(alive_players.size()) * row_height + 2 * padding;

                int box_x = btn_x - 110;
                int box_y = btn_y - list_height - 15;
                drawText("Active Players:", box_x, box_y - 30, 18, sf::Color(200, 200, 255));

                float legend_x = box_x + 60;
                float legend_y = box_y - 70;
                float rect_size = 10;
                float spacing = 100;

                sf::RectangleShape redBox(sf::Vector2f(rect_size, rect_size));
                redBox.setPosition(legend_x + spacing - 15, legend_y -7);
                redBox.setFillColor(sf::Color::Red);
                window.draw(redBox);
                drawText("= Disabled Arrest", legend_x + spacing, legend_y - 10, 14, sf::Color::White);

                sf::RectangleShape blueBox(sf::Vector2f(rect_size, rect_size));
                blueBox.setPosition(legend_x + spacing - 15, legend_y + 8);
                blueBox.setFillColor(sf::Color::Blue);
                window.draw(blueBox);
                drawText("= Sanctioned", legend_x + spacing, legend_y + 5, 14, sf::Color::White);

                sf::RectangleShape yellowBox(sf::Vector2f(rect_size, rect_size));
                yellowBox.setPosition(legend_x + spacing - 15, legend_y +23);
                yellowBox.setFillColor(sf::Color::Yellow);
                window.draw(yellowBox);
                drawText("= Both", legend_x + spacing, legend_y + 20, 14, sf::Color::White);

                sf::RectangleShape bg(sf::Vector2f(box_width, list_height));
                bg.setPosition(box_x, box_y);
                bg.setFillColor(sf::Color(40, 40, 80, 220));
                bg.setOutlineColor(sf::Color::White);
                bg.setOutlineThickness(2);
                window.draw(bg);

                int text_x = box_x + 10;
                int text_y = box_y + padding;
                for (const std::string &name : alive_players)
                {
                    auto p = game.get_player_by_name(name);
                    std::string label = name + " (" + p->role() + ")";
                    sf::Color color = sf::Color::White;
                    if (p->is_arrest_disabled() && p->is_sanctioned())
                        color = sf::Color::Yellow;
                    else if (p->is_arrest_disabled())
                        color = sf::Color::Red;
                    else if (p->is_sanctioned())
                        color = sf::Color::Blue;

                    drawText(label, text_x, text_y, 16, color);
                    text_y += row_height;
                }

                sf::RectangleShape newGameBtn = createButton(
                    btn_x,
                    btn_y,
                    button_width,
                    button_height,
                    sf::Color(100, 200, 100));
                window.draw(newGameBtn);

                sf::FloatRect newGameBounds = newGameBtn.getGlobalBounds();
                float center_x = newGameBounds.left + (newGameBounds.width - 80) / 2;
                float center_y = newGameBounds.top + 10;
                drawText("New Game", center_x, center_y, 16, sf::Color::Black);

                persistent_new_game_button = newGameBtn.getGlobalBounds();
            }

            drawTurnInfo();
        }
        catch (const std::exception &e)
        {
            error_message = std::string("GUI internal error") + e.what();
            sf::RectangleShape box(sf::Vector2f(500, 80));
            box.setFillColor(sf::Color(80, 0, 0));
            box.setOutlineColor(sf::Color::Red);
            box.setOutlineThickness(2);
            box.setPosition(30, 450);
            window.draw(box);
            drawText("Error:", 40, 460, 20, sf::Color::White);
            drawText(error_message, 40, 490, 18, sf::Color(255, 180, 180));
        }

        window.display();
    }

} // namespace coup
