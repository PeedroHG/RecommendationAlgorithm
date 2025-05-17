CXX      := g++                 
CXXFLAGS := -Wall -Wextra -O3 -g
LDFLAGS  := -lm                 
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/
TARGET   := app
INCLUDE  := -Iinclude/
SRC      := $(wildcard src/*.cpp) 

OBJECTS := $(SRC:src/%.cpp=$(OBJ_DIR)/src/%.o)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) -o $(APP_DIR)/$(TARGET) $(OBJECTS)

.PHONY: all build clean debug release run

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O3
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

run:
	./$(BUILD)/$(TARGET)
