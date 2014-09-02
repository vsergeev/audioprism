#!/bin/bash
PROJECT="spectrogram"
g++ `sdl2-config --cflags` -std=c++11 -W -Wall -Wextra -pedantic ${PROJECT}.cpp -o ${PROJECT} `sdl2-config --libs` -lpulse-simple -lpulse -lfftw3
