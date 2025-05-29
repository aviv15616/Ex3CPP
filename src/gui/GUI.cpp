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


namespace coup
{

    GUI::GUI(Game &game, bool auto_start) : game(game), window(sf::VideoMode(1024, 720), "Coup Game")
    {
        debug_auto_start = auto_start;
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
    }

    void GUI::run()
    {
        while (window.isOpen())
        {

            handleEvents();
            render();
            if (debug_auto_start && !debug_start_done)
            {
                try
                {
                    game.add_player("a", "Governor");
                    game.add_player("b", "Spy");
                    game.add_player("c", "Judge");
                    game.add_player("d", "Baron");
                    game.add_player("e", "General");
                    game.add_player("f", "Merchant");
                    state = GUIState::Playing;
                    debug_start_done = true;
                    error_message.clear();
                    info_message = "Game auto started with 6 players.";
                }
                catch (const std::exception &e)
                {
                    handle_gui_exception(e);
                }
            }

            sf::sleep(sf::milliseconds(50));
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
                    return;
                }

                if (event.type == sf::Event::MouseButtonPressed)
                {
                    // ✅ פותח תקיעות מהודעת שגיאה
                    error_message.clear();
                }

                // 💡 מסך ההוספה Setup
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
                            }
                        }

                        sf::RectangleShape addBtn = createButton(30, 180, 200, 40, sf::Color(0, 200, 100));
                        if (isMouseOver(addBtn, mouse))
                        {

                            if (!name_input.empty() && !selected_role.empty())
                            {
                                if (tryCreateAndAddPlayer(name_input, selected_role))
                                {
                                    name_input.clear();
                                    selected_role.clear();
                                }
                            }
                        }

                        sf::RectangleShape startBtn = createButton(30, 240, 200, 40, sf::Color(255, 215, 0));
                        if (isMouseOver(startBtn, mouse) && game.players().size() >= 2)
                        {
                            state = GUIState::Playing;
                            error_message.clear();
                            info_message.clear();
                        }
                    }
                }

                // 💡 מצב משחק פעיל Playing
                else if (state == GUIState::Playing && event.type == sf::Event::MouseButtonPressed)
                {
                    sf::Vector2i mouse = sf::Mouse::getPosition(window);

                    // כפתור "משחק חדש" בצד
                    if (persistent_new_game_button.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
                    {
                        game.reset();
                        name_input.clear();
                        selected_role.clear();
                        error_message.clear();
                        info_message.clear();
                        pending_target_action = PendingTargetAction::None;
                        state = GUIState::Setup;

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
                        }
                        else
                        {
                            error_message = "Game is over. Click 'New Game' to start again.";
                        }
                        return;
                    }

                    std::string current_turn = game.turn();
                    auto current = game.get_player_by_name(current_turn);
                    if (!current->has_available_actions())
                    {
                        info_message = "Player " + current_turn + " has no actions available... skipping turn.";
                        error_message.clear();
                        render();                  // מציג את ההודעה לפני השהייה
                        sf::sleep(sf::seconds(10)); // אפשר לקצר ל־1 שנייה כדי לשמור על קצב
                        game.next_turn();          // מעביר את התור לשחקן הבא
                                           // מונע מהקוד להמשיך עם כפתורים
                    }

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
                                    auto *gov_real = dynamic_cast<Governor *>(gov_ptr.get());
                                    if (!gov_real)
                                    {
                                        throw std::runtime_error("Player is not a Governor");
                                    }

                                    gov_real->set_coins(original_coins);

                                    std::vector<std::shared_ptr<coup::Player>> tax_targets;
                                    const auto &last_actions = game.get_last_actions();

                                    for (const auto &entry : last_actions)
                                    {
                                        const std::string &player_name = entry.first;
                                        const std::string &action = entry.second;

                                        if (action == "tax" && game.can_still_undo(player_name) && player_name != target_name)
                                        {
                                            try
                                            {
                                                auto p = game.get_player_by_name(player_name);
                                                if (p && p->is_active())
                                                {
                                                    tax_targets.push_back(p);
                                                }
                                            }
                                            catch (...)
                                            {
                                            }
                                        }
                                    }

                                    if (!tax_targets.empty())
                                    {
                                        auto selected = show_selection_popup(tax_targets, "Choose Player to undo tax for:", sf::Color(70, 70, 200));
                                        if (selected)
                                        {
                                            try
                                            {
                                                gov_real->undo_tax(*selected);
                                            }
                                            catch (const std::exception &e)
                                            {
                                                handle_gui_exception(e);
                                                info_message.clear();
                                            }
                                        }
                                        else
                                        {
                                            info_message = "No target selected.";
                                            pending_target_action = PendingTargetAction::None;
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
                                    auto judge_ptr = game.get_player_by_name(actor_name);
                                    auto *judge_real = dynamic_cast<Judge *>(judge_ptr.get());
                                    if (!judge_real)
                                    {
                                        throw std::runtime_error("Player is not a Judge");
                                    }

                                    judge_real->set_coins(original_coins);

                                    auto turn_player_ptr = game.get_player_by_name(game.turn());

                                    try
                                    {
                                        judge_real->undo_bribe(*turn_player_ptr);
                                    }
                                    catch (const std::exception &e)
                                    {
                                        handle_gui_exception(e);
                                        info_message.clear();
                                    }

                                    current->set_coins(judge_real->coins());
                                }

                                else if (role == "General")
                                {
                                    auto general_ptr = game.get_player_by_name(target_name);
                                    auto *general_real = dynamic_cast<General *>(general_ptr.get());
                                    if (!general_real)
                                    {
                                        throw std::runtime_error("Player is not a General");
                                    }

                                    general_real->set_coins(original_coins);

                                    std::vector<std::shared_ptr<coup::Player>> coup_targets;

                                    for (const auto &entry : game.get_coup_pending_list())
                                    {
                                        const std::string &victim_name = entry.second;

                                        try
                                        {
                                            std::shared_ptr<coup::Player> p = game.get_player_by_name(victim_name);
                                            if (p && !p->is_active() && game.can_still_undo(entry.first))
                                            {
                                                coup_targets.push_back(p);
                                            }
                                        }
                                        catch (...)
                                        {
                                        }
                                    }

                                    if (!coup_targets.empty())
                                    {
                                        auto selected = show_selection_popup(coup_targets, "Choose Player to revive from coup", sf::Color(180, 50, 50));
                                        if (selected)
                                        {
                                            try
                                            {
                                                general_real->undo_coup(*selected);
                                            }
                                            catch (const std::exception &e)
                                            {
                                                handle_gui_exception(e);
                                                info_message.clear();
                                            }
                                        }
                                        else
                                        {
                                            info_message = "No target selected.";
                                            pending_target_action = PendingTargetAction::None;
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
                                    auto spy_ptr = game.get_player_by_name(target_name);
                                    auto *spy_real = dynamic_cast<Spy *>(spy_ptr.get());
                                    if (!spy_real)
                                    {
                                        throw std::runtime_error("Player is not a Spy");
                                    }

                                    spy_real->set_coins(original_coins);

                                    std::vector<std::shared_ptr<coup::Player>> targets;
                                    for (const auto &name : game.players())
                                    {
                                        if (name != target_name)
                                            targets.push_back(game.get_player_by_name(name));
                                    }

                                    auto selected = show_selection_popup(targets, "Choose Player to Peek&Disable arrest for", sf::Color(70, 70, 200));
                                    if (selected)
                                    {
                                        try
                                        {
                                            spy_real->peek_and_disable(*selected);
                                            show_peek_result_popup(selected->role(), selected->coins());
                                        }
                                        catch (const std::exception &e)
                                        {
                                            handle_gui_exception(e);
                                            info_message.clear();
                                        }
                                    }
                                    else
                                    {
                                        info_message = "No target selected.";
                                        pending_target_action = PendingTargetAction::None;
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

                        for (size_t i = 0; i < target_button_bounds.size(); ++i)
                        {
                            if (target_button_bounds[i].contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))

                            {
                                const std::string &selected_name = current_target_names[i];
                                auto target = game.get_player_by_name(selected_name);

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
                                    std::string err = e.what();
                                    handle_gui_exception(e);
                                    info_message.clear();

                                    if (err.find("must perform a coup") != std::string::npos)
                                    {
                                        // העדכון הזה קריטי: יוצרים מחדש את תפריט ה־Coup
                                        pending_target_action = PendingTargetAction::Coup;

                                        current_target_names.clear();
                                        for (const std::string &name : game.players())
                                        {
                                            if (name != game.turn())
                                                current_target_names.push_back(name);
                                        }

                                        error_message = "you must perform a coup, choose target:";
                                        info_message.clear(); // לא מציגים שגיאת coup
                                        return;               // ✅ מונע את reset של pending_target_action בסוף הפונקציה
                                    }
                                }

                                pending_target_action = PendingTargetAction::None;
                                return;
                            }
                        }

                        // לחיצה שלא על אף אחד מהכפתורים
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
                                {
                                    current->gather();
                                    info_message = game.get_last_action();
                                }
                                else if (pair.second == "Tax")
                                {
                                    current->tax();
                                    info_message = game.get_last_action();
                                }
                                else if (pair.second == "Bribe")
                                {
                                    current->bribe();
                                    info_message = game.get_last_action();
                                }
                                else if (pair.second == "Invest")
                                {
                                    if (auto *baron = dynamic_cast<Baron *>(current.get()))
                                    {
                                        baron->invest();
                                        info_message = game.get_last_action();
                                    }
                                    else
                                    {
                                        throw InvalidActionException("Only a Baron can use Invest.");
                                    }
                                }
                                else if (pair.second == "Arrest")
                                {
                                    pending_target_action = PendingTargetAction::Arrest;
                                    info_message = "Choose a player to arrest:";
                                }
                                else if (pair.second == "Sanction")
                                {
                                    pending_target_action = PendingTargetAction::Sanction;
                                    info_message = "Choose a player to sanction:";
                                }
                                else if (pair.second == "Coup")
                                {
                                    pending_target_action = PendingTargetAction::Coup;
                                    info_message = "Choose a player to coup:";
                                }

                                error_message.clear();
                            }
                            catch (const std::exception &e)
                            {
                                std::string err = e.what();
                                handle_gui_exception(e);
                                info_message.clear();

                                if (err.find("must perform a coup") != std::string::npos)
                                {
                                    pending_target_action = PendingTargetAction::Coup;
                                    current_target_names.clear();
                                    for (const std::string &name : game.players())
                                    {
                                        if (name != game.turn())
                                            current_target_names.push_back(name);
                                    }

                                    error_message = "you must perform a coup, choose target:";
                                    info_message.clear();
                                }
                            }

                            return;
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[GUI Exception] " << e.what() << std::endl;
                handle_gui_exception(e);
                pending_target_action = PendingTargetAction::None;
            }
        }
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
    std::shared_ptr<coup::Player> GUI::show_selection_popup(
        const std::vector<std::shared_ptr<coup::Player>> &targets,
        const std::string &title,
        const sf::Color &button_color)
    {
        const int width = 500;
        const int height = 80 + static_cast<int>(targets.size()) * 50;

        sf::RenderWindow popup(sf::VideoMode(width, height), title, sf::Style::Titlebar | sf::Style::Close);
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
            btn.setFillColor(button_color);
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
    void GUI::handle_gui_exception(const std::exception &e)
    {
        error_message = e.what();
        info_message.clear();
        std::cerr << "[GUI Exception] " << e.what() << std::endl;

        // הוספה חשובה: עדכון מצב כדי שהמשתמש יוכל לבצע פעולה מחדש
        pending_target_action = PendingTargetAction::None;

        // נסה להכריח רינדור מחדש אם תקוע
        try
        {
            render();
        }
        catch (const std::exception &re)
        {
            std::cerr << "[Render Exception] " << re.what() << std::endl;
        }
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
        bg.setPosition(WINDOW_WIDTH - 600 - 30, 30);
        bg.setFillColor(sf::Color(30, 30, 30)); // צבע רקע כמו חלון
        window.draw(bg);

        // ואז מצייר טקסט חדש
        drawText("Player: " + p->get_name() + " (" + p->role() + ") - Coins: " + std::to_string(p->coins()), 30, 30);
    }

    void GUI::drawActionButtons(std::shared_ptr<Player> player)
    {
        action_buttons_bounds.clear();

        std::vector<std::string> basic = {"Gather", "Tax", "Bribe"};
        int btn_width = 100;
        int btn_height = 35;
        int spacing = 110;

        int start_x = 30;
        int start_y = 80;

        // כפתורים רגילים (פעולה מיידית)
        for (size_t i = 0; i < basic.size(); ++i)
        {
            int x = start_x + static_cast<int>(i) * spacing;
            sf::RectangleShape btn = createButton(x, start_y, btn_width, btn_height, sf::Color(70, 130, 180));
            btn.setPosition(x, start_y); // ✅ מוודא מיקום
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
            btn.setPosition(x, y); // ✅ קריטי לשמירה מדויקת של הגבולות
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
                int button_width = 140;
                int button_height = 40;
                int margin = 30;

                int btn_x = window.getSize().x - button_width - margin;
                int btn_y = window.getSize().y - button_height - margin;

                // 🧮 מידע על שחקנים חיים
                auto alive_players = game.players();
                int row_height = 26;
                int box_width = 260;
                int padding = 10;
                int list_height = static_cast<int>(alive_players.size()) * row_height + 2 * padding;

                // ✅ מיקום הטבלה - קצת מעל new game, מיושר לימין
                int box_x = btn_x - 110;
                int box_y = btn_y - list_height - 15;
                drawText("Active Players:", box_x, box_y - 30, 18, sf::Color(200, 200, 255));

                // 📦 רקע הטבלה
                sf::RectangleShape bg(sf::Vector2f(box_width, list_height));
                bg.setPosition(box_x, box_y);
                bg.setFillColor(sf::Color(40, 40, 80, 220)); // כחול כהה חצי שקוף
                bg.setOutlineColor(sf::Color::White);
                bg.setOutlineThickness(2);
                window.draw(bg);

                // 🖊️ כיתוב לכל שחקן
                int text_x = box_x + 10;
                int text_y = box_y + padding;
                for (const std::string &name : alive_players)
                {
                    auto p = game.get_player_by_name(name);
                    std::string label = name + " (" + p->role() + ")";
                    drawText(label, text_x, text_y, 16, sf::Color::White);
                    text_y += row_height;
                }

                // 🟩 כפתור new game
                sf::RectangleShape newGameBtn = createButton(
                    btn_x,
                    btn_y,
                    button_width,
                    button_height,
                    sf::Color(100, 200, 100));
                window.draw(newGameBtn);

                // טקסט ממורכז בכפתור
                sf::FloatRect newGameBounds = newGameBtn.getGlobalBounds();
                float center_x = newGameBounds.left + (newGameBounds.width - 80) / 2;
                float center_y = newGameBounds.top + 10;
                drawText("New Game", center_x, center_y, 16, sf::Color::Black);

                persistent_new_game_button = newGameBtn.getGlobalBounds();
            }

            // ציור כפתור New Game
            if (state == GUIState::Setup || game.is_game_over())
            {
                sf::RectangleShape newGameBtn = createButton(250, 300, 300, 50, sf::Color(100, 200, 100));
                window.draw(newGameBtn);
                drawText("Start New Game", 270, 310, 20);
            }

            // ציור הודעות שגיאה או מידע
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
