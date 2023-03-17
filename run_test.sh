#!/bin/bash

REPO=${PWD}
PROJECT_NAME="gen_objects"

source ${REPO}/.venv/bin/activate

export COMMAND_RUN="docker run --rm -it -p 8080:8080 game_server"

export CONFIG_PATH=${REPO}/sprint3/problems/${PROJECT_NAME}/solution/data/config.json
export IMAGE_NAME="game_server"

python3 -m pytest --rootdir=${REPO} -v -vv -s --junitxml=results.xml cpp-backend-tests-practicum/tests/test_s03_${PROJECT_NAME}.py
# python3 -m pytest --rootdir=${REPO} -v -l -vv -s --junitxml=results.xml cpp-backend-tests-practicum/tests/test_s03_${PROJECT_NAME}.py::test_state_success
