#pragma once

#include <vector>

#include "menu.hpp"

class MainMenu: public SimpleItemValueMenu {
public:
    MainMenu();
    virtual ~MainMenu();
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters,
        absolute_time_t& next_timer) final;
    bool process_timer(bool controller_is_connected, int& return_code,
        absolute_time_t& next_timer) final;

private:
    enum MenuItemPositions {
        MIP_ENGINEERING,
        MIP_DC_RAMP,
        MIP_SPI_TEST,
        MIP_LED_TEST,
        MIP_REBOOT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    static std::vector<MenuItem> make_menu_items();
};
