#CXX = g++
#CXX = clang++

EXE = hssc-gui
IMGUI_DIR = ..
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/imgui_impl_sdl3.cpp $(IMGUI_DIR)/imgui_impl_sdlrenderer3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL


CXXFLAGS = -I$(IMGUI_DIR)
CXXFLAGS += -Os -s -DNDEBUG -ffunction-sections -fdata-sections # -flto 
СXXFLAGS += -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables
CXXFLAGS += -Wl,--gc-sections -Wl,--strip-all -g -Wall -Wextra -std=c++23


LIBS =

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	LIBS += -ldl `pkg-config sdl3 --libs`

	CXXFLAGS += `pkg-config sdl3 --cflags`
	CFLAGS = $(CXXFLAGS)
endif

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)

install: all
	sudo cp -f $(EXE) /usr/bin

uninstall: clean
	sudo rm /usr/bin/$(EXE)