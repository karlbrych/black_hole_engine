# Dependencies
Nainstaluj nejdriv tyhle sracky -> 
sudo pacman -Syu
sudo pacman -S base-devel cmake glfw-x11 glm mesa

## Cmake kompilace
mkdir build
cd build
cmake ..
make

## čistý g++
g++ src/main.cpp src/glad.c -Iinclude -lglfw -lGL -ldl -lm -o Blackhole



