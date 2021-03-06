CC = gcc

BUILD_DIR = bin
SRC_DIR = src

TARGET = scrcpy_dev.exe

MKDIR_P_BUILD = if not exist "$(BUILD_DIR)" mkdir $(BUILD_DIR)

CINCLUDE = -I.\$(SRC_DIR)\ -I.\$(SRC_DIR)\android -I.\$(SRC_DIR)\custom -I\msys64\mingw64\include
CLIBS = -L\msys64\mingw64\lib -lavcodec -lavformat -lavutil -lws2_32 -lmingw32 -mwindows -mconsole -lSDL2

SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/sys/win/*.c) $(wildcard $(SRC_DIR)/custom/*.c)

$(TARGET): $(SOURCES)
	$(shell $(MKDIR_P_BUILD))
	$(CC) -o $(BUILD_DIR)/$@ $(CINCLUDE) $^ $(CLIBS)	