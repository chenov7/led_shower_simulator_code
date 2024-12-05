#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"
#include "ledtest_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

LEDtestMenu::LEDtestMenu() :
    SimpleItemValueMenu(make_menu_items(), "Two-LED Delayed Trigger Menu")
{
    sync_values();
    sync_values2();
}

void LEDtestMenu::delay()
{
    sleep_us(1);
}

void LEDtestMenu::enable_flashes()
{
    gpio_put(DAC_EN_PIN, 0);
    gpio_put(DAC_WR_PIN, 0);
    delay();

    gpio_put_masked(0x000003 << DAC_SEL_BASE_PIN, 1 << DAC_SEL_BASE_PIN);
    gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
    delay();
    gpio_put(DAC_WR_PIN, 1);
    delay();
    gpio_put(DAC_WR_PIN, 0);
    delay();

    gpio_put_masked(0x000003 << DAC_SEL_BASE_PIN, 3 << DAC_SEL_BASE_PIN);
    gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_2 << VDAC_BASE_PIN);
    delay();
    gpio_put(DAC_WR_PIN, 1);
    delay();
    gpio_put(DAC_WR_PIN, 0);
    delay();

    gpio_put_masked(0x000003 << DAC_SEL_BASE_PIN, 0 << DAC_SEL_BASE_PIN);
    gpio_put_masked(0x0000FF << VDAC_BASE_PIN, 0 << VDAC_BASE_PIN);
    delay();
    gpio_put_masked(0x00000F << ROW_A_BASE_PIN, ar_ << ROW_A_BASE_PIN);
    gpio_put_masked(0x00000F << COL_A_BASE_PIN, ac_ << COL_A_BASE_PIN);
    delay();
    gpio_put(DAC_EN_PIN, 1);
    gpio_put(DAC_WR_PIN, 1);
    gpio_put(TRIG_PIN, 1);
}

void LEDtestMenu::disable_flashes()
{
    gpio_put(DAC_EN_PIN, 0);
    gpio_put(DAC_WR_PIN, 0);
    delay();
    gpio_put_masked(0x0000FF << VDAC_BASE_PIN, 0 << VDAC_BASE_PIN);
    gpio_put_masked(0x00000F << ROW_A_BASE_PIN, 0 << ROW_A_BASE_PIN);
    gpio_put_masked(0x00000F << COL_A_BASE_PIN, 0 << COL_A_BASE_PIN);
    delay();
    gpio_put(TRIG_PIN, 0);
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
    vdac_2    = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_2     = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_2     = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
}

void LEDtestMenu::set_enable_disable_value(bool draw)
{
    menu_items_[MIP_FED].value = enable_disable_ ? ">ENABLE<" : "disable";
    menu_items_[MIP_FED].value_style = enable_disable_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_FED);
}

void LEDtestMenu::set_frequency_value(bool draw)
{
    menu_items_[MIP_FREQUENCY].value = std::to_string(frequency_);
    if(draw)draw_item_value(MIP_FREQUENCY);
}

void LEDtestMenu::set_delay_value(bool draw)
{
    menu_items_[MIP_DELAY].value = std::to_string(delay_);
    if(draw)draw_item_value(MIP_DELAY);
}

void LEDtestMenu::set_brightness_secled_value(bool draw)
{
    menu_items_[MIP_LEDTWO_BRIGHTNESS].value = std::to_string(vdac_2);
    if(draw)draw_item_value(MIP_LEDTWO_BRIGHTNESS);
}

void LEDtestMenu::set_brightness_firstled_value(bool draw)
{
    menu_items_[MIP_LEDONE_BRIGHTNESS].value = std::to_string(vdac_);
    if(draw)draw_item_value(MIP_LEDONE_BRIGHTNESS);
}

void LEDtestMenu::setsecled_rc_value(bool draw)
{
    rc_to_value_string(menu_items_[MIP_LEDTWO_ROWCOL].value, ar_2, ac_2);
    if(draw)draw_item_value(MIP_LEDTWO_ROWCOL);
}

void LEDtestMenu::setfirstled_rc_value(bool draw)
{
    rc_to_value_string(menu_items_[MIP_LEDONE_ROWCOL].value, ar_, ac_);
    if(draw)draw_item_value(MIP_LEDONE_ROWCOL);
}

std::vector<SimpleItemValueMenu::MenuItem> LEDtestMenu::make_menu_items()
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_LEDONE_ROWCOL) = {"Cursors    : Change column & row LED1", 3, "A0"};
    menu_items.at(MIP_LEDTWO_ROWCOL) = {"hjkl       : Change column & row LED2", 3, "B0"};
    menu_items.at(MIP_LEDONE_BRIGHTNESS) = {"</s/>      : Manage LED1 brightness", 4, "0"};
    menu_items.at(MIP_LEDTWO_BRIGHTNESS) = {"[/S/]      : Manage LED2 brightness", 4, "0"};
    menu_items.at(MIP_DELAY) = {"-/d/+      : Delay between LED1 and LED2", 8, "0"};
    menu_items.at(MIP_FREQUENCY) = {"{/f/}      : Manage frequency of flashes", 8, "100"};
    menu_items.at(MIP_FED) = {"E          : Enable and disable the flashes", 8, "disable"};
    menu_items.at(MIP_EXIT) = {"q          : Exit menu", 0, ""};
    return menu_items;
}

bool LEDtestMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer)
{
    if(process_rc_keys(ar_, ac_, key, key_count)) {
        setfirstled_rc_value(); //set the first led row and column value
        //gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
        //                (ar_ << ROW_A_BASE_PIN)|(ac_ << COL_A_BASE_PIN)); //color test = passed
        if (menu_items_[MIP_LEDONE_ROWCOL].value == menu_items_[MIP_LEDTWO_ROWCOL].value) { //error handling for the same row and column value
            beep();
        }
        return true;
    }
    if(hjkl_rc_keys(ar_2, ac_2, key, key_count)) {
        setsecled_rc_value(); //set the second led row and column value
        //gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
        //                (ar_2 << ROW_A_BASE_PIN)|(ac_2 << COL_A_BASE_PIN)); //color test = passed
        if (menu_items_[MIP_LEDTWO_ROWCOL].value == menu_items_[MIP_LEDONE_ROWCOL].value) { //error handling for the same row and column value
            beep();
        }
        return true;
    }

    switch (key) {
        case '>':
            increase_value_in_range(vdac_, 255, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN); //color test = passed
            set_brightness_firstled_value();
            break;
        case '<':
            decrease_value_in_range(vdac_, 0, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN); //color test = passed
            set_brightness_firstled_value();
            break;
        case 's':
            if(InplaceInputMenu::input_value_in_range(vdac_, 0, 255, this, MIP_LEDONE_BRIGHTNESS, 3)) {
                //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN); //color test = passed
            }
            set_brightness_firstled_value(true);
            break;
        case ']':
            increase_value_in_range(vdac_2, 255, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_2 << VDAC_BASE_PIN); //color test = passed
            set_brightness_secled_value();
            break;
        case '[':
            decrease_value_in_range(vdac_2, 0, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_2 << VDAC_BASE_PIN); //color test = passed
            set_brightness_secled_value();
            break;
        case 'S':
            if(InplaceInputMenu::input_value_in_range(vdac_2, 0, 255, this, MIP_LEDTWO_BRIGHTNESS, 3)) {
                //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_2 << VDAC_BASE_PIN); //color test = passed
            }
            set_brightness_secled_value(true);
            break;
        case '+':
            increase_value_in_range(delay_, 64, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, delay_ << VDAC_BASE_PIN); //color test = passed
            set_delay_value();
            break;
        case '-':
            decrease_value_in_range(delay_, 0, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, delay_ << VDAC_BASE_PIN); //color test = passed
            set_delay_value();
            break;
        case 'd':
            if(InplaceInputMenu::input_value_in_range(delay_, 0, 64, this, MIP_DELAY, 3)) {
                //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, delay_ << VDAC_BASE_PIN); //color test = passed
            }
            set_delay_value(true);
            break;
        case '}':
            increase_value_in_range(frequency_, 1000, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, frequency_ << VDAC_BASE_PIN); //color test = passed
            set_frequency_value();
            break;
        case '{':
            decrease_value_in_range(frequency_, 1, (key_count >= 15 ? 5 : 1), key_count==1);
            //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, frequency_ << VDAC_BASE_PIN); //color test = passed
            set_frequency_value();
            break;
        case 'f':
            if(InplaceInputMenu::input_value_in_range(frequency_, 1, 1000, this, MIP_FREQUENCY, 3)) {
                //gpio_put_masked(0x0000FF << VDAC_BASE_PIN, frequency_ << VDAC_BASE_PIN); //color test = passed
            }
            set_frequency_value(true);
            break;
        case 'E':
            enable_disable_ = !enable_disable_;
            set_enable_disable_value(true);
            if (enable_disable_) {
                enable_flashes();
            } else {
                disable_flashes();
            }
            break;
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
