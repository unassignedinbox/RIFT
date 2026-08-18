# RIFT — standalone transcribed panels. Sandbox build; Module.toml carries the declarative unit map.
#
#   make            build both hosts
#   make proof      run the hosts headlessly and encode VisualProof PNGs
#   make clean      remove build output

CXX      ?= g++
CXXFLAGS ?= -std=c++20 -O1 -g0 -Wall -Wno-unused-parameter -Wno-unused-variable
INCLUDES := -I ExternalPackages/imgui -I .

IMGUI_SOURCES := \
    ExternalPackages/imgui/imgui.cpp \
    ExternalPackages/imgui/imgui_draw.cpp \
    ExternalPackages/imgui/imgui_tables.cpp \
    ExternalPackages/imgui/imgui_widgets.cpp

ENGINE_SOURCES := \
    Engine/SlateUI/Interface/RecordingSurface/Source/RecordingSurface.cpp \
    Engine/SlateUI/Interface/IconDepot/Source/IconDepot.cpp \
    Engine/SlateUI/Interface/ControlPanel/Source/ControlPanel.cpp \
    Engine/SlateUI/Interface/OutlinerPanel/Source/OutlinerPanel.cpp \
    Engine/SlateUI/Interface/TexturePaintPanel/Source/TexturePaintPanel.cpp \
    Engine/SlateUI/Interface/CadPanel/Source/CadPanel.cpp \
    Engine/SlateUI/Interface/RasterCodec/Source/RasterCodec.cpp

BUILD := Build

.PHONY: all proof outliner validation clean

all: $(BUILD)/OutlinerHost $(BUILD)/PanelValidationHost

outliner: $(BUILD)/OutlinerHost

validation: $(BUILD)/PanelValidationHost

$(BUILD):
	mkdir -p $(BUILD) $(BUILD)/Shots

$(BUILD)/OutlinerHost: $(IMGUI_SOURCES) $(ENGINE_SOURCES) Engine/Application/OutlinerHost/Source/OutlinerHost.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(BUILD)/PanelValidationHost: $(IMGUI_SOURCES) $(ENGINE_SOURCES) Engine/Application/PanelValidationHost/Source/PanelValidationHost.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

proof: all
	mkdir -p VisualProof/OutlinerHost VisualProof/PanelValidationHost
	$(BUILD)/OutlinerHost --prefix $(BUILD)/Shots/outliner
	$(BUILD)/PanelValidationHost --prefix $(BUILD)/Shots/validation
	for Dump in $(BUILD)/Shots/outliner-*.rgba; do \
		python3 Tools/EncodeProof.py "$$Dump" "VisualProof/OutlinerHost/$$(basename "$$Dump" .rgba).png"; \
	done
	for Dump in $(BUILD)/Shots/validation-*.rgba; do \
		python3 Tools/EncodeProof.py "$$Dump" "VisualProof/PanelValidationHost/$$(basename "$$Dump" .rgba).png"; \
	done

clean:
	rm -rf $(BUILD)

check: proof
	python3 Tools/AssertProofs.py
