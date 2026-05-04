CXX      = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -I include -I imgui -I imgui/backends -DGL_SILENCE_DEPRECATION

GLFW_FLAGS = $(shell pkg-config --cflags glfw3)
GLFW_LIBS  = $(shell pkg-config --libs glfw3)
MAC_LIBS   = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

IMGUI_SRC = imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp \
            imgui/imgui_widgets.cpp imgui/imgui_demo.cpp \
            imgui/backends/imgui_impl_glfw.cpp \
            imgui/backends/imgui_impl_opengl3.cpp

APP_SRC = $(wildcard src/*.cpp)
TARGET  = finance_app

all: $(TARGET)

$(TARGET): $(APP_SRC) $(IMGUI_SRC)
	$(CXX) $(CXXFLAGS) $(GLFW_FLAGS) $^ -o $@ $(GLFW_LIBS) $(MAC_LIBS)

test:
	$(CXX) $(CXXFLAGS) src/tests.cpp src/HashMap.cpp src/MinHeap.cpp \
	src/BST.cpp src/BudgetManager.cpp src/Expense.cpp src/Bill.cpp \
	src/CategoryInfo.cpp src/Date.cpp -o run_tests && ./run_tests

clean:
	rm -f $(TARGET) run_tests

.PHONY: all clean test
