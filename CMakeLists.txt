cmake_minimum_required(VERSION 3.20)
project(DJ_ROOFRAT VERSION 0.1.0 LANGUAGES CXX)

set(DJ_ROOFRAT_TARGET dj_roofrat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DJ_SIM_ENABLE_AUDIO_BACKEND "Enable PortAudio and libsndfile when available" ON)
option(DJROOFRAT_ENABLE_GRAPHICS "Enable DirectX 11 visuals" OFF)

set(DJ_SIM_USE_PORTAUDIO OFF)
set(DJ_SIM_USE_SNDFILE OFF)

if(DJ_SIM_ENABLE_AUDIO_BACKEND)
    find_path(PORTAUDIO_INCLUDE_DIR NAMES portaudio.h)
    find_library(PORTAUDIO_LIBRARY NAMES portaudio portaudio_x64)
    if(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)
        set(DJ_SIM_USE_PORTAUDIO ON)
    endif()

    find_package(SndFile QUIET)
    if(SndFile_FOUND)
        set(DJ_SIM_USE_SNDFILE ON)
        set(SNDFILE_TARGET SndFile::sndfile)
    else()
        find_path(SNDFILE_INCLUDE_DIR NAMES sndfile.h)
        find_library(SNDFILE_LIBRARY NAMES sndfile sndfile-1 libsndfile-1)
        if(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
            set(DJ_SIM_USE_SNDFILE ON)
        endif()
    endif()
endif()

add_executable(${DJ_ROOFRAT_TARGET}
    src/main.cpp
    audio/AudioClip.cpp
    audio/TrackLoader.cpp
    audio/ThreeBandEQ.cpp
    audio/Deck.cpp
    audio/Mixer.cpp
    audio/BPMDetector.cpp
    audio/PortAudioPlayer.cpp
    crowdAI/CrowdStateMachine.cpp
    visuals/WaveformRenderer.cpp
    visuals/GraphicsContext.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    gameplay/GameModes.cpp
    input/InputMapper.cpp
)

set_target_properties(${DJ_ROOFRAT_TARGET} PROPERTIES
    OUTPUT_NAME "DJ-ROOFRAT"
)

target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJ_SIM_USE_PORTAUDIO)
    target_compile_definitions(${DJ_ROOFRAT_TARGET} PRIVATE DJ_SIM_USE_PORTAUDIO=1)
    target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${PORTAUDIO_INCLUDE_DIR})
    target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE ${PORTAUDIO_LIBRARY})
endif()

if(DJ_SIM_USE_SNDFILE)
    target_compile_definitions(${DJ_ROOFRAT_TARGET} PRIVATE DJ_SIM_USE_SNDFILE=1)
    if(SNDFILE_TARGET)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE ${SNDFILE_TARGET})
    else()
        target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${SNDFILE_INCLUDE_DIR})
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE ${SNDFILE_LIBRARY})
    endif()
endif()

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(${DJ_ROOFRAT_TARGET} PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE d3d11 dxgi)
    endif()
endif()

if(MSVC)
    target_compile_options(${DJ_ROOFRAT_TARGET} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${DJ_ROOFRAT_TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "DJ_SIM_USE_PORTAUDIO=${DJ_SIM_USE_PORTAUDIO}")
message(STATUS "DJ_SIM_USE_SNDFILE=${DJ_SIM_USE_SNDFILE}")
message(STATUS "DJROOFRAT_ENABLE_GRAPHICS=${DJROOFRAT_ENABLE_GRAPHICS}")

# Test targets
add_executable(deck_phase2_test
    audio/Deck_Phase2_test.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
)

target_include_directories(deck_phase2_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(deck_phase2_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(deck_phase2_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(deck_phase2_test PROPERTIES OUTPUT_NAME "Deck_Phase2_test")

# InputMapper Phase 2 test
add_executable(input_mapper_phase2_test
    input/InputMapper_Phase2_test.cpp
    input/InputMapper.cpp
)

target_include_directories(input_mapper_phase2_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(input_mapper_phase2_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(input_mapper_phase2_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(input_mapper_phase2_test PROPERTIES OUTPUT_NAME "InputMapper_Phase2_test")

# Deck Phase 3 test
add_executable(deck_phase3_test
    audio/Deck_Phase3_test.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
)

target_include_directories(deck_phase3_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(deck_phase3_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(deck_phase3_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(deck_phase3_test PROPERTIES OUTPUT_NAME "Deck_Phase3_test")

# InputMapper Phase 3 test
add_executable(input_mapper_phase3_test
    input/InputMapper_Phase3_test.cpp
    input/InputMapper.cpp
)

target_include_directories(input_mapper_phase3_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(input_mapper_phase3_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(input_mapper_phase3_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(input_mapper_phase3_test PROPERTIES OUTPUT_NAME "InputMapper_Phase3_test")

# Deck Phase 4 test
add_executable(deck_phase4_test
    audio/Deck_Phase4_test.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
)

target_include_directories(deck_phase4_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(deck_phase4_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(deck_phase4_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(deck_phase4_test PROPERTIES OUTPUT_NAME "Deck_Phase4_test")

# InputMapper Phase 4 test
add_executable(input_mapper_phase4_test
    input/InputMapper_Phase4_test.cpp
    input/InputMapper.cpp
)

target_include_directories(input_mapper_phase4_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(input_mapper_phase4_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(input_mapper_phase4_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(input_mapper_phase4_test PROPERTIES OUTPUT_NAME "InputMapper_Phase4_test")

# CrowdStateMachine Phase 5 test
add_executable(crowd_phase5_test
    crowdAI/CrowdStateMachine_Phase5_test.cpp
    crowdAI/CrowdStateMachine.cpp
)

target_include_directories(crowd_phase5_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(crowd_phase5_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(crowd_phase5_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(crowd_phase5_test PROPERTIES OUTPUT_NAME "CrowdStateMachine_Phase5_test")

# CrowdStateMachine Phase 6 test
add_executable(crowd_phase6_test
    crowdAI/CrowdStateMachine_Phase6_test.cpp
    crowdAI/CrowdStateMachine.cpp
)

target_include_directories(crowd_phase6_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(crowd_phase6_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(crowd_phase6_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(crowd_phase6_test PROPERTIES OUTPUT_NAME "CrowdStateMachine_Phase6_test")

# GraphicsContext Phase 7 test
add_executable(graphics_context_phase7_test
    visuals/GraphicsContext_Phase7_test.cpp
    visuals/GraphicsContext.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
)

target_include_directories(graphics_context_phase7_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(graphics_context_phase7_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(graphics_context_phase7_test PRIVATE d3d11 dxgi)
    endif()
endif()

if(MSVC)
    target_compile_options(graphics_context_phase7_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_context_phase7_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_context_phase7_test PROPERTIES OUTPUT_NAME "GraphicsContext_Phase7_test")

# LightingRig Phase 7 test
add_executable(lighting_rig_phase7_test
    visuals/LightingRig_Phase7_test.cpp
    visuals/LightingRig.cpp
)

target_include_directories(lighting_rig_phase7_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(lighting_rig_phase7_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(lighting_rig_phase7_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(lighting_rig_phase7_test PROPERTIES OUTPUT_NAME "LightingRig_Phase7_test")

# LaserController Phase 8 test
add_executable(laser_controller_phase8_test
    visuals/LaserController_Phase8_test.cpp
    visuals/LaserController.cpp
)

target_include_directories(laser_controller_phase8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(laser_controller_phase8_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(laser_controller_phase8_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(laser_controller_phase8_test PROPERTIES OUTPUT_NAME "LaserController_Phase8_test")

# CrowdRenderer Phase 8 test
add_executable(crowd_renderer_phase8_test
    visuals/CrowdRenderer_Phase8_test.cpp
    visuals/CrowdRenderer.cpp
)

target_include_directories(crowd_renderer_phase8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(crowd_renderer_phase8_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(crowd_renderer_phase8_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(crowd_renderer_phase8_test PROPERTIES OUTPUT_NAME "CrowdRenderer_Phase8_test")

# GraphicsContext Phase 8 test
add_executable(graphics_context_phase8_test
    visuals/GraphicsContext_Phase8_test.cpp
    visuals/GraphicsContext.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
)

target_include_directories(graphics_context_phase8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(graphics_context_phase8_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(graphics_context_phase8_test PRIVATE d3d11 dxgi)
    endif()
endif()

if(MSVC)
    target_compile_options(graphics_context_phase8_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_context_phase8_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_context_phase8_test PROPERTIES OUTPUT_NAME "GraphicsContext_Phase8_test")
