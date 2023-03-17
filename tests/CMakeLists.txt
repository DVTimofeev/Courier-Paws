cmake_minimum_required(VERSION 3.11)

set(TEST_FILES 
  loot_generator_tests.cpp
  collision-detector-tests.cpp
)

add_executable(game_server_tests ${TEST_FILES})

# используем "импортированную" цель CONAN_PKG::boost
target_include_directories(game_server_tests PRIVATE ${MY_INCLUDE_DIR})
# target_include_directories(game_server_tests PRIVATE CONAN_PKG::boost)
# target_link_libraries(tests PRIVATE Threads::Threads CONAN_PKG::boost CONAN_PKG::catch2)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 MyLib)