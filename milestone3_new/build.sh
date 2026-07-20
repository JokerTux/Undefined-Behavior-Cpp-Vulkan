#!/bin/bash

SRC=src
OUT=bin
VENDOR=vendors
EXE=mesh.exe

gpp(){	
		g++ \
			$SRC/main.cpp \
			$SRC/platform/SDL_window.cpp \
		    $SRC/renderer/instance.cpp \
		    $SRC/renderer/surface.cpp \
		    $SRC/renderer/device.cpp \
		    $SRC/renderer/swapchain.cpp \
		    $SRC/renderer/image_views.cpp \
			$SRC/renderer/render_pass.cpp \
			$SRC/renderer/framebuffers.cpp \
			$SRC/renderer/command_pool.cpp \
			$SRC/renderer/pipeline.cpp \
			$SRC/renderer/sync.cpp \
			$SRC/renderer/buffers.cpp \
			$SRC/mesh.cpp \
			-I$VENDOR/tinyobjloader \
			-I$VENDOR/glm \
			-lmingw32 \
			-lSDL2main \
			-lSDL2 \
			-lvulkan-1 \
			-o $OUT/$EXE

		return $?		
	}

mkdir -p $OUT

if gpp ; then
	echo "Build complete!"
else
	echo "Build failed !"
fi
