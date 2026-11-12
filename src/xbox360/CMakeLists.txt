
cmake_minimum_required(VERSION 3.25)

project(J2ME_Loader_360 C)

add_executable(j2me_loader_360 main.c)

set_target_properties(j2me_loader_360 PROPERTIES
    C_STANDARD 17
    C_EXTENSIONS ON
)

target_compile_options(j2me_loader_360 PRIVATE
    -Wall
    -Wextra
)

xsdk_add_xex(j2me_loader_360
    NAME "J2ME-Loader-360"
    VERSION 0.1.0.0
)
