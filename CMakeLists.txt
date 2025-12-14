cmake_minimum_required(VERSION 3.15)
project(TurntableMIDI VERSION 1.0.0)

# Add JUCE as a subdirectory
add_subdirectory(../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Create binary data from images
juce_add_binary_data(TurntableMIDIBinaryData
    SOURCES
        viking_full.png
        viking_head.png
        switch_toggle.png
        knob_simplegray.png
        wallpaper.jpg
)

# Create the plugin target
juce_add_plugin(TurntableMIDI
    COMPANY_NAME "BeowulfAudio"
    PLUGIN_MANUFACTURER_CODE Bwlf
    PLUGIN_CODE Skld
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Skald"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Instrument" "Generator"
    PLUGIN_MANUFACTURER_EMAIL "support@beowulfaudio.com"
)

# Add source files
target_sources(TurntableMIDI PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# Compiler definitions
target_compile_definitions(TurntableMIDI PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link required JUCE modules
target_link_libraries(TurntableMIDI PRIVATE
    TurntableMIDIBinaryData
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# C++ standard
target_compile_features(TurntableMIDI PRIVATE cxx_std_17)
