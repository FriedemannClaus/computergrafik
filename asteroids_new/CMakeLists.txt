cmake_minimum_required (VERSION 3.9)
project (asteroids)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-g -Wall -Wextra -Wpedantic -Wl,--stack,16777216)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(main_game game.cc math.cc matrix.cc geometry.cc sdl2_renderer.cc opengl_renderer.cc sound.cc main_game.cc physics.cc sdl2_game_controller.cc timer.cc)

# GLEW
add_subdirectory(glew-2.1.0/build/cmake EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glew-2.1.0/include)
#find_package(GLEW REQUIRED)

#SDL2
add_subdirectory(SDL-release-2.28.4 EXCLUDE_FROM_ALL)

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(main_game PRIVATE SDL2::SDL2main SDL2::SDL2 OPENGL32 glew) #GLEW32 GLEW::glew
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL library.
# target_link_libraries(main_game OPENGL32 GLEW32 PRIVATE SDL2::SDL2)

if (WIN32)
    add_custom_command(
            TARGET main_game POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:main_game>"
            VERBATIM
    )

    add_custom_command(
            TARGET main_game POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:glew>" "$<TARGET_FILE_DIR:main_game>"
            VERBATIM
    )
endif()


#target_link_libraries(main_game OPENGL32 GLEW32) # MinGW
# target_link_libraries(main_game SDL2 SDL2_mixer GL GLEW) # Linux

enable_testing()
add_executable(math_test math_test.cc math.cc)
target_link_libraries(math_test gtest gtest_main)
add_executable(matrix_test matrix_test.cc matrix.cc math.cc)
target_link_libraries(matrix_test gtest gtest_main)
add_executable(geometry_test geometry_test.cc geometry.cc math.cc)
target_link_libraries(geometry_test gtest gtest_main)
add_executable(physics_test physics_test.cc physics.cc geometry.cc math.cc timer.cc)
target_link_libraries(physics_test gtest gtest_main SDL2)
add_executable(game_test game_test.cc game.cc physics.cc geometry.cc math.cc timer.cc)
target_link_libraries(game_test gtest gtest_main SDL2)

