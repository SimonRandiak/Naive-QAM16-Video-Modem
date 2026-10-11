#!/bin/bash

g++ main.cpp -o main -std=c++20 -Wall -Wextra -O3 -march=native -lzmq -I/usr/include/eigen3 -o rx
