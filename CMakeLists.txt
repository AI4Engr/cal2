cmake_minimum_required(VERSION 3.10)
project(cal2)

add_executable(cal2 cal2.cpp)

if(WIN32 AND NOT MSVC)  # MSYS2
    execute_process(
        COMMAND test -w /usr/bin
        RESULT_VARIABLE CAN_WRITE_USR_BIN
    )
    if(CAN_WRITE_USR_BIN EQUAL 0)
        set(INSTALL_BIN_DIR "/usr/bin")
    else()
        message(WARNING "Cannot write to /usr/bin, falling back to ~/.local/bin")
        set(INSTALL_BIN_DIR "$ENV{HOME}/.local/bin")
    endif()
else()
    set(INSTALL_BIN_DIR "bin")  # Or "/usr/local/bin" if you prefer absolute path
endif()

install(
    TARGETS cal2
    DESTINATION ${INSTALL_BIN_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE 
                WORLD_READ WORLD_EXECUTE
)
