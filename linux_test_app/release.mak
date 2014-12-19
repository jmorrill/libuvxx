#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Release

#Toolchain
CC := C:/SysGCC/linaro/bin/arm-linux-gnueabihf-gcc.exe
CXX := C:/SysGCC/linaro/bin/arm-linux-gnueabihf-g++.exe
LD := $(CXX)
AR := C:/SysGCC/linaro/bin/arm-linux-gnueabihf-ar.exe
OBJCOPY := C:/SysGCC/linaro/bin/arm-linux-gnueabihf-objcopy.exe

#Additional flags
PREPROCESSOR_MACROS := NDEBUG RELEASE
INCLUDE_DIRS := ../libuv/include ../libuvxx/include
LIBRARY_DIRS := ../libuv/Release ../libuvxx/Release
LIBRARY_NAMES := uv uvxx pthread
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O3  -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -std=c++1y -pthread -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 -fexceptions -fnon-call-exceptions -fno-common -march=armv7-a -mtune=cortex-a8 -mfpu=neon -finline-limit=900
CXXFLAGS := -ggdb -ffunction-sections -O3  -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -std=c++1y -pthread -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 -fexceptions -fnon-call-exceptions -fno-common -march=armv7-a -mtune=cortex-a8 -mfpu=neon -finline-limit=900
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections -fexceptions -ggdb -ffunction-sections -O3 -std=c++1y -Wno-unused-parameter -Wextra -fno-common -pthread -fexceptions -fnon-call-exceptions -march=armv7-a -mtune=cortex-a8 -mfpu=neon -flto -flto-partition=none -finline-limit=900
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
USE_DEL_TO_CLEAN := 1
IS_LINUX_PROJECT := 1
