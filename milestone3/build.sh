#!/bin/bash

SRC=src
OUT=bin
EXE=picture.exe

mkdir -p $OUT

g++							 \
$SRC/main.cpp 				 \
$SRC/platform/SDL_window.cpp \
$SRC/renderer/vk_init.cpp 	 \
-o $OUT/$EXE 				 \
-lmingw32  					 \
-lSDL2main 					 \
-lSDL2 						 \
-lvulkan-1

echo "Build complete!"