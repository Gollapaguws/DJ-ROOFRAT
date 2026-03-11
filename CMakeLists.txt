cmake_minimum_required(VERSION 3.20)

# vcpkg toolchain is set automatically via CMAKE_TOOLCHAIN_FILE env variable or command line

project(DJ_ROOFRAT VERSION 0.1.0 LANGUAGES CXX)

set(DJ_ROOFRAT_TARGET dj_roofrat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(DJ_SIM_ENABLE_AUDIO_BACKEND "Enable PortAudio and libsndfile when available" ON)
option(DJROOFRAT_ENABLE_GRAPHICS "Enable DirectX 11 visuals" OFF)

set(DJ_SIM_USE_PORTAUDIO OFF)
set(DJ_SIM_USE_SNDFILE OFF)
set(DJ_SIM_USE_SQLITE3 ON)

# SQLite3 setup - try multiple methods to find it
set(SQLITE3_TARGET "")
set(SQLITE3_INCLUDE_DIR "")

# Method 1: Try find_package
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    message(STATUS "SQLite3 found via find_package: ${SQLite3_LIBRARIES}")
    set(SQLITE3_TARGET SQLite3::SQLite3)
    if(NOT SQLite3_INCLUDE_DIRS)
        find_path(SQLite3_INCLUDE_DIRS NAMES sqlite3.h)
    endif()
endif()

# Method 2: Try pkg-config
if(NOT SQLite3_FOUND AND PKG_CONFIG_FOUND)
    pkg_check_modules(SQLite3 QUIET sqlite3)
    if(SQLite3_FOUND)
        message(STATUS "SQLite3 found via pkg-config: ${SQLite3_LIBRARIES}")
        set(SQLITE3_TARGET ${SQLite3_LIBRARIES})
        set(SQLITE3_INCLUDE_DIR ${SQLite3_INCLUDE_DIRS})
    endif()
endif()

# Method 3: Try find_library and find_path directly
if(NOT SQLite3_FOUND)
    find_path(SQLITE3_INCLUDE_DIR NAMES sqlite3.h)
    find_library(SQLITE3_LIBRARY NAMES sqlite3 sqlite libsqlite3)
    
    if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
        message(STATUS "SQLite3 found via find_library: ${SQLITE3_LIBRARY}")
        set(SQLite3_FOUND TRUE)
        set(SQLITE3_TARGET ${SQLITE3_LIBRARY})
    endif()
endif()

if(NOT SQLite3_FOUND)
    message(WARNING "SQLite3 not found on system. Trying to fetch from GitHub...")
    # Try to download and build the amalgamation... (optional fallback)
endif()

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
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/PortAudioPlayer.cpp
    crowdAI/CrowdStateMachine.cpp
    visuals/WaveformRenderer.cpp
    visuals/GraphicsContext.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    gameplay/GameModes.cpp
    input/InputMapper.cpp
    library/TrackLibrary.cpp
    library/LibraryScanner.cpp
    library/TrackBrowser.cpp
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

# Configure SQLite3 for main target
if(SQLite3_FOUND)
    target_compile_definitions(${DJ_ROOFRAT_TARGET} PRIVATE DJ_SIM_USE_SQLITE3=1)
    if(SQLITE3_TARGET)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE ${SQLITE3_TARGET})
    endif()
    if(SQLITE3_INCLUDE_DIR)
        target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${SQLITE3_INCLUDE_DIR})
    elseif(SQLite3_INCLUDE_DIRS)
        target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()
else()
    message(WARNING "SQLite3 not found - TrackLibrary will not have database support")
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
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
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
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
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
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
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

# TrackLoader Phase 9 test (MP3 support and comprehensive audio loading)
add_executable(trackloader_phase9_test
    audio/TrackLoader_Phase9_test.cpp
    audio/TrackLoader.cpp
    audio/AudioClip.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
)

target_include_directories(trackloader_phase9_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJ_SIM_USE_SNDFILE)
    target_compile_definitions(trackloader_phase9_test PRIVATE DJ_SIM_USE_SNDFILE=1)
    if(SNDFILE_TARGET)
        target_link_libraries(trackloader_phase9_test PRIVATE ${SNDFILE_TARGET})
    else()
        target_include_directories(trackloader_phase9_test PRIVATE ${SNDFILE_INCLUDE_DIR})
        target_link_libraries(trackloader_phase9_test PRIVATE ${SNDFILE_LIBRARY})
    endif()
endif()

if(MSVC)
    target_compile_options(trackloader_phase9_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(trackloader_phase9_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(trackloader_phase9_test PROPERTIES OUTPUT_NAME "TrackLoader_Phase9_test")

# TrackLoader Phase 10 test (Track Metadata & BPM Detection)
add_executable(trackloader_phase10_test
    audio/TrackLoader_Phase10_test.cpp
    audio/TrackLoader.cpp
    audio/AudioClip.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/BPMDetector.cpp
)

target_include_directories(trackloader_phase10_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJ_SIM_USE_SNDFILE)
    target_compile_definitions(trackloader_phase10_test PRIVATE DJ_SIM_USE_SNDFILE=1)
    if(SNDFILE_TARGET)
        target_link_libraries(trackloader_phase10_test PRIVATE ${SNDFILE_TARGET})
    else()
        target_include_directories(trackloader_phase10_test PRIVATE ${SNDFILE_INCLUDE_DIR})
        target_link_libraries(trackloader_phase10_test PRIVATE ${SNDFILE_LIBRARY})
    endif()
endif()

if(MSVC)
    target_compile_options(trackloader_phase10_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(trackloader_phase10_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(trackloader_phase10_test PROPERTIES OUTPUT_NAME "TrackLoader_Phase10_test")

# TrackLibrary Phase 11 test (Track Library & Browser)
add_executable(tracklibrary_phase11_test
    library/TrackLibrary_Phase11_test.cpp
    library/TrackLibrary.cpp
    library/LibraryScanner.cpp
    library/TrackBrowser.cpp
)

target_include_directories(tracklibrary_phase11_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
    target_compile_options(tracklibrary_phase11_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(tracklibrary_phase11_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(tracklibrary_phase11_test PROPERTIES OUTPUT_NAME "TrackLibrary_Phase11_test")
