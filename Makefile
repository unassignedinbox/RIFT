# RIFT — standalone transcribed panels. Sandbox build; Module.toml carries the declarative unit map.
#
#   make            build both hosts
#   make proof      run the hosts headlessly and encode VisualProof PNGs
#   make clean      remove build output

CXX      ?= g++
CXXFLAGS ?= -std=c++20 -O1 -g0 -Wall -Wno-unused-parameter -Wno-unused-variable
INCLUDES := -I ExternalPackages/imgui -I Engine -I .

IMGUI_SOURCES := \
    ExternalPackages/imgui/imgui.cpp \
    ExternalPackages/imgui/imgui_draw.cpp \
    ExternalPackages/imgui/imgui_tables.cpp \
    ExternalPackages/imgui/imgui_widgets.cpp

ENGINE_SOURCES := \
    Engine/SlateUI/Interface/PanelExchange/Source/PanelExchange.cpp \
    Engine/SlateUI/Interface/InterfaceSequence/Source/InterfaceSequence.cpp \
    Engine/SlateUI/Interface/IconDepot/Source/IconDepot.cpp \
    Engine/SlateUI/Interface/ControlPanel/Source/ControlPanel.cpp \
    Engine/SlateUI/Interface/OutlinerPanel/Source/OutlinerPanel.cpp \
    Engine/SlateUI/Interface/PropertiesPanel/Source/PropertiesPanel.cpp \
    Engine/SlateUI/Interface/DraftingPanel/Source/DraftingPanel.cpp \
    Engine/SlateUI/Interface/TexturePaintPanel/Source/TexturePaintPanel.cpp \
    Engine/SlateUI/Interface/RasterCodec/Source/RasterCodec.cpp

BUILD := Build

.PHONY: all proof outliner validation clean

all: $(BUILD)/OutlinerHost $(BUILD)/PanelValidationHost

outliner: $(BUILD)/OutlinerHost

validation: $(BUILD)/PanelValidationHost

$(BUILD):
	mkdir -p $(BUILD) $(BUILD)/Shots

$(BUILD)/OutlinerHost: $(IMGUI_SOURCES) $(ENGINE_SOURCES) \
                       Engine/Application/OutlinerHost/Source/OutlinerHost.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(BUILD)/PanelValidationHost: $(IMGUI_SOURCES) $(ENGINE_SOURCES) Engine/Application/PanelValidationHost/Source/PanelValidationHost.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

proof: all
	mkdir -p VisualProof/OutlinerHost VisualProof/PanelValidationHost $(BUILD)/Shots
	rm -f $(BUILD)/Shots/*.rgba
	$(BUILD)/OutlinerHost --prefix VisualProof/OutlinerHost
	$(BUILD)/PanelValidationHost --prefix VisualProof/PanelValidationHost
	for Dump in $(BUILD)/Shots/directory.rgba $(BUILD)/Shots/multiselect.rgba $(BUILD)/Shots/filter.rgba; do \
		python3 Tools/EncodeProof.py "$$Dump" "VisualProof/OutlinerHost/$$(basename "$$Dump" .rgba).png"; \
	done
	for Dump in $(BUILD)/Shots/texturepaint-*.rgba $(BUILD)/Shots/cad-*.rgba; do \
		python3 Tools/EncodeProof.py "$$Dump" "VisualProof/PanelValidationHost/$$(basename "$$Dump" .rgba).png"; \
	done

check: proof
	python3 Tools/AssertProofs.py

clean:
	rm -rf $(BUILD)

# ① The interactive window host — requires local GLFW and OpenGL development packages
#    (`pkg-config glfw3 gl`). Not built by default; the sandbox has neither.
outliner-window: $(IMGUI_SOURCES) $(ENGINE_SOURCES) \
                 Engine/Application/OutlinerHost/Source/WindowHost.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(shell pkg-config --cflags --libs glfw3 gl) -o $(BUILD)/OutlinerWindowHost

check: proof
	python3 Tools/AssertProofs.py
