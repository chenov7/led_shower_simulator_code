add_executable(flasher)

pico_generate_pio_header(flasher ${CMAKE_CURRENT_LIST_DIR}/set_charges.pio)

target_sources(flasher PRIVATE flasher.cpp 
        menu.cpp menu_event_loop.cpp input_menu.cpp reboot_menu.cpp
        engineering_menu.cpp event_generators.cpp event_dispatcher.cpp 
        keypress_menu.cpp main_menu.cpp dc_ramp_menu.cpp spi_test_menu.cpp
        ledtest_menu.cpp)

# pull in common dependencies
target_link_libraries(flasher PRIVATE
        pico_stdlib pico_multicore pico_sync hardware_pio)
target_compile_definitions(flasher PRIVATE)

# create map/bin/hex file etc.
pico_add_extra_outputs(flasher)

# add url via pico_set_program_url
flasher_auto_set_url(flasher)

pico_enable_stdio_usb(flasher 1)
pico_enable_stdio_uart(flasher 0)

