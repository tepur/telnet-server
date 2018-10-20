#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>

#include "server.h"
#include "menu.h"

void print(MenuManager* manager, const std::string& text) {
    manager->set_text_under_menu(text);
}
void go_to_menu(MenuManager* manager, Menu *menu) {
    manager->set_current_menu(menu);
}
void end_user(MenuManager* manager) {
    manager->end();
}

void init_menus(Menu& main_menu, Menu& menu_B) {
    main_menu.add_option(MenuOption("Opcja A", bind(print, std::placeholders::_1, "A")));
    main_menu.add_option(MenuOption("Opcja B", bind(go_to_menu, std::placeholders::_1, &menu_B)));
    main_menu.add_option(MenuOption("Koniec", end_user));

    menu_B.add_option(MenuOption("Opcja B1", bind(print, std::placeholders::_1, "B1")));
    menu_B.add_option(MenuOption("Opcja B2", bind(print, std::placeholders::_1, "B2")));
    menu_B.add_option(MenuOption("Wstecz", bind(go_to_menu, std::placeholders::_1, &main_menu)));
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }

    uint16_t port_number;
    try {
        port_number = Server::c_str_to_uint16_t(argv[1]);
    } catch(std::exception& exc) {
        std::cerr << "Wrong port number" << std::endl;
        return 1;
    }

    Server server;

    try {
        server.init_server(port_number);
    } catch (ServerException& exc) {
        return 1;
    }

    Menu main_menu, menu_B;
    MenuManager manager(&main_menu);
    init_menus(main_menu, menu_B);

    std::string menu_render;

    while(true) {
        try {
            server.accept_new_client();
            server.set_client_telnet();

            do {
                menu_render.clear();
                manager.render_to_string(menu_render);
                server.write_message(menu_render);

                std::string message = server.read_message();
                if(server.is_telnet_negotiation(message)) {
                    server.accept_telnet_negotiation(message);
                }
                else {
                    manager.handle_input(message);
                }
            } while (server.is_client_connected() && manager.is_alive());

            server.disconnect_client();
        } catch (ServerException& exc) {
            return 1;
        } catch (ProblemWithUserException& exc) {
            try {
                server.disconnect_client();
            } catch(ServerException& exc) {
                return 1;
            }
        }
        main_menu.reset();
        menu_B.reset();
        manager.reset(&main_menu);
    }

    return 0;
}