#include "menu.h"

MenuOption::MenuOption(std::string text, std::function<void(MenuManager *)> execute)
        : _text(std::move(text)), _execute(std::move(execute)) {}

void MenuOption::render_to_string(std::string &str) {
    str += _text;
}

void MenuOption::render_to_string_selected(std::string &str) {
    str += MenuRenderCodes::BEGIN_UNDERLINE_CODE;
    render_to_string(str);
    str += MenuRenderCodes::END_UNDERLINE_CODE;
}

void MenuOption::select(MenuManager *manager) {
    _execute(manager);
}

void Menu::clamped_next(std::vector<MenuOption>::iterator &it, const std::vector<MenuOption> &vec) {
    ++it;
    if(it == vec.end()) {
        --it;
    }
}

void Menu::clamped_prev(std::vector<MenuOption>::iterator &it, const std::vector<MenuOption> &vec) {
    if(it != vec.begin()) {
        --it;
    }
}

Menu::Menu(std::vector<MenuOption> options) : _options(std::move(options)) {
    _current_option = _options.begin();
}

void Menu::add_option(const MenuOption &option) {
    _options.push_back(option);
    _current_option = _options.begin();
}

void Menu::handle_input(const std::string &input, MenuManager *manager) {
    if (input == ARROW_DOWN) {
        clamped_next(_current_option, _options);
    } else if(input == ARROW_UP) {
        clamped_prev(_current_option, _options);
    } else if(input == ENTER) {
        (*_current_option).select(manager);
    } else {
        std::clog << "Wrong input" << std::endl;
    }
}

void Menu::render(std::string &str) {
    for(auto it = _options.begin(); it != _options.end(); ++it) {
        if(it == _current_option) {
            (*it).render_to_string_selected(str);
        } else {
            (*it).render_to_string(str);
        }
        str += MenuRenderCodes::CURSOR_DOWN_TO_COLUMN_1_CODE;
    }
}

void Menu::reset() {
    _current_option = _options.begin();
}

MenuManager::MenuManager(Menu *current_menu) : _current_menu(current_menu), _text_under_menu("") {}

void MenuManager::handle_input(const std::string &input) {
    _current_menu->handle_input(input, this);
}

void MenuManager::set_text_under_menu(const std::string &new_text) {
    _text_under_menu = new_text;
}

void MenuManager::set_current_menu(Menu *menu) {
    _current_menu = menu;
}

void MenuManager::end() {
    _is_alive = false;
}

bool MenuManager::is_alive() {
    return _is_alive;
}

void MenuManager::render_to_string(std::string &str) {
    str += MenuRenderCodes::CLEAR_SCREEN_CODE;
    _current_menu->render(str);
    str += _text_under_menu;
    str += MenuRenderCodes::CURSOR_DOWN_TO_COLUMN_1_CODE;
    _text_under_menu = "";
}

void MenuManager::reset(Menu *start_menu) {
    _is_alive = true;
    set_current_menu(start_menu);
}
