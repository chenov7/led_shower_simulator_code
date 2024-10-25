#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"
#include "spi_test_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

SPItestMenu::SPItestMenu() : 
    SimpleItemValueMenu(make_menu_items(), "SPI test Menu") 
{
    sync_values();
}

void SPItestMenu::sync_values()
{
    unsigned all_gpio = gpio_get_all();
    vdac_    = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_      = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_      = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
}

void SPItestMenu::delay()
{
    sleep_us(1);
}

void SPItestMenu::program_delay()
{
    mask_ = 128;
    int i;
    gpio_put(SPI_CLK_PIN, 1);
    gpio_put(SPI_COL_EN_PIN, 1);
    gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
                        (ar_ << ROW_A_BASE_PIN)|(ac_ << COL_A_BASE_PIN));
    delay();
    for(i = 7; i >= 0; i -= 1) {
        gpio_put(SPI_DOUT_PIN, delay_ & mask_ ? 1 : 0);
        gpio_put(SPI_CLK_PIN, 0);
        delay();
        gpio_put(SPI_CLK_PIN, 1);
        delay();
        mask_ = mask_ >> 1;
    }
    gpio_put(SPI_COL_EN_PIN, 0);
    gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
                        (0 << ROW_A_BASE_PIN)|(0 << COL_A_BASE_PIN));
    delay();
    gpio_put(SPI_DOUT_PIN, 0);
    gpio_put(SPI_CLK_PIN, 0);
}

void SPItestMenu::send_trigger()
{
    gpio_put(TRIG_PIN, 1);
    delay();
    gpio_put(TRIG_PIN, 0);
}

void SPItestMenu::set_rc_value(bool draw) 
{
    rc_to_value_string(menu_items_[MIP_ROWCOL].value, ar_, ac_);
    if(draw)draw_item_value(MIP_ROWCOL);
}

void SPItestMenu::set_delay_value(bool draw)
{
    menu_items_[MIP_DELAY].value = std::to_string(delay_); 
    if(draw)draw_item_value(MIP_DELAY);
}

void SPItestMenu::set_enable_value(bool draw) 
{
    menu_items_[MIP_ENABLE].value = enable_ ? ">ENABLE<" : "disable"; 
    menu_items_[MIP_ENABLE].value_style = enable_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_ENABLE);
}

void SPItestMenu::set_trigger_value(bool draw) 
{
    menu_items_[MIP_TRIGGER].value = trigger_ ? ">ON<" : "off";
    menu_items_[MIP_TRIGGER].value_style = trigger_ ? ANSI_INVERT : ""; 
    if(draw)draw_item_value(MIP_TRIGGER);
}

void SPItestMenu::set_enable_auto_trigger_value(bool draw) 
{
    menu_items_[MIP_AUTO_TRIGGER].value = enable_auto_trigger_ ? ">ENABLE<" : "disable"; 
    menu_items_[MIP_AUTO_TRIGGER].value_style = enable_auto_trigger_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_AUTO_TRIGGER);
}

std::vector<SimpleItemValueMenu::MenuItem> SPItestMenu::make_menu_items() 
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ROWCOL)       = {"Cursors : Change column & row", 3, "A0"};
    menu_items.at(MIP_DELAY)        = {"</d/>   : Delay", 3, "0"};
    menu_items.at(MIP_ENABLE)       = {"P       : Program delay", 8, "disable"};
    menu_items.at(MIP_TRIGGER)      = {"T       : Send trigger", 4, "off"};
    menu_items.at(MIP_AUTO_TRIGGER) = {"A       : enable auto-trigger", 8, "disable"};
    menu_items.at(MIP_EXIT)         = {"Q       : Exit menu", 0, ""};
    return menu_items;
}

bool SPItestMenu::controller_connected(int& return_code)
{
    return_code = 0;
    return true;
}

bool SPItestMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return true;
}

bool SPItestMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer)
{
    if(process_rc_keys(ar_, ac_, key, key_count)) {
        set_rc_value();
        return true;
    }

    switch (key) {
        case '<':
            decrease_value_in_range(delay_, 0, (key_count >= 15 ? 5 : 1), key_count==1);
            set_delay_value();
            break;
        case '>':
            increase_value_in_range(delay_, 255, (key_count >= 15 ? 5 : 1), key_count==1);
            set_delay_value();
            break;
        case 'D':
        case 'd':
            {
                SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_DELAY);
                InplaceInputMenu input(rc_getter, 3, VI_NATURAL, true, this);
                if(input.event_loop()==1 and input.get_value().size()!=0) {
                    int val = std::stoi(input.get_value());
                    if(val>=0 and val<=255) {
                        delay_ = val;
                    } else {
                        input.cancelled();
                    }
                } else {
                    input.cancelled();
                }
                set_delay_value(true);
            }
            break;
        case 'P':
            enable_ = true;
            set_enable_value();
            program_delay();
            enable_ = false;
            set_enable_value();
            break;
        case 'T':
            trigger_ = true;
            set_trigger_value();
            send_trigger();
            trigger_ = false;
            set_trigger_value();
            break;
        case 'A':
            time_ = 0;
            enable_auto_trigger_ = !enable_auto_trigger_;
            set_enable_auto_trigger_value();
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

bool SPItestMenu::process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer)
{
    heartbeat_timer_count_ += 1;
    if(heartbeat_timer_count_ == 100) {
        if(controller_is_connected) {
            set_heartbeat(!heartbeat_);
        }
        heartbeat_timer_count_ = 0;
    }

    if (enable_auto_trigger_ == true){
        time_ += 1;
        if (time_ >= 10){
            trigger_ = true;
            set_trigger_value();
            send_trigger();
            trigger_ = false;
            set_trigger_value();
            time_ = 0;
        }
    }
    return true;
}