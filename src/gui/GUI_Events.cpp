// Anksilae@gmail.com
// GUI_Events.cpp
#include "GUI.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Exceptions.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Judge.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Merchant.hpp"

namespace coup
{

    void GUI::handleSetupInput(const sf::Event &event)
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

    bool GUI::handleGlobalButtons(const sf::Vector2i &mouse)
    {
        if (persistent_new_game_button.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
        {
            game.reset();
            name_input.clear();
            selected_role.clear();
            error_message.clear();
            info_message.clear();
            pending_target_action = PendingTargetAction::None;
            state = GUIState::Setup;
            return true;
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
            return true;
        }

        return false;
    }

    bool GUI::handleSpecialButtonClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current)
    {
        for (const auto &[bounds, target_name, role] : special_buttons_positions)
        {
            if (bounds.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
            {
                try
                {

                    auto target = game.get_player_by_name(target_name);
                    int original_coins = current->coins();
                   
                    if (role == "Governor")
                    {
                         if (target_name == current->get_name())
                    {
                        target->ensure_coup_required();
                    }
                        auto *gov_real = dynamic_cast<Governor *>(target.get());
                        if (!gov_real)
                            throw std::runtime_error("Player is not a Governor");
                        gov_real->set_coins(original_coins);
                        std::vector<std::shared_ptr<Player>> tax_targets;
                        const auto &last_actions = game.get_last_actions();

                        for (const auto &[player_name, action] : last_actions)
                        {
                            if (action == "tax" && game.can_still_undo(player_name) && player_name != target_name)
                            {
                                auto p = game.get_player_by_name(player_name);
                                if (p && p->is_active())
                                    tax_targets.push_back(p);
                            }
                        }

                        if (!tax_targets.empty())
                        {
                            auto selected = show_selection_popup(tax_targets, "Choose Player to undo tax for:", sf::Color(70, 70, 200));
                            if (selected)
                                gov_real->undo_tax(*selected);
                            else
                                info_message = "No target selected.";
                        }
                        else
                            error_message = "No tax targets available.";
                    }

                    else if (role == "Judge")
                    {
                         if (target_name == current->get_name())
                    {
                        target->ensure_coup_required();
                    }
                        auto *judge_real = dynamic_cast<Judge *>(target.get());
                        if (!judge_real)
                            throw std::runtime_error("Player is not a Judge");
                        judge_real->set_coins(original_coins);
                        auto turn_player_ptr = game.get_player_by_name(game.turn());
                        judge_real->undo_bribe(*turn_player_ptr);
                        current->set_coins(judge_real->coins());
                    }

                    else if (role == "General")
                    {
                         if (target_name == current->get_name())
                    {
                        target->ensure_coup_required();
                    }
                        auto *general_real = dynamic_cast<General *>(target.get());
                        if (!general_real)
                            throw std::runtime_error("Player is not a General");
                        general_real->set_coins(original_coins);
                        std::vector<std::shared_ptr<Player>> coup_targets;

                        for (const auto &[attacker, victim] : game.get_coup_pending_list())
                        {
                            auto p = game.get_player_by_name(victim);
                            if (p && !p->is_active() && game.can_still_undo(attacker))
                                coup_targets.push_back(p);
                        }

                        if (!coup_targets.empty())
                        {
                            auto selected = show_selection_popup(coup_targets, "Choose Player to revive from coup", sf::Color(180, 50, 50));
                            if (selected)
                                general_real->undo_coup(*selected);
                            else
                                info_message = "No target selected.";
                        }
                        else
                            error_message = "No coup targets available.";
                    }

                    else if (role == "Spy")
                    {
                         if (target_name == current->get_name())
                    {
                        target->ensure_coup_required();
                    }
                        auto *spy_real = dynamic_cast<Spy *>(target.get());
                        if (!spy_real)
                            throw std::runtime_error("Player is not a Spy");
                        spy_real->set_coins(original_coins);
                        std::vector<std::shared_ptr<Player>> targets;
                        for (const auto &name : game.players())
                        {
                            if (name != target_name)
                                targets.push_back(game.get_player_by_name(name));
                        }

                        auto selected = show_selection_popup(targets, "Choose Player to Peek&Disable arrest for", sf::Color(70, 70, 200));
                        if (selected)
                        {
                            spy_real->peek_and_disable(*selected);
                            show_peek_result_popup(selected->role(), selected->coins());
                        }
                        else
                            info_message = "No target selected.";
                    }

                    info_message = game.get_last_action();
                }
                catch (const std::exception &e)
                {
                    handle_gui_exception(e);
                    info_message.clear();
                }

                return true;
            }
        }
        return false;
    }

    bool GUI::handleTargetActionClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current)
    {
        if (pending_target_action == PendingTargetAction::None)
            return false;

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
                    pending_target_action = PendingTargetAction::None;
                    return true;
                }
                catch (const std::exception &e)
                {
                    std::string msg = e.what();
                    std::cerr << "[GUI Exception] " << msg << std::endl;

                    // ✅ אם זו חריגת Coup (האם צריך לבצע Coup)
                    if (msg.find("must perform a coup") != std::string::npos)
                    {
                        error_message = msg;
                        pending_target_action = PendingTargetAction::Coup;
                        info_message = "Choose a player to coup:";

                        // ⬅️ הכנס את כפתורי היעדים באופן ישיר
                        current_target_names.clear();
                        target_button_bounds.clear();
                        for (const auto &name : game.players())
                        {
                            if (name != current->get_name())
                            {
                                current_target_names.push_back(name); // הוסף את השחקנים המועמדים ל־Coup
                            }
                        }

                        // ⬇️ עכשיו צייר את כפתורי היעדים
                        drawActionButtons(current); // צייר את הכפתורים עם היעדים
                        render();                   // רנדר את המסך מחדש
                        return true;                // אל תחזור מיד מ־catch, כי התפריט לא ייפתח אחרת
                    }
                    else
                    {
                        error_message = msg;
                        info_message = (pending_target_action == PendingTargetAction::Arrest)
                                           ? "Choose a player to arrest:"
                                       : (pending_target_action == PendingTargetAction::Sanction)
                                           ? "Choose a player to sanction:": "Choose a player to coup:";
                    }

                    // ❌ שים לב, רק אם לא הייתה חריגה של Coup תחזור ותחזיר false
                    return false;
                }
            }
        }
        return false;
    }
    bool GUI::handleBasicActionClick(const sf::Vector2i &mouse, std::shared_ptr<Player> current)
    {
        for (const auto &pair : action_buttons_bounds)
        {
            if (pair.first.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)))
            {
                try
                {
                    if (pair.second == "Gather")
                    {
                        current->gather();
                    }
                    else if (pair.second == "Tax")
                    {
                        current->tax();
                    }
                    else if (pair.second == "Bribe")
                    {
                        current->bribe();
                    }
                    else if (pair.second == "Invest")
                    {
                        auto *baron = dynamic_cast<Baron *>(current.get());
                        if (!baron)
                            throw InvalidActionException("Only a Baron can use Invest.");
                        baron->invest();
                    }
                    else if (pair.second == "Skip Turn")
                    {
                        current->skip_turn();
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

                    // רק אם זו לא פעולה שדורשת מטרה
                    if (pair.second != "Coup" && pair.second != "Sanction" && pair.second != "Arrest" && pair.second != "Skip Turn")
                    {
                        info_message = game.get_last_action();
                        render();
                    }
                }
                catch (const std::exception &e)
                {
                    handle_gui_exception(e);
                    info_message.clear();
                }

                return true;
            }
        }
        return false;
    }

} // namespace coup
