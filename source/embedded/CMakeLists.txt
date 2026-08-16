##
# @file CMakeLists.txt
# @brief
#/

# APP_PATH
set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})

# APP_NAME
get_filename_component(APP_NAME ${APP_PATH} NAME)

# APP_SRC
# 注意：aux_source_directory 不递归。每新增一个源码子目录都要在这里追加一行，
# 否则文件不会被编译，且不会有任何报错（只会在链接时报 undefined reference）。
aux_source_directory(${APP_PATH}/src         APP_SRC)
aux_source_directory(${APP_PATH}/src/board   APP_SRC_BOARD)
aux_source_directory(${APP_PATH}/src/app     APP_SRC_APP)
aux_source_directory(${APP_PATH}/src/ui      APP_SRC_UI)
aux_source_directory(${APP_PATH}/src/ui/fonts APP_SRC_FONTS)
aux_source_directory(${APP_PATH}/src/input   APP_SRC_INPUT)
aux_source_directory(${APP_PATH}/src/haptic  APP_SRC_HAPTIC)
aux_source_directory(${APP_PATH}/src/ble     APP_SRC_BLE)
aux_source_directory(${APP_PATH}/src/audio   APP_SRC_AUDIO)
aux_source_directory(${APP_PATH}/src/storage APP_SRC_STORAGE)
list(APPEND APP_SRC
    ${APP_SRC_BOARD}
    ${APP_SRC_APP}
    ${APP_SRC_UI}
    ${APP_SRC_FONTS}
    ${APP_SRC_INPUT}
    ${APP_SRC_HAPTIC}
    ${APP_SRC_BLE}
    ${APP_SRC_AUDIO}
    ${APP_SRC_STORAGE}
    )

# APP_INC
set(APP_INC
    ${APP_PATH}/include
    ${APP_PATH}/src/app
    ${APP_PATH}/src/ui
    ${APP_PATH}/src/input
    ${APP_PATH}/src/haptic
    ${APP_PATH}/src/ble
    ${APP_PATH}/src/audio
    ${APP_PATH}/src/storage
    )

# APP_OPTIONS
set(APP_OPTIONS "")  # -W

########################################
# Target Configure
########################################
add_library(${EXAMPLE_LIB})
message(STATUS "EXAMPLE_LIB:${APP_PATH}")

target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_SRC}
    )

target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_INC}
    )

target_compile_options(${EXAMPLE_LIB}
    PRIVATE
        ${APP_OPTIONS}
    )
