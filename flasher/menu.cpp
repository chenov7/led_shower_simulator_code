#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/stdlib.h>

#include "menu.hpp"

Menu::~Menu() 
{ 
    // nothing to see here
}

int Menu::puts_raw_nonl(const char* s) 
{
    for (size_t i = 0; s[i]; ++i) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    return 0;
}

int Menu::puts_raw_nonl(const char* s, size_t maxchars, bool fill) 
{
    for (size_t i = 0; s[i] && maxchars; ++i, --maxchars) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    if(fill && maxchars) {
        while(maxchars--) {
            if (putchar_raw(' ') == EOF) return EOF;
        }
    }
    return 0;
}

int Menu::puts_raw_nonl(const std::string& s) {
    for (size_t i=0; i<s.size(); ++i) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    return 0;
}

int Menu::puts_raw_nonl(const std::string& s, size_t maxchars, bool fill) 
{
    size_t schars = std::min(maxchars, s.size());
    for (size_t i=0; i<schars; ++i, --maxchars) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    if(fill && maxchars) {
        while(maxchars--) {
            if (putchar_raw(' ') == EOF) return EOF;
        }
    }
    return 0;
}

void Menu::cls() 
{ 
    puts_raw_nonl("\033[2J"); 
}

void Menu::show_cursor() 
{ 
    puts_raw_nonl("\033[?25h\0337p"); 
}

void Menu::hide_cursor() 
{ 
    puts_raw_nonl("\033[?25l\0336p"); 
}

void Menu::curpos(int r, int c) 
{ 
    char buffer[80]; 
    sprintf(buffer,"\033[%d;%dH",r,c); 
    puts_raw_nonl(buffer);
}

void Menu::send_request_screen_size() 
{
    puts_raw_nonl("\033[999;999H\033[6n");
}

void Menu::draw_box(int fh, int fw, int fr, int fc) {
    char buffer[80];
    sprintf(buffer,"\033[%d;%dH",fr+1,fc+1);
    puts_raw_nonl(buffer);
    putchar_raw('+');
    for(int ic=2;ic<fw;++ic)putchar_raw('-');
    putchar_raw('+');
    for(int ir=2;ir<fh;++ir) {
        sprintf(buffer,"\033[%d;%dH|\033[%d;%dH|",fr+ir,fc+1,fr+ir,fc+fw);
        puts_raw_nonl(buffer);
    }
    sprintf(buffer,"\033[%d;%dH",fr+fh,fc+1);
    puts_raw_nonl(buffer);
    putchar_raw('+');
    for(int ic=2;ic<fw;++ic)putchar_raw('-');
    putchar_raw('+');
}

bool Menu::draw_title(const std::string& title, int fh, int fw, int fr, int fc,
     const std::string& title_style) 
{
    char buffer[80];
    if(fh<5 || fw<5)return false;
    int tw = std::min(int(title.size()), fw-4);
    int tc = fc + (fw-tw)/2;
    sprintf(buffer,"\033[%d;%dH",fr+3,tc+1);
    puts_raw_nonl(buffer);
    if(!title_style.empty())puts_raw_nonl(title_style);
    puts_raw_nonl(title, tw);
    if(!title_style.empty())puts_raw_nonl("\033[0m");
    return tw == int(title.size());
}

int Menu::event_loop(bool enable_escape_sequences)
{
    int return_code = 0;
    bool continue_looping = true;
    bool was_connected = false;
    int last_key = -1;
    int key_count = 0;
    std::string escape_sequence;
    while(continue_looping) {
        if(stdio_usb_connected()) {
            if(!was_connected) {
                this->redraw();
            }
            was_connected = true;
            int key = getchar_timeout_us(100000);
            if(key >= 0) { 
                if(!escape_sequence.empty()) {
                    bool continue_accumulating_escape_sequence = false;
                    int escaped_key =
                        decode_partial_escape_sequence(key, escape_sequence,    
                            continue_accumulating_escape_sequence);
                    if(escaped_key >= 0) {
                        if(escaped_key == last_key) {
                            ++key_count;
                        } else {
                            last_key = escaped_key;
                            key_count = 1;
                        }
                        continue_looping = this->process_key_press(escaped_key, key_count, return_code);
                        escape_sequence.clear();
                    } else if (continue_accumulating_escape_sequence) {
                        escape_sequence.push_back(key);
                    } else {
                        last_key = -1;
                        key_count = 0;
                        for(auto k : escape_sequence) {
                            continue_looping = this->process_key_press(k, 1, return_code);
                            if(!continue_looping)return return_code;
                        }
                        continue_looping = this->process_key_press(key, 1, return_code);
                        escape_sequence.clear();
                    }
                } else if(enable_escape_sequences and key == '\033') {
                    escape_sequence.push_back(key);
                    continue_looping = true;
                } else if(key == '\014') {
                    last_key = -1;
                    key_count = 0;
                    this->redraw();
                    continue;
                } else {
                    if(key == last_key) {
                        ++key_count;
                    } else {
                        last_key = key;
                        key_count = 1;
                    }
                    continue_looping = this->process_key_press(key, key_count, return_code);
                }
            } else {
                if(!escape_sequence.empty()) {
                    for(auto k : escape_sequence) {
                        continue_looping = this->process_key_press(k, 1, return_code);
                        if(!continue_looping)return return_code;
                    }
                    escape_sequence.clear();
                }
                last_key = -1;
                key_count = 0;
                continue_looping = this->process_timeout(return_code);
            }
        } else {
            was_connected = false;
            escape_sequence.clear();
            sleep_us(1000);
        }
    }
    return return_code;
}

int Menu::decode_partial_escape_sequence(int key, std::string& escape_sequence,
    bool& continue_accumulating_escape_sequence)
{
    int escaped_key = -1;
    continue_accumulating_escape_sequence = false;
    int escape_sequence_size = escape_sequence.size();
    if(escape_sequence_size == 1) {
        switch(key) {
        case '\033':
            escaped_key = '\033'; break;
        case '[':
        case 'O':
            continue_accumulating_escape_sequence = true; break;
        default: break;
        }
    } else if(escape_sequence_size==2 and escape_sequence[1] == 'O') {
        switch(key) {
        // See https://www.gnu.org/software/screen/manual/html_node/Input-Translation.html#Input-Translation
        case 'A': escaped_key = KEY_UP; break;
        case 'B': escaped_key = KEY_DOWN; break;
        case 'C': escaped_key = KEY_RIGHT; break;
        case 'D': escaped_key = KEY_LEFT; break;
        case 'H': escaped_key = KEY_HOME; break;
        case 'F': escaped_key = KEY_END; break;
        case 'P': escaped_key = KEY_F1; break;
        case 'Q': escaped_key = KEY_F2; break;
        case 'R': escaped_key = KEY_F3; break;
        case 'S': escaped_key = KEY_F4; break;
        case 'p': escaped_key = '0'; break;
        case 'q': escaped_key = '1'; break;
        case 'r': escaped_key = '2'; break;
        case 's': escaped_key = '3'; break;
        case 't': escaped_key = '4'; break;
        case 'u': escaped_key = '5'; break;
        case 'v': escaped_key = '6'; break;
        case 'w': escaped_key = '7'; break;
        case 'x': escaped_key = '8'; break;
        case 'y': escaped_key = '9'; break;
        case 'k': escaped_key = '+'; break;
        case 'm': escaped_key = '-'; break;
        case 'j': escaped_key = '*'; break;
        case 'o': escaped_key = '/'; break;
        case 'X': escaped_key = '='; break;
        case 'n': escaped_key = '.'; break;
        case 'l': escaped_key = ','; break;
        case 'M': escaped_key = '\r'; break;
        default: break;
        }
    } else if(escape_sequence[1] == '[') {
        switch(key)
        {
        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            continue_accumulating_escape_sequence = true;
            break;
        case 'A': escaped_key = KEY_UP; break;
        case 'B': escaped_key = KEY_DOWN; break;
        case 'C': escaped_key = KEY_RIGHT; break;
        case 'D': escaped_key = KEY_LEFT; break;
        case 'H': escaped_key = KEY_HOME; break;
        case 'F': escaped_key = KEY_END; break;
        case '~':
            if(escape_sequence_size == 3) {
                switch(escape_sequence[2]) 
                {
                case '1': escaped_key = KEY_HOME; break;
                case '2': escaped_key = KEY_INSERT; break;
                case '3': escaped_key = KEY_DELETE; break;
                case '4': escaped_key = KEY_END; break;
                case '5': escaped_key = KEY_PAGE_UP; break;
                case '6': escaped_key = KEY_PAGE_DOWN; break;
                case '7': escaped_key = KEY_HOME; break;
                case '8': escaped_key = KEY_END; break;
                default: break;
                }
            } else if(escape_sequence_size==4 and escape_sequence[2]=='1') {
                switch(escape_sequence[3]) 
                {
                case '0': escaped_key = KEY_F0; break;
                case '1': escaped_key = KEY_F1; break;
                case '2': escaped_key = KEY_F2; break;
                case '3': escaped_key = KEY_F3; break;
                case '4': escaped_key = KEY_F4; break;
                case '5': escaped_key = KEY_F5; break;
                case '7': escaped_key = KEY_F6; break;
                case '8': escaped_key = KEY_F7; break;
                case '9': escaped_key = KEY_F8; break;
                default: break;
                }
            } else if(escape_sequence_size==4 and escape_sequence[2]=='2') {
                switch(escape_sequence[3]) 
                {
                case '0': escaped_key = KEY_F9; break;
                case '1': escaped_key = KEY_F10; break;
                case '3': escaped_key = KEY_F11; break;
                case '4': escaped_key = KEY_F12; break;
                default: break;
                }
            }
        default: break;
        }
    }
    return escaped_key;
}

FramedMenu::FramedMenu(const std::string& title, int frame_h, int frame_w, int frame_pos):
    title_(title), req_h_(frame_h), req_w_(frame_w), req_pos_(frame_pos)
{
    // nothing to see here
}

FramedMenu::~FramedMenu()
{
    // nothing to see here
}

void FramedMenu::redraw()
{
    setup_frame();
    hide_cursor();
    cls();
    draw_box(frame_h_, frame_w_, frame_r_, frame_c_);
    if(!title_.empty())draw_title(title_, frame_h_, frame_w_, frame_r_, frame_c_, {});
}

void FramedMenu::setup_frame()
{
    frame_h_ = (req_h_>0) ? std::min(screen_h_, req_h_) : screen_h_;
    frame_w_ = (req_w_>0) ? std::min(screen_w_, req_w_) : screen_w_;
    switch(req_pos_) {
        case 0: case 3: case 7: default:
            frame_r_ = (screen_h_-frame_h_)/2; break;
        case 1: case 2: case 8: 
            frame_r_ = 0; break;
        case 4: case 5: case 6: 
            frame_r_ = screen_h_-frame_h_; break;
    }
    switch(req_pos_) {
        case 0: case 1: case 5: default:
            frame_c_ = (screen_w_-frame_w_)/2; break;
        case 2: case 3: case 4: 
            frame_c_ = screen_w_-frame_w_; break;
        case 6: case 7: case 8: 
            frame_c_ = 0; break;
    }
}

SimpleItemValueMenu::MenuItem::
MenuItem(const std::string& item_, int max_value_size_, const std::string& value_):
    item(item_), max_value_size(max_value_size_), value(value_)
{ 
    // nothing to see here
}

SimpleItemValueMenu::SimpleItemValueMenu(const std::vector<MenuItem>& menu_items, 
        const std::string& title, int frame_h, int frame_w, int frame_pos):
    FramedMenu(title, frame_h, frame_w, frame_pos), menu_items_(menu_items)
{
    // nothing to see here
}

SimpleItemValueMenu::~SimpleItemValueMenu()
{
    // nothing to see here
}

void SimpleItemValueMenu::setup_menu()
{
    item_r_ = frame_r_ + (title_.empty() ? 2 : 4);
    for(const auto& i : menu_items_) {
        item_w_ = std::max(item_w_, int(i.item.size()));
        val_w_ = std::max(val_w_, int(i.max_value_size));        
    }
    item_count_ = menu_items_.size();
    if(2*item_count_+item_r_+1 < frame_h_) {
        item_h_ = 2*item_count_-1;
        item_dr_ = 2;
    } else {
        item_h_ = std::min(item_count_, frame_h_-item_r_-2);
        item_dr_ = 1;
        item_count_ = item_h_;
    }
    item_w_ = std::min(item_w_, frame_w_ - val_w_ - 6);
    item_r_ += (frame_h_ - item_h_ - item_r_ - 2)/2;
    item_c_ = frame_c_ + std::min(5, frame_w_-(item_w_+val_w_+6)/2);
    val_c_ = frame_c_ + frame_w_ - val_w_ - (item_c_ - frame_c_);
}


void SimpleItemValueMenu::redraw()
{
    FramedMenu::redraw();
    setup_menu();
    for(int iitem=0;iitem<item_count_;++iitem) {
        draw_item(iitem);
    }
}

void SimpleItemValueMenu::draw_item(unsigned iitem)
{
    curpos(item_r_+iitem*item_dr_+1, item_c_+1);
    if(menu_items_[iitem].max_value_size > 0) {
        puts_raw_nonl(menu_items_[iitem].item, item_w_);
        putchar_raw(' ');
        for(int ic = item_c_+menu_items_[iitem].item.size()+2; ic<val_c_; ic ++)
            putchar_raw('.');
        putchar_raw(' ');
        draw_item_value(iitem);
    } else {
        puts_raw_nonl(menu_items_[iitem].item, item_w_+val_w_+2);
    }
}

void SimpleItemValueMenu::draw_item_value(unsigned iitem)
{
    if(iitem<menu_items_.size()) {
        curpos(item_r_+iitem*item_dr_+1, val_c_+1);
        puts_raw_nonl(menu_items_[iitem].value, menu_items_[iitem].max_value_size, true);
    }
}
