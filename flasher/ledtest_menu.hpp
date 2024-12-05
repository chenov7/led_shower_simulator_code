#pragma once

#include <vector>
#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"

class LEDtestMenu: public SimpleItemValueMenu {
public:
    LEDtestMenu();
    virtual ~LEDtestMenu() { }
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer) final;
    bool process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer) final;

private:
    enum MenuItemPositions {
        MIP_LEDONE_ROWCOL,
        MIP_LEDTWO_ROWCOL,
        MIP_LEDONE_BRIGHTNESS,
        MIP_LEDTWO_BRIGHTNESS,
        MIP_DELAY,
        MIP_FREQUENCY,
        MIP_FED, //FED = Flash Enable Disable
        //MIP_MAN,
        MIP_EXIT,
        MIP_NUM_ITEMS
    };

    std::vector<MenuItem> make_menu_items();

    void sync_values(); //sync values for first led
    void sync_values2(); //sync values for second led
    void setfirstled_rc_value(bool draw = true);
    void setsecled_rc_value(bool draw = true);
    void set_brightness_firstled_value(bool draw = true);
    void set_brightness_secled_value(bool draw = true);
    void set_frequency_value(bool draw = true);
    void set_delay_value(bool draw = true);
    void set_enable_disable_value(bool draw);

    void enable_flashes();
    void disable_flashes();
    void delay();

    int vdac_ = 0; //vdac_ for first led
    int vdac_2 = 0; //vdac_2 for second led
    int ac_ = 0; //ac_ for first led
    int ar_ = 0; //ar_ for first led
    int ac_2 = 1; //ac_2 for second led (starting at 1 to avoid conflict with first led)
    int ar_2 = 0; //ar_2 for second led
    int frequency_ = 100;
    int delay_ = 0;
    bool enable_disable_ = 0;
    unsigned heartbeat_timer_count_ = 0;
};