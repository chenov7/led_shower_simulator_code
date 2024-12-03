#include "flasher.hpp"
#include "menu.hpp"
#include "main_menu.hpp"
#include "keypress_menu.hpp"
#include "engineering_menu.hpp"
#include "dc_ramp_menu.hpp"
#include "spi_test_menu.hpp"
#include "ledtest_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

#define WRITEVAL(x) \
    { \
        char buffer[80]; \
        sprintf(buffer, "%-20s : %d\n\r", #x, x); \
        puts_raw_nonl(buffer); \
    }

std::vector<SimpleItemValueMenu::MenuItem> MainMenu::make_menu_items() {
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ENGINEERING) = {"e       : Engineering menu", 0, ""};
    menu_items.at(MIP_REBOOT)      = {"Ctrl-b  : Reboot flasher (press and hold)", 0, ""};
    menu_items.at(MIP_DC_RAMP)     = {"r       : Ramp menu", 0, ""};
    menu_items.at(MIP_SPI_TEST)    = {"s       : SPI test menu", 0, ""};
    menu_items.at(MIP_LED_TEST)    = {"2       : Two-LED Delayed Trigger menu", 0, ""}; //pos: 5 but non existant before mod
    return menu_items;
}

MainMenu::MainMenu():
    SimpleItemValueMenu(make_menu_items(), std::string("LLR flasher : Main menu (Build ")+BuildDate::latest_build_date+")") 
{
    timer_interval_us_ = 1000000; // 1Hz
}
    
MainMenu::~MainMenu()
{
    // nothing to see here
}

bool MainMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer)
{
    switch(key) {
    case 'E': 
    case 'e': 
        {
            EngineeringMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case 'R': 
    case 'r': 
        {
            DCRampMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case 'S': 
    case 's': 
        {
            SPItestMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case '2':
        {
            LEDtestMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case 11: /* ctrl-K : secret keypress menu */
        {
            KeypressMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case 7: /* ctrl-g : secret display of menu parameters - to remove */
        cls();
        curpos(1,1);
        WRITEVAL(req_h_);
        WRITEVAL(req_w_);
        WRITEVAL(req_pos_);
        WRITEVAL(screen_h_);
        WRITEVAL(screen_w_);
        WRITEVAL(frame_h_);
        WRITEVAL(frame_w_);
        WRITEVAL(frame_r_);
        WRITEVAL(frame_c_);
        WRITEVAL(item_count_);
        WRITEVAL(item_h_);
        WRITEVAL(item_w_);
        WRITEVAL(val_w_);
        WRITEVAL(item_r_);
        WRITEVAL(item_c_);
        WRITEVAL(val_c_);
        WRITEVAL(item_dr_);
        puts("Press ctrl-L to redraw menu...");
        break;

    default:
        if(key_count==1) {
            beep();
        }
    }
    return true;
}

bool MainMenu::process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer)
{
    if(controller_is_connected) {
        set_heartbeat(!heartbeat_);
    }
    return true;
}
