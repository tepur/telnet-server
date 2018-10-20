#ifndef SIK1_MENU_H
#define SIK1_MENU_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>

class MenuManager;

class MenuRenderCodes {
public:
    static constexpr const char* CURSOR_DOWN_TO_COLUMN_1_CODE = "\033[E";
    static constexpr const char* BEGIN_UNDERLINE_CODE = "\033[4m";
    static constexpr const char* END_UNDERLINE_CODE = "\033[0m";
    static constexpr const char* CLEAR_SCREEN_CODE = "\033[H\033[J";
};

class MenuOption {
private:
    std::string _text;
    std::function<void(MenuManager*)> _execute;
public:
    MenuOption(std::string text, std::function<void(MenuManager *)> execute);

    void render_to_string(std::string &str);
    void render_to_string_selected(std::string &str);
    void select(MenuManager *manager);
};

class Menu {
    static constexpr const char* ARROW_DOWN = "\x1b[B";
    static constexpr const char* ARROW_UP = "\x1b[A";
    static constexpr const char* ENTER = "\x0d";

    std::vector<MenuOption> _options;
    std::vector<MenuOption>::iterator _current_option;

    static inline void clamped_next(std::vector<MenuOption>::iterator& it, const std::vector<MenuOption>& vec);
    static inline void clamped_prev(std::vector<MenuOption>::iterator& it, const std::vector<MenuOption>& vec);

public:
    explicit Menu(std::vector<MenuOption> options);
    Menu() = default;

    void add_option(const MenuOption& option);

    void handle_input(const std::string& input, MenuManager* manager);

    void render(std::string& str);

    void reset();
};

class MenuManager {
    Menu* _current_menu;
    std::string _text_under_menu;
    bool _is_alive = true;

public:
    explicit MenuManager(Menu* current_menu);;

    void handle_input(const std::string& input);

    void set_text_under_menu(const std::string& new_text);

    void set_current_menu(Menu* menu);

    void end();

    bool is_alive();

    void render_to_string(std::string& str);

    void reset(Menu* start_menu);
};

#endif //SIK1_MENU_H
