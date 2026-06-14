#!/usr/bin/env bash

# g++ --std=c++20 -g -Wall -Wextra ./src/test_glm.cpp -I./deps/glm/ -o test_glm
g++ -std=c++20 ./src/test_glm.cpp -I./deps/glm/ -o test_glm
