// Anksilae@gmail.com
// GUI_Draw.cpp

#include "GUI.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>

namespace coup {

void GUI::drawSetupScreen()
    {
        drawText("Enter Name:", 30, 30);
        window.draw(input_box);
        drawText(name_input, input_box.getPosition().x + 5, input_box.getPosition().y + 5);
        drawText("Select Role:", 30, 80);

        std::vector<std::string> roles = {"Governor", "Spy", "Judge", "Baron", "General", "Merchant"};
        for (size_t i = 0; i < roles.size(); ++i)
        {
            sf::Color color = (selected_role == roles[i]) ? sf::Color(72, 118, 255) : sf::Color(100, 149, 237);
            sf::RectangleShape btn = createButton(30 + i * 120, 120, 100, 40, color);
            window.draw(btn);
            drawText(roles[i], 35 + i * 120, 125, 16);
        }

        window.draw(createButton(30, 180, 200, 40, sf::Color(0, 200, 100)));
        drawText("Add Player", 50, 185);

        if (game.players().size() >= 2)
        {
            window.draw(createButton(30, 240, 200, 40, sf::Color(255, 215, 0)));
            drawText("Start Game", 50, 245);
        }

        drawText("Players:", 30, 300);
        auto names = game.players();
        for (size_t i = 0; i < names.size(); ++i)
        {
            auto p = game.get_player_by_name(names[i]);
            drawText("- " + p->get_name() + " (" + p->role() + ")", 50, 330 + i * 25);
        }

        if (!error_message.empty())
        {
            sf::RectangleShape box(sf::Vector2f(500, 80));
            box.setFillColor(sf::Color(80, 0, 0));
            box.setOutlineColor(sf::Color::Red);
            box.setOutlineThickness(2);
            box.setPosition(30, 500);
            window.draw(box);
            drawText("Error:", 40, 510, 20, sf::Color::White);
            drawText(error_message, 40, 540, 18, sf::Color(255, 180, 180));
        }
    }
    

void GUI::drawPlayerPanel(std::shared_ptr<Player> p)
    {
        // מחיקת רקע קודם של הטקסט
        sf::RectangleShape bg(sf::Vector2f(600, 40));
        bg.setPosition(WINDOW_WIDTH - 600 - 30, 30);
        bg.setFillColor(sf::Color(30, 30, 30)); // צבע רקע כמו חלון
        window.draw(bg);

        // ואז מצייר טקסט חדש
        drawText("Player: " + p->get_name() + " (" + p->role() + ") - Coins: " + std::to_string(p->coins()), 30, 30);
    }


  void GUI::drawActionButtons(std::shared_ptr<Player> player)
{
    action_buttons_bounds.clear();

    std::vector<std::string> basic = {"Gather", "Tax", "Bribe", "Skip Turn"}; // ← הוספנו Skip Turn
    int btn_width = 100;
    int btn_height = 35;
    int spacing = 110;

    int start_x = 30;
    int start_y = 80;

    // כפתורים רגילים (פעולה מיידית)
    for (size_t i = 0; i < basic.size(); ++i)
    {
        int x = start_x + static_cast<int>(i) * spacing;
        sf::RectangleShape btn = createButton(x, start_y, btn_width, btn_height,
                                              basic[i] == "Skip Turn" ? sf::Color(100, 100, 255) : sf::Color(70, 130, 180));
        btn.setPosition(x, start_y);
        window.draw(btn);
        drawText(basic[i], x + 10, start_y + 8, 16);
        action_buttons_bounds.push_back({btn.getGlobalBounds(), basic[i]});
    }

    // כפתור Invest עבור Baron
    if (player->role() == "Baron")
    {
        int i = static_cast<int>(basic.size());
        int x = start_x + i * spacing;
        sf::RectangleShape btn = createButton(x, start_y, btn_width, btn_height, sf::Color(255, 180, 90));
        btn.setPosition(x, start_y);
        window.draw(btn);
        drawText("Invest", x + 10, start_y + 8, 16);
        action_buttons_bounds.push_back({btn.getGlobalBounds(), "Invest"});
    }

    // כפתורים של פעולת מטרה (Arrest, Sanction, Coup)
    std::vector<std::string> target = {"Arrest", "Sanction", "Coup"};
    for (size_t i = 0; i < target.size(); ++i)
    {
        int x = start_x + static_cast<int>(i) * spacing;
        int y = start_y + 60;

        sf::RectangleShape btn = createButton(x, y, btn_width, btn_height, sf::Color(200, 120, 80));
        btn.setPosition(x, y);
        window.draw(btn);
        drawText(target[i], x + 10, y + 8, 16);
        action_buttons_bounds.push_back({btn.getGlobalBounds(), target[i]});
    }

    // ציור כפתורי מטרה אם נדרש
    drawTargetSelectionButtons();
}


void GUI::drawSpecialButtonsPanel()
    {
        special_buttons_positions.clear();
        size_t y = 20;
        int padding_x = 8;
        int padding_y = 5;
        int text_size = 14;
        int spacing = 10;
        int button_width = 90;
        int button_height = 28;

        // שלב 1: חישוב רוחב מקסימלי של שם+תפקיד
        int max_label_width = 0;
        for (const auto &p : game.get_all_players_raw())
        {
            std::string label = p->get_name() + " (" + p->role() + ")";
            if ((p->is_active() || p->role() == "General"))
            {
                int label_px = label.size() * 8; // הערכה
                max_label_width = std::max(max_label_width, label_px);
            }
        }

        int box_width = max_label_width + button_width + spacing + 2 * padding_x;
        int box_x = static_cast<int>(window.getSize().x) - box_width - 30;

        // הצגת כותרת מעל המסגרת
        drawText("Out Of Turn Actions", box_x, y, 18, sf::Color(180, 180, 255));
        y += 30;

        // שלב 2: ציור הכפתורים המיוחדים
        for (const auto &p : game.get_all_players_raw())
        {
            std::string name = p->get_name();
            std::string role = p->role();
            std::string label = name + " (" + role + ")";

            bool include = p->is_active();

            // עבור General מת גם ניתן לכלול אם ניתן לבטל עליו coup
            if (!include && role == "General")
            {
                for (const auto &entry : game.get_coup_pending_list())
                {
                    if (entry.second == name && game.can_still_undo(entry.first))
                    {
                        include = true;
                        break;
                    }
                }
            }

            if (!include)
                continue;

            std::string action_text;
            if (role == "Governor")
                action_text = "Undo Tax";
            else if (role == "Spy")
                action_text = "Spy Peek";
            else if (role == "General")
                action_text = "Undo Coup";
            else if (role == "Judge")
                action_text = "Undo Bribe";

            if (!action_text.empty())
            {
                sf::RectangleShape box(sf::Vector2f(box_width, button_height + 2 * padding_y));
                box.setPosition(box_x, y);
                box.setFillColor(sf::Color(30, 30, 30));
                box.setOutlineColor(sf::Color::White);
                box.setOutlineThickness(1.0f);
                window.draw(box);

                drawText(label, box_x + padding_x, y + padding_y, text_size);

                int btn_x = box_x + box_width - button_width - padding_x;
                sf::RectangleShape btn = createButton(btn_x, y + padding_y, button_width, button_height, sf::Color(120, 120, 255));
                window.draw(btn);
                drawText(action_text, btn_x + 5, y + padding_y + 6, 12);

                special_buttons_positions.push_back({btn.getGlobalBounds(), name, role});
                y += button_height + padding_y + 10;
            }
        }
    }
    

  void GUI::drawTargetSelectionButtons()
    {
        if (pending_target_action == PendingTargetAction::None)
            return;

        std::string current_name = game.turn();
        std::vector<std::string> all = game.players();

        current_target_names.clear();
        target_button_bounds.clear(); // ⬅️ חשוב!

        for (const auto &name : all)
        {
            if (name != current_name)
                current_target_names.push_back(name);
        }

        int btn_width = 140;
        int btn_height = 32;
        int start_x = 30;
        int start_y = window.getSize().y - 120;

        drawText("Targets:", start_x, start_y - 25, 16, sf::Color(180, 180, 255));

        for (size_t i = 0; i < current_target_names.size(); ++i)
        {
            const std::string &name = current_target_names[i];
            int x = start_x + static_cast<int>(i) * (btn_width + 12);

            sf::RectangleShape btn = createButton(x, start_y, btn_width, btn_height, sf::Color(160, 80, 80));
            window.draw(btn);

            auto player = game.get_player_by_name(name);
            std::string label = name + " (" + player->role() + ")";
            drawText(label, x + 10, start_y + 8, 14, sf::Color::White);

            target_button_bounds.push_back(btn.getGlobalBounds()); // ⬅️ מוסיפים את המיקום ללחיצה
        }
    }
  

 void GUI::drawTurnInfo()
    {
        float box_x = 30;
        float box_y = 430; // אותו y לשתי ההודעות
        float box_width = 700;
        float box_height = 80;
        float padding_left = 10;
        float padding_top = 10;

        if (!error_message.empty())
        {
            sf::RectangleShape box(sf::Vector2f(box_width, box_height));
            box.setFillColor(sf::Color(80, 0, 0));
            box.setOutlineColor(sf::Color::Red);
            box.setOutlineThickness(2);
            box.setPosition(box_x, box_y);
            window.draw(box);

            drawText("Error:", box_x + padding_left, box_y + padding_top, 20, sf::Color::White);
            drawText(error_message, box_x + padding_left, box_y + padding_top + 30, 18, sf::Color(255, 180, 180));
        }
        else if (!info_message.empty())
        {
            sf::RectangleShape box(sf::Vector2f(box_width, box_height));
            box.setFillColor(sf::Color(80, 80, 0));
            box.setOutlineColor(sf::Color::Yellow);
            box.setOutlineThickness(2);
            box.setPosition(box_x, box_y);
            window.draw(box);

            drawText("Action:", box_x + padding_left, box_y + padding_top, 20, sf::Color::White);
            drawText(info_message, box_x + padding_left, box_y + padding_top + 30, 18, sf::Color(255, 255, 180));
        }
    }


 void GUI::drawText(const std::string &str, float x, float y, unsigned size, sf::Color color)
    {
        sf::Text text(str, font, size);
        text.setPosition(x, y);
        text.setFillColor(color);
        window.draw(text);
    }


} // namespace coup
