cmake_minimum_required(VERSION 3.11)

set(PRJ_NAME gen_objects)

project(${PRJ_NAME} CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(MY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# find_package(Boost 1.78.0 REQUIRED)
# if(Boost_FOUND)
#   include_directories(${Boost_INCLUDE_DIRS})
# endif()

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

# #создаем статическую либу с элементами "низкой вероятностью" изменения при расширении приложения
# add_library (MyLib STATIC
#   src/boost_json.cpp
#   src/json_loader.cpp
#   src/loot_generator.cpp
#   src/logging.cpp
#   src/model.cpp
#   src/player.cpp
#   src/response.cpp
#   src/ticker.cpp
#   src/token.cpp
#   src/uri_api.cpp
# )

# target_include_directories(MyLib PUBLIC ${MY_INCLUDE_DIR})
# target_link_libraries(MyLib PUBLIC CONAN_PKG::boost)

# target_include_directories(MyLib PUBLIC CONAN_PKG::boost ${MY_INCLUDE_DIR})
# target_link_libraries(MyLib PUBLIC Threads::Threads CONAN_PKG::boost)

add_subdirectory(src)
add_subdirectory(tests)
