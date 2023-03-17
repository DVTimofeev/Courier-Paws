cmake_minimum_required(VERSION 3.11)

#создаем статическую либу с элементами "низкой вероятностью" изменения при расширении приложения
add_library (MyLib STATIC
  boost_json.cpp
  json_loader.cpp
  loot_generator.cpp
  logging.cpp
  model.cpp
  player.cpp
  response.cpp
  ticker.cpp
  token.cpp
  uri_api.cpp
  collision_detector.cpp
)

target_include_directories(MyLib PUBLIC CONAN_PKG::boost ${MY_INCLUDE_DIR})
target_link_libraries(MyLib PUBLIC Threads::Threads CONAN_PKG::boost)

set(EXECUTABLE_FILES 
  main.cpp
  http_server.cpp
  request_handler.cpp
  api_handler.cpp
  application.cpp
  use_cases.cpp
)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server ${EXECUTABLE_FILES})

# используем "импортированную" цель CONAN_PKG::boost
target_link_libraries(game_server Threads::Threads MyLib)