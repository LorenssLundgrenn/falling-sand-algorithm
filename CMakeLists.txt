cmake_minimum_required(VERSION 3.28.3)

project(sand)

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/)
set(SRC_DIR ./src/)

set(SRC ${SRC_DIR}main.cpp
    ${SRC_DIR}state.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

set(EXTLIB_DIR C:/dependencies/)

target_include_directories( ${PROJECT_NAME} 
    PRIVATE ./includes
    PRIVATE ${EXTLIB_DIR}PDCurses-3.9/
    PRIVATE ${EXTLIB_DIR}PDCurses-3.9/wincon/
)

target_link_libraries( ${PROJECT_NAME}
    PRIVATE ${EXTLIB_DIR}PDCurses-3.9/wincon/pdcurses.a
)

set(INPUT_FILE ${BUILD_DIR}in.txt)
if(NOT EXISTS ${INPUT_FILE})
    file( WRITE ${INPUT_FILE} 
        "write anything here and the program\n"
        "will simulate the characters as\n"
        "individual particles\n\n\n\n\n\n"
    )
endif()