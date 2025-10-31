cmake_minimum_required(VERSION 3.16)
project(touch_windows C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add executable
add_executable(touch_windows main.c)

# Link against screen library
target_link_libraries(touch_windows screen)

# Set properties for QNX
set_target_properties(touch_windows PROPERTIES
    C_EXTENSIONS OFF
    SUFFIX ""  # Ensure no .elf extension if your toolchain adds it
)

# Install target if needed
install(TARGETS touch_windows
    RUNTIME DESTINATION bin
    COMPONENT applications
)