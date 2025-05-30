//anksilae@gmail.com
// GUI_Utils.cpp
#include "GUI.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Exceptions.hpp"

namespace coup {

/**
 * @brief Attempts to create and add a player to the game.
 * 
 * Displays error messages if the name is duplicate or other exceptions occur.
 * 
 * @param name The name of the player to add.
 * @param role The role selected for the player.
 * @return true if the player was successfully added.
 * @return false if an error occurred.
 */
bool GUI::tryCreateAndAddPlayer(const std::string &name, const std::string &role) {
    try {
        game.add_player(name, role);
        error_message.clear();
        return true;
    } catch (const DuplicatePlayerNameException &e) {
        error_message = e.what();
    } catch (const std::exception &e) {
        error_message = "Failed to add player: " + std::string(e.what());
    }
    return false;
}

/**
 * @brief Handles exceptions raised during GUI interactions.
 * 
 * Displays error messages and updates GUI state accordingly (e.g., entering coup mode).
 * 
 * @param e The exception that was thrown.
 */
void GUI::handle_gui_exception(const std::exception &e) {
    std::string msg = e.what();
    std::cerr << "[GUI Exception] " << msg << std::endl;
    error_message = msg;
    info_message.clear();
    if (msg.find("must perform a coup") != std::string::npos) {
        pending_target_action = PendingTargetAction::Coup;
    } else {
        pending_target_action = PendingTargetAction::None;
    }

    render();
}

/**
 * @brief Creates a rectangular SFML button with position, size, and color.
 * 
 * @param x The X coordinate of the button.
 * @param y The Y coordinate of the button.
 * @param w The width of the button.
 * @param h The height of the button.
 * @param color The fill color of the button.
 * @return sf::RectangleShape The constructed button.
 */
sf::RectangleShape GUI::createButton(float x, float y, float w, float h, const sf::Color &color) {
    sf::RectangleShape rect(sf::Vector2f(w, h));
    rect.setPosition(x, y);
    rect.setFillColor(color);
    rect.setOutlineColor(sf::Color::White);
    rect.setOutlineThickness(1.5f);
    return rect;
}

/**
 * @brief Checks whether a mouse position is over a given rectangle (button).
 * 
 * @param rect The rectangle to check.
 * @param mousePos The current position of the mouse.
 * @return true if the mouse is over the rectangle.
 * @return false otherwise.
 */
bool GUI::isMouseOver(const sf::RectangleShape &rect, sf::Vector2i mousePos) {
    sf::Vector2f pos = rect.getPosition();
    sf::Vector2f size = rect.getSize();
    return mousePos.x >= pos.x && mousePos.x <= pos.x + size.x &&
           mousePos.y >= pos.y && mousePos.y <= pos.y + size.y;
}

/**
 * @brief Shows a popup window with a list of players to select from.
 * 
 * The popup closes when the user selects a target or closes the window.
 * 
 * @param targets List of player targets to choose from.
 * @param title The window title.
 * @param button_color The color of the selection buttons.
 * @return std::shared_ptr<Player> The selected player, or nullptr if none selected.
 */
std::shared_ptr<coup::Player> GUI::show_selection_popup(
    const std::vector<std::shared_ptr<coup::Player>> &targets,
    const std::string &title,
    const sf::Color &button_color)
{
    std::cout << "[DEBUG] Entered show_selection_popup!" << std::endl;

    const int width = 500;
    const int height = 80 + static_cast<int>(targets.size()) * 50;

    sf::RenderWindow popup(sf::VideoMode(width, height), title, sf::Style::Titlebar | sf::Style::Close);
    sf::Font popup_font;
    if (!popup_font.loadFromFile("assets/OpenSans.ttf"))
    {
        std::cerr << "Failed to load font.\n";
        return nullptr;
    }

    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::Text> labels;

    for (size_t i = 0; i < targets.size(); ++i)
    {
        sf::RectangleShape btn(sf::Vector2f(400, 40));
        btn.setPosition(50, 40 + static_cast<float>(i) * 50);
        btn.setFillColor(button_color);
        buttons.push_back(btn);

        sf::Text txt;
        txt.setFont(popup_font);
        txt.setCharacterSize(18);
        txt.setFillColor(sf::Color::White);
        txt.setString(targets[i]->get_name() + " (" + targets[i]->role() + ")");
        txt.setPosition(btn.getPosition().x + 10, btn.getPosition().y + 7);
        labels.push_back(txt);
    }

    while (popup.isOpen())
    {
        sf::Event event;
        while (popup.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                popup.close();
                return nullptr; // ✅ קריטי כדי למנוע nullptr שימוש שגוי
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mpos(sf::Mouse::getPosition(popup));
                for (size_t i = 0; i < buttons.size(); ++i)
                {
                    if (buttons[i].getGlobalBounds().contains(mpos))
                    {
                        popup.close();
                        return targets[i]; // ✅ יחזור עם הבחירה
                    }
                }
            }
        }

        if (!popup.isOpen())
            break;

        popup.clear(sf::Color(30, 30, 30));
        for (size_t i = 0; i < buttons.size(); ++i)
        {
            popup.draw(buttons[i]);
            popup.draw(labels[i]);
        }
        popup.display();
        sf::sleep(sf::milliseconds(20));
    }

    return nullptr; // ✅ אם יצאנו מהלולאה בלי בחירה
}

/**
 * @brief Displays a popup window showing the result of a Spy's peek action.
 * 
 * Indicates the target's role and coins, and that arrest is disabled.
 * 
 * @param role The role of the peeked player.
 * @param coins The number of coins the player has.
 */
void GUI::show_peek_result_popup(const std::string &role, int coins)
{
    std::cout << "[DEBUG] Entered show_peek_popup!" << std::endl;

    sf::RenderWindow popup(sf::VideoMode(450, 150), "Peek Result", sf::Style::Titlebar | sf::Style::Close);
    sf::Font popup_font;
    if (!popup_font.loadFromFile("assets/OpenSans.ttf"))
    {
        std::cerr << "Failed to load font for peek result.\n";
        return;
    }

    sf::Text message;
    message.setFont(popup_font);
    message.setCharacterSize(20);
    message.setFillColor(sf::Color::White);
    message.setString(role + " has " + std::to_string(coins) +
                      " coins,\nand will not be able to use arrest next turn!");
    message.setPosition(20, 30);

    while (popup.isOpen())
    {
        sf::Event event;
        while (popup.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || event.type == sf::Event::MouseButtonPressed)
            {
                popup.close();
                return;
            }
        }

        popup.clear(sf::Color(40, 40, 40));
        popup.draw(message);
        popup.display();
    }
}

} // namespace coup
