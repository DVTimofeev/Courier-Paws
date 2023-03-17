#!/bin/bash

# запуск программы
./build/bin/game_server --config-file ./data/config.json --www-root ./static --tick-period 50 --randomize-spawn-points
# ./build/bin/game_server --config-file ./data/config.json --www-root ./static --tick-period 10
# ./build/bin/game_server --config-file ./data/config.json --www-root ./static --randomize-spawn-points
# ./build_debug/bin/command_line --config-file ./data/config.json --www-root ./static --randomize-spawn-points
