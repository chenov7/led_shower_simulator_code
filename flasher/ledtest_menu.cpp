#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"
#include "ledtest_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

LEDtestMenu::LEDtestMenu() : 
    SimpleItemValueMenu(make_menu_items(), "TwoLEDDelayedTriggerMenu")
{
    sync_values();
    sync_values2();
}

void LEDtestMenu::sync_values()
{
    unsigned all_gpio = gpio_get_all();
    vdac_    = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_      = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_      = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
}

void LEDtestMenu::sync_values2()
{
    unsigned all_gpio = gpio_get_all();
    vdac_    = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_2     = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_2     = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
}

void LEDtestMenu::setfirstled_rc_value(bool draw) 
{
    rc_to_value_string(menu_items_[MIP_LEDFIRST_ROWCOL].value, ar_, ac_);
    if(draw)draw_item_value(MIP_LEDFIRST_ROWCOL);
}

void LEDtestMenu::setsecled_rc_value(bool draw) 
{
    rc_to_value_string(menu_items_[MIP_LEDSEC_ROWCOL].value, ar_2, ac_2);
    if(draw)draw_item_value(MIP_LEDSEC_ROWCOL);
}

std::vector<SimpleItemValueMenu::MenuItem> LEDtestMenu::make_menu_items() 
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_LEDFIRST_ROWCOL) = {"Cursors    : Change column & row LED1", 3, "A0"}; //working
    menu_items.at(MIP_LEDSEC_ROWCOL) = {"hjkl       : Change column & row LED2", 3, "A0"}; //working
    menu_items.at(MIP_LED1_BRIGHTNESS) = {"</s/>      : Manage LED1 brightness", 4, "off"}; //in progress
    menu_items.at(MIP_LED2_BRIGHTNESS) = {"[/S/]      : Manage LED2 brightness", 4, "off"}; //in progress
    menu_items.at(MIP_DELAY) = {"+/d/-      : Delay between LED1 and LED2", 8, "0"}; //in progress
    menu_items.at(MIP_FREQUENCY) = {"{/f/}      : Manage frequency of flashes", 8, "disable"}; //in progress
    menu_items.at(MIP_START) = {"E          : Enable and desable the flashes", 8, "disable"}; //in progress
    menu_items.at(MIP_EXIT) = {"q          : Exit menu", 0, ""}; //working
    return menu_items;
}

bool LEDtestMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer)
{
    if(process_rc_keys(ar_, ac_, key, key_count)) {
        setfirstled_rc_value(); //set the first led row and column value
        gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
                        (ar_ << ROW_A_BASE_PIN)|(ac_ << COL_A_BASE_PIN)); //color test = passed
        return true;
    }
    if(hjkl_rc_keys(ar_2, ac_2, key, key_count)) {
        setsecled_rc_value(); //set the second led row and column value
        gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
                        (ar_2 << ROW_A_BASE_PIN)|(ac_2 << COL_A_BASE_PIN)); //color test = passed
        return true;
    }

    switch (key) {
        case 'q':
        case 'Q':
            return_code = 0;
            return false;
        default:
            beep();
    }
    return true;
}

bool LEDtestMenu::process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer)
{
    heartbeat_timer_count_ += 1;
    if(heartbeat_timer_count_ == 100) {
        if(controller_is_connected) {
            set_heartbeat(!heartbeat_);
        }
        heartbeat_timer_count_ = 0;
    }
    return true;
}
