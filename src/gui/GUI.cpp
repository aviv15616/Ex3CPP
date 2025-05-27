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
namespace coup
{

    GUI::GUI(Game &game) : game(game), window(sf::VideoMode(800, 600), "Coup Game")
    {
        if (game.players().empty())
        {
            game.add_player("a", "Governor");
            game.add_player("b", "Spy");
            game.add_player("c", "Judge");
            game.add_player("d", "Baron");
            game.add_player("e", "General");
            game.add_player("f", "Merchant");

            state = GUIState::Playing;
        }
        if (!font.loadFromFile("assets/OpenSans.ttf"))
        {
            std::cerr << "Could not load font" << std::endl;
        }

        input_box.setSize(sf::Vector2f(200, 35));
        input_box.setFillColor(sf::Color(50, 50, 50));
        input_box.setOutlineColor(sf::Color(200, 200, 200));
        input_box.setOutlineThickness(2);
        input_box.setPosition(160, 25);
    }

    void GUI::run()
    {
        while (window.isOpen())
        {
            handleEvents();
            render();
        }
    }

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
                }

                if (event.type == sf::Event::MouseButtonPressed)
                {
                    std::cout << "[Event] Mouse Clicked - State=" << (state == GUIState::Setup ? "Setup" : "Playing")
                              << ", pending_target_action=" << static_cast<int>(pending_target_action)
                              << ", game_over=" << game.is_game_over() << std::endl;
                }

                if (state == GUIState::Setup)
                {
                    if (event.type == sf::Event::TextEntered && event.text.unicode < 128)
                    {
                        char entered = static_cast<char>(event.text.unicode);
                        if (std::isalnum(entered) || entered == ' ')
                            name_input += entered;
                        else if (entered == 8 && !name_input.empty())
                            name_input.pop_back();
                    }

                    if (event.type == sf::Event::MouseButtonPressed)
                    {
                        sf::Vector2i mouse = sf::Mouse::getPosition(window);
                        std::vector<std::string> roles = {"Governor", "Spy", "Judge", "Baron", "General", "Merchant"};

                        for (size_t i = 0; i < roles.size(); ++i)
                        {
                            sf::RectangleShape roleBtn = createButton(30 + i * 120, 120, 100, 40, sf::Color(100, 149, 237));
                            if (isMouseOver(roleBtn, mouse))
                            {
                                selected_role = roles[i];
                                error_message.clear();
                                info_message.clear();
                                std::cout << "[Click] Role selected: " << selected_role << std::endl;
                            }
                        }

                        sf::RectangleShape addBtn = createButton(30, 180, 200, 40, sf::Color(0, 200, 100));
                        if (isMouseOver(addBtn, mouse))
                        {
                            std::cout << "[Click] Add Player button" << std::endl;
                            if (!name_input.empty() && !selected_role.empty())
                            {
                                if (tryCreateAndAddPlayer(name_input, selected_role))
                                {
                                    name_input.clear();
                                    selected_role.clear();
                                    std::cout << "[Info] Player added: " << name_input << std::endl;
                                }
                            }
                        }

                        sf::RectangleShape startBtn = createButton(30, 240, 200, 40, sf::Color(255, 215, 0));
                        if (isMouseOver(startBtn, mouse) && game.players().size() >= 2)
                        {
                            state = GUIState::Playing;
                            error_message.clear();
                            info_message.clear();
                            std::cout << "[Click] Start Game" << std::endl;
                        }
                    }
                }

                else if (state == GUIState::Playing && event.type == sf::Event::MouseButtonPressed)
                {
                    sf::Vector2i mouse = sf::Mouse::getPosition(window);

                    // ✅ בדיקת לחיצה על כפתור new game
                    if (persistent_new_game_button.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
                    {
                        game.reset();
                        name_input.clear();
                        selected_role.clear();
                        error_message.clear();
                        info_message.clear();
                        pending_target_action = PendingTargetAction::None;
                        state = GUIState::Setup;
                        std::cout << "[Click] New Game (manual)" << std::endl;
                        return;
                    }

                    if (game.is_game_over())
                    {
                        sf::RectangleShape newGameBtn = createButton(250, 300, 300, 50, sf::Color(100, 200, 100));
                        if (isMouseOver(newGameBtn, mouse))
                        {
                            game.reset();
                            name_input.clear();
                            selected_role.clear();
                            error_message.clear();
                            info_message.clear();
                            pending_target_action = PendingTargetAction::None;
                            state = GUIState::Setup;
                            std::cout << "[Click] New Game" << std::endl;
                        }
                        else
                        {
                            error_message = "Game is over. Click 'New Game' to start again.";
                        }
                        return;
                    }

                    std::string current_turn = game.turn();
                    auto current = game.get_player_by_name(current_turn);

                    // ✅ טיפול בלחיצה על special buttons בצד ימין (Undo / Peek)
                    for (const auto &[bounds, target_name, role] : special_buttons_positions)
                    {
                        if (bounds.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
                        {
                            try
                            {
                                auto target = game.get_player_by_name(target_name); // השחקן שעליו פועלים
                                const std::string &actor_name = target_name;        // ✅ זה השחקן שעל הכפתור שנלחץ
                                int original_coins = current->coins();              // לשמור מטבעות מקוריים

                                if (role == "Governor")
                                {
                                    auto gov_ptr = game.get_player_by_name(target_name);
                                    coup::Governor temp_gov(game, target_name);
                                    temp_gov.set_coins(gov_ptr->coins());

                                    std::vector<std::shared_ptr<coup::Player>> tax_targets;
                                    const auto &last_actions = game.last_actions; // חשוב: נדרשת פונקציה accessor

                                    for (const auto &entry : last_actions)
                                    {
                                        const std::string &player_name = entry.first;
                                        const std::string &action = entry.second;

                                        if (action == "tax")
                                        {
                                            try
                                            {
                                                std::shared_ptr<coup::Player> p = game.get_player_by_name(player_name);
                                                if (p != nullptr && p->is_active())
                                                {
                                                    tax_targets.push_back(p);
                                                }
                                            }
                                            catch (const std::exception &e)
                                            {
                                                std::cerr << "[ERROR] Could not find player: " << e.what() << std::endl;
                                            }
                                        }
                                    }

                                    if (!tax_targets.empty())
                                    {
                                        auto selected = show_tax_target_selection_popup(tax_targets);
                                        if (selected != nullptr)
                                        {
                                            try
                                            {
                                                temp_gov.undo_tax(*selected);
                                                gov_ptr->set_coins(temp_gov.coins());
                                            }
                                            catch (const std::exception &e)
                                            {
                                                handle_gui_exception(e);
                                                info_message.clear();
                                                render(); // ⬅️ מוודא שהGUI מתעדכן מיידית
                                            }
                                        }
                                        else
                                        {
                                            info_message = "No target selected.";
                                        }
                                    }
                                    else
                                    {
                                        error_message = "No tax targets available.";
                                        info_message.clear();
                                    }

                                    info_message = game.get_last_action();
                                }

                                else if (role == "Judge")
                                {
                                    coup::Judge temp(game, actor_name);
                                    temp.set_coins(original_coins);
                                    auto turn_player_ptr = game.get_player_by_name(game.turn());
                                    temp.undo_bribe(*turn_player_ptr);
                                    current->set_coins(temp.coins());
                                }
                                else if (role == "General")
                                {
                                    auto general_ptr = game.get_player_by_name(target_name);
                                    coup::General temp_general(game, target_name);
                                    temp_general.set_coins(original_coins);

                                    std::vector<std::shared_ptr<coup::Player>> coup_targets;

                                    // איסוף כל השחקנים עליהם יש coup ממתין
                                    for (const auto &entry : game.get_coup_pending_list())
                                    {
                                        const std::string &victim_name = entry.second;
                                        try
                                        {
                                            std::shared_ptr<coup::Player> p = game.get_player_by_name(victim_name);
                                            if (p != nullptr && !p->is_active()) // רוצים רק שחקנים מתים שניתן להחיות
                                            {
                                                coup_targets.push_back(p);
                                            }
                                        }
                                        catch (const std::exception &e)
                                        {
                                            std::cerr << "[ERROR] General: could not find coup victim: " << e.what() << std::endl;
                                        }
                                    }

                                    if (!coup_targets.empty())
                                    {
auto selected = show_coup_target_selection_popup(coup_targets);
                                        if (selected != nullptr)
                                        {
                                            try
                                            {
                                                temp_general.undo_coup(*selected);
                                                general_ptr->set_coins(temp_general.coins());
                                            }
                                            catch (const std::exception &e)
                                            {
                                                handle_gui_exception(e);
                                                info_message.clear();
                                                render(); // לעדכון מידי של GUI
                                            }
                                        }
                                        else
                                        {
                                            info_message = "No target selected.";
                                        }
                                    }
                                    else
                                    {
                                        error_message = "No coup targets available.";
                                        info_message.clear();
                                    }

                                    info_message = game.get_last_action();
                                }
                                else if (role == "Spy")
                                {
                                    std::cout << "[DEBUG] SPY button clicked for: " << target_name << std::endl;

                                    auto spy_ptr = game.get_player_by_name(target_name);
                                    coup::Spy temp_spy(game, target_name);
                                    temp_spy.set_coins(spy_ptr->coins()); // ← הכסף של השחקן הנכון

                                    std::vector<std::shared_ptr<coup::Player>> targets;
                                    for (const auto &name : game.players())
                                    {
                                        if (name != target_name) // 🔁 לא מרגל על עצמו
                                            targets.push_back(game.get_player_by_name(name));
                                    }

                                    auto selected = show_peek_target_selection_popup(targets);
                                    if (selected != nullptr)
                                    {
                                        try
                                        {
                                            std::cout << "[DEBUG] Spy " << target_name << " peeks on " << selected->get_name() << "\n";
                                            temp_spy.peek_and_disable(*selected);

                                            // מעדכן את הכסף של השחקן ב־game (ולא current!)
                                            spy_ptr->set_coins(temp_spy.coins());

                                            show_peek_result_popup(selected->role(), selected->coins());
                                        }
                                        catch (const std::exception &e)
                                        {
                                            handle_gui_exception(e);
                                            info_message.clear();
                                            render(); // ⬅️ מוודא שהGUI מתעדכן מיידית
                                        }
                                    }
                                    else
                                    {
                                        info_message = "No target selected.";
                                    }

                                    info_message = game.get_last_action();
                                }
                            }
                            catch (const std::exception &e)
                            {
                                handle_gui_exception(e);
                                info_message.clear();
                            }

                            return;
                        }
                    }

                    // 🎯 פעולת מטרה
                    // 🎯 פעולת מטרה - גרסה מתוקנת
                    // 🎯 פעולת מטרה - גרסה מתוקנת
                    if (pending_target_action != PendingTargetAction::None)
                    {
                        int btn_width = 100;
                        int btn_height = 32;
                        int start_x = 30;
                        int start_y = 390;

                        std::cout << "[DEBUG] Handling click at X=" << mouse.x << ", Y=" << mouse.y << "\n";

                        for (size_t i = 0; i < current_target_names.size(); ++i)
                        {
                            int x = start_x + static_cast<int>(i) * (btn_width + 12);
                            sf::FloatRect btnBounds(x, start_y, btn_width, btn_height);

                            std::cout << " - Button #" << i << ": " << current_target_names[i]
                                      << " Bounds=(" << x << "," << start_y << "," << btn_width << "," << btn_height << ")\n";

                            if (btnBounds.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
                            {
                                const std::string &selected_name = current_target_names[i];
                                auto target = game.get_player_by_name(selected_name);

                                std::cout << "[DEBUG] Selected target: " << selected_name << std::endl;

                                try
                                {
                                    if (pending_target_action == PendingTargetAction::Arrest)
                                        current->arrest(*target);
                                    else if (pending_target_action == PendingTargetAction::Sanction)
                                        current->sanction(*target);
                                    else if (pending_target_action == PendingTargetAction::Coup)
                                        current->coup(*target);

                                    info_message = game.get_last_action();
                                    error_message.clear();
                                }
                                catch (const std::exception &e)
                                {
                                    handle_gui_exception(e);
                                    info_message.clear();
                                }

                                pending_target_action = PendingTargetAction::None;
                                return;
                            }
                        }

                        // לחיצה לא על אף מטרה
                        pending_target_action = PendingTargetAction::None;
                        info_message.clear();
                        return;
                    }

                    // ⚙️ פעולות רגילות
                    // ✅ בדיקה לפי מיקומים שנשמרו ב־drawActionButtons
                    // ⚙️ פעולות רגילות
                    for (const auto &pair : action_buttons_bounds)
                    {
                        if (pair.first.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
                        {
                            try
                            {
                                if (pair.second == "Gather")
                                    current->gather();
                                else if (pair.second == "Tax")
                                    current->tax();
                                else if (pair.second == "Bribe")
                                    current->bribe();
                                else if (pair.second == "Invest")
                                {
                                    if (auto *baron = dynamic_cast<Baron *>(current.get()))
                                    {
                                        baron->invest();
                                    }
                                    else
                                    {
                                        throw InvalidActionException("Only a Baron can use Invest.");
                                    }
                                }
                                else if (pair.second == "Arrest")
                                {
                                    pending_target_action = PendingTargetAction::Arrest;
                                }
                                else if (pair.second == "Sanction")
                                {
                                    pending_target_action = PendingTargetAction::Sanction;
                                }
                                else if (pair.second == "Coup")
                                {
                                    pending_target_action = PendingTargetAction::Coup;
                                }

                                if (pending_target_action != PendingTargetAction::None)
                                {
                                    current_target_names.clear();
                                    for (const std::string &name : game.players())
                                    {
                                        if (name != game.turn())
                                            current_target_names.push_back(name);
                                    }

                                    info_message = "בחר שחקן לביצוע: " + pair.second;
                                }
                                else
                                {
                                    info_message = game.get_last_action();
                                }

                                error_message.clear();
                            }
                            catch (const std::exception &e)
                            {
                                handle_gui_exception(e);
                                info_message.clear();
                            }

                            return; // סיימנו את הטיפול בלחיצה הזו
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                error_message = std::string("Internal error") + e.what();
                std::cerr << "[GUI Exception] " << e.what() << std::endl;
            }
        }
    }
    std::shared_ptr<coup::Player> GUI::show_tax_target_selection_popup(
        const std::vector<std::shared_ptr<coup::Player>> &targets)
    {
        const int width = 500;
        const int height = 80 + static_cast<int>(targets.size()) * 50;

        sf::RenderWindow popup(sf::VideoMode(width, height), "Choose Player to undo tax for:", sf::Style::Titlebar | sf::Style::Close);
        sf::Font font;
        if (!font.loadFromFile("assets/OpenSans.ttf"))
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
            btn.setFillColor(sf::Color(70, 70, 200));
            buttons.push_back(btn);

            sf::Text txt;
            txt.setFont(font);
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
                    return nullptr;
                }

                if (event.type == sf::Event::MouseButtonPressed)
                {
                    sf::Vector2f mpos(sf::Mouse::getPosition(popup));
                    for (size_t i = 0; i < buttons.size(); ++i)
                    {
                        if (buttons[i].getGlobalBounds().contains(mpos))
                        {
                            popup.close();
                            return targets[i];
                        }
                    }
                }
            }

            popup.clear(sf::Color(30, 30, 30));
            for (size_t i = 0; i < buttons.size(); ++i)
            {
                popup.draw(buttons[i]);
                popup.draw(labels[i]);
            }
            popup.display();
        }

        return nullptr;
    }

    std::shared_ptr<coup::Player> GUI::show_peek_target_selection_popup(
        const std::vector<std::shared_ptr<coup::Player>> &targets)
    {
        const int width = 500;
        const int height = 80 + static_cast<int>(targets.size()) * 50;

        sf::RenderWindow popup(sf::VideoMode(width, height), "Choose Player to Peek&Disable arrest for", sf::Style::Titlebar | sf::Style::Close);
        sf::Font font;
        if (!font.loadFromFile("assets/OpenSans.ttf"))
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
            btn.setFillColor(sf::Color(70, 70, 200));
            buttons.push_back(btn);

            sf::Text txt;
            txt.setFont(font);
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
                    return nullptr;
                }

                if (event.type == sf::Event::MouseButtonPressed)
                {
                    sf::Vector2f mpos(sf::Mouse::getPosition(popup));
                    for (size_t i = 0; i < buttons.size(); ++i)
                    {
                        if (buttons[i].getGlobalBounds().contains(mpos))
                        {
                            popup.close();
                            return targets[i];
                        }
                    }
                }
            }

            popup.clear(sf::Color(30, 30, 30));
            for (size_t i = 0; i < buttons.size(); ++i)
            {
                popup.draw(buttons[i]);
                popup.draw(labels[i]);
            }
            popup.display();
        }

        return nullptr;
    }

    void GUI::show_peek_result_popup(const std::string &role, int coins)
    {
        sf::RenderWindow popup(sf::VideoMode(450, 150), "Peek Result", sf::Style::Titlebar | sf::Style::Close);
        sf::Font font;
        if (!font.loadFromFile("assets/OpenSans.ttf"))
        {
            std::cerr << "Failed to load font for peek result.\n";
            return;
        }

        sf::Text message;
        message.setFont(font);
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
std::shared_ptr<coup::Player> GUI::show_coup_target_selection_popup(
    const std::vector<std::shared_ptr<coup::Player>> &targets)
{
    const int width = 500;
    const int height = 80 + static_cast<int>(targets.size()) * 50;

    sf::RenderWindow popup(sf::VideoMode(width, height), "Choose Player to revive from coup", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;
    if (!font.loadFromFile("assets/OpenSans.ttf"))
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
        btn.setFillColor(sf::Color(180, 50, 50));  // צבע שונה לזיהוי
        buttons.push_back(btn);

        sf::Text txt;
        txt.setFont(font);
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
                return nullptr;
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mpos(sf::Mouse::getPosition(popup));
                for (size_t i = 0; i < buttons.size(); ++i)
                {
                    if (buttons[i].getGlobalBounds().contains(mpos))
                    {
                        popup.close();
                        return targets[i];
                    }
                }
            }
        }

        popup.clear(sf::Color(25, 25, 25));
        for (size_t i = 0; i < buttons.size(); ++i)
        {
            popup.draw(buttons[i]);
            popup.draw(labels[i]);
        }
        popup.display();
    }

    return nullptr;
}

    void GUI::drawTargetSelectionButtons()
    {
        if (pending_target_action == PendingTargetAction::None)
            return;

        std::string current_name = game.turn();
        std::vector<std::string> all = game.players();

        current_target_names.clear(); // ⛔️ חשוב: נבנה את הרשימה תוך כדי ציור
        for (const auto &name : all)
        {
            if (name != current_name)
                current_target_names.push_back(name);
        }

        // ✅ קבע רוחב קבוע
        int btn_width = 100;
        int btn_height = 32;
        int start_x = 30;
        int start_y = 390;

        drawText("Targets:", start_x, start_y - 25, 16, sf::Color(180, 180, 255));

        for (size_t i = 0; i < current_target_names.size(); ++i)
        {
            const std::string &name = current_target_names[i];
            int x = start_x + static_cast<int>(i) * (btn_width + 12);

            sf::RectangleShape btn = createButton(x, start_y, btn_width, btn_height, sf::Color(160, 80, 80));
            window.draw(btn);
            drawText(name, x + 10, start_y + 8, 14, sf::Color::White);
        }
    }

    void GUI::handle_gui_exception(const std::exception &e)
    {
        error_message = e.what();
        info_message.clear();
        std::cerr << "[GUI Exception] " << e.what() << std::endl;
    }
    void GUI::handle_gui_info(const std::string &msg)
    {
        info_message = msg;
        error_message.clear();
        std::cout << "[GUI Info] " << msg << std::endl;
        game.log_action("[Info] " + msg);
    }

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
    void GUI::drawText(const std::string &str, float x, float y, unsigned size, sf::Color color)
    {
        sf::Text text(str, font, size);
        text.setPosition(x, y);
        text.setFillColor(color);
        window.draw(text);
    }

    void GUI::drawPlayerPanel(std::shared_ptr<Player> p)
    {
        // מחיקת רקע קודם של הטקסט
        sf::RectangleShape bg(sf::Vector2f(600, 40));
        bg.setPosition(30, 30);
        bg.setFillColor(sf::Color(30, 30, 30)); // צבע רקע כמו חלון
        window.draw(bg);

        // ואז מצייר טקסט חדש
        drawText("Player: " + p->get_name() + " (" + p->role() + ") - Coins: " + std::to_string(p->coins()), 30, 30);
    }

    void GUI::drawActionButtons(std::shared_ptr<Player> player)
    {
        action_buttons_bounds.clear(); // ✅ ננקה את הלחיצות הקודמות

        std::vector<std::string> basic = {"Gather", "Tax", "Bribe"};
        int btn_width = 100;
        int btn_height = 35;
        int spacing = 110;
        int start_x = 30;
        int start_y = 80;

        for (size_t i = 0; i < basic.size(); ++i)
        {
            auto btn = createButton(start_x + i * spacing, start_y, btn_width, btn_height, sf::Color(70, 130, 180));
            window.draw(btn);
            drawText(basic[i], start_x + i * spacing + 10, start_y + 8, 16);
            action_buttons_bounds.push_back({btn.getGlobalBounds(), basic[i]}); // ✅ נשמור את המיקום והפעולה
        }

        if (player->role() == "Baron")
        {
            int i = static_cast<int>(basic.size());
            auto btn = createButton(start_x + i * spacing, start_y, btn_width, btn_height, sf::Color(255, 180, 90));
            window.draw(btn);
            drawText("Invest", start_x + i * spacing + 10, start_y + 8, 16);
            action_buttons_bounds.push_back({btn.getGlobalBounds(), "Invest"});
        }

        std::vector<std::string> target = {"Arrest", "Sanction", "Coup"};
        for (size_t i = 0; i < target.size(); ++i)
        {
            auto btn = createButton(start_x + i * spacing, start_y + 60, btn_width, btn_height, sf::Color(200, 120, 80));
            window.draw(btn);
            drawText(target[i], start_x + i * spacing + 10, start_y + 68, 16);
            action_buttons_bounds.push_back({btn.getGlobalBounds(), target[i]});
        }

        drawTargetSelectionButtons();
    }

    void GUI::drawSpecialButtonsPanel()
    {
        const auto &players = game.players();
        size_t y = 20;
        int padding_x = 8;
        int padding_y = 5;
        int text_size = 14;
        int spacing = 10;
        int button_width = 90;
        int button_height = 28;

        drawText("Out Of Turn Actions", 530, y, 18, sf::Color(180, 180, 255));
        y += 30;

        // 🧮 שלב 1: חישוב רוחב מקסימלי נדרש למסגרת
        int max_label_width = 0;
        for (const std::string &name : players)
        {
            auto p = game.get_player_by_name(name);
            std::string role = p->role();
            std::string label = name + " (" + role + ")";

            if (role == "Governor" || role == "Spy" || role == "General" || role == "Judge")
            {
                int label_px = label.size() * 8; // הערכה פשוטה
                if (label_px > max_label_width)
                {
                    max_label_width = label_px;
                }
            }
        }

        int box_width = max_label_width + button_width + spacing + 2 * padding_x;

        // 🎨 שלב 2: ציור כל השחקנים
        for (const std::string &name : players)
        {
            auto p = game.get_player_by_name(name);
            std::string role = p->role();
            std::string label = name + " (" + role + ")";

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
                int box_x = 530;

                // מסגרת אחידה בגודל
                sf::RectangleShape box(sf::Vector2f(box_width, button_height + 2 * padding_y));
                box.setPosition(box_x, y);
                box.setFillColor(sf::Color(30, 30, 30));
                box.setOutlineColor(sf::Color::White);
                box.setOutlineThickness(1.0f);
                window.draw(box);

                // טקסט בצד שמאל
                drawText(label, box_x + padding_x, y + padding_y, text_size);

                // כפתור בצד ימין של המסגרת
                int btn_x = box_x + box_width - button_width - padding_x;
                sf::RectangleShape btn = createButton(btn_x, y + padding_y, button_width, button_height, sf::Color(120, 120, 255));
                window.draw(btn);
                drawText(action_text, btn_x + 5, y + padding_y + 6, 12);

                special_buttons_positions.push_back({btn.getGlobalBounds(), name, role});
                y += button_height + padding_y + 10;
            }
        }
    }

    void GUI::drawTurnInfo()
    {
        std::string current_turn = game.turn();

        if (!error_message.empty())
        {

            sf::RectangleShape box(sf::Vector2f(600, 80));
            box.setFillColor(sf::Color(80, 0, 0));
            box.setOutlineColor(sf::Color::Red);
            box.setOutlineThickness(2);
            box.setPosition(30, 450);
            window.draw(box);
            drawText("Error:", 40, 460, 20, sf::Color::White);
            drawText(error_message, 40, 490, 18, sf::Color(255, 180, 180));
        }
        else if (!info_message.empty())
        {
            sf::RectangleShape box(sf::Vector2f(500, 80));
            box.setFillColor(sf::Color(80, 80, 0));
            box.setOutlineColor(sf::Color::Yellow);
            box.setOutlineThickness(2);
            box.setPosition(30, 450);
            window.draw(box);
            drawText("Action:", 40, 460, 20, sf::Color::White);
            drawText(info_message, 40, 490, 18, sf::Color(255, 255, 180));
        }
    }

    bool GUI::tryCreateAndAddPlayer(const std::string &name, const std::string &role)
    {
        try
        {
            game.add_player(name, role);
            error_message.clear();
            return true;
        }
        catch (const DuplicatePlayerNameException &e)
        {
            error_message = e.what();
        }
        catch (const std::exception &e)
        {
            error_message = "Failed to add player: " + std::string(e.what());
        }
        return false;
    }

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
                if (game.is_game_over())
                {
                    auto newGameBtn = createButton(250, 300, 300, 50, sf::Color(100, 200, 100));
                    window.draw(newGameBtn);
                    drawText("Start New Game", 270, 310, 20);

                    try
                    {
                        drawText("WINNER IS : " + game.winner() + " GAME OVER !", 180, 240, 22, sf::Color::Yellow);
                    }
                    catch (const std::exception &e)
                    {
                        drawText("GAME OVER - Error getting winner", 180, 240, 22, sf::Color::Red);
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

                // ✅ צייר כפתור new game קבוע
                sf::RectangleShape newGameBtn = createButton(630, 530, 140, 40, sf::Color(100, 200, 100));
                window.draw(newGameBtn);
                drawText("New Game", 650, 540, 16, sf::Color::Black);
                persistent_new_game_button = newGameBtn.getGlobalBounds();
            }
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
    sf::RectangleShape GUI::createButton(float x, float y, float w, float h, const sf::Color &color)
    {
        sf::RectangleShape rect(sf::Vector2f(w, h));
        rect.setPosition(x, y);
        rect.setFillColor(color);
        rect.setOutlineColor(sf::Color::White);
        rect.setOutlineThickness(1.5f);
        return rect;
    }

    bool GUI::isMouseOver(const sf::RectangleShape &rect, sf::Vector2i mousePos)
    {
        sf::Vector2f pos = rect.getPosition();
        sf::Vector2f size = rect.getSize();
        return mousePos.x >= pos.x && mousePos.x <= pos.x + size.x &&
               mousePos.y >= pos.y && mousePos.y <= pos.y + size.y;
    }
}
// namespace coup
