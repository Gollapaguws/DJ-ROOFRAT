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

    # Try to find LAME for MP3 encoding (optional)
    find_package(Lame QUIET)
    set(DJ_SIM_USE_LAME OFF)
    if(Lame_FOUND)
        set(DJ_SIM_USE_LAME ON)
    else()
        find_path(LAME_INCLUDE_DIR NAMES lame/lame.h)
        find_library(LAME_LIBRARY NAMES mp3lame libmp3lame)
        if(LAME_INCLUDE_DIR AND LAME_LIBRARY)
            set(DJ_SIM_USE_LAME ON)
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
    audio/Recorder.cpp
    audio/WAVExporter.cpp
    audio/MP3Exporter.cpp
    audio/SessionMetadata.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/EffectChain.cpp
    audio/Flanger.cpp
    audio/Phaser.cpp
    audio/Bitcrusher.cpp
    audio/RingModulator.cpp
    audio/AutoFilter.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/OnsetDetector.cpp
    audio/BeatGrid.cpp
    audio/BeatGridEditor.cpp
    audio/PhaseAligner.cpp
    audio/EnergyAnalyzer.cpp
    audio/CamelotAnalyzer.cpp
    crowdAI/CrowdStateMachine.cpp
    visuals/WaveformRenderer.cpp
    visuals/SpectrumRenderer.cpp
    visuals/BeatGridRenderer.cpp
    visuals/BeatMarkerOverlay.cpp
    visuals/EnergyHistogram.cpp
    visuals/GraphicsContext.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
    gameplay/GameModes.cpp
    gameplay/TutorialSystem.cpp
    gameplay/ProgressTracker.cpp
    gameplay/MissionSystem.cpp
    gameplay/Leaderboard.cpp
    gameplay/Venue.cpp
    gameplay/UnlockSystem.cpp
    gameplay/AchievementSystem.cpp
    gameplay/lessons/BeatmatchingLesson.cpp
    gameplay/lessons/EQMixingLesson.cpp
    gameplay/missions/SustainedBeatmatchMission.cpp
    gameplay/missions/EnergySurvivalMission.cpp
    gameplay/missions/TransitionMission.cpp
    gameplay/EnergyCurve.cpp
    gameplay/MixQualityAnalyzer.cpp
    visuals/EnergyCurveRenderer.cpp
    multiplayer/BattleMode.cpp
    multiplayer/BattleRules.cpp
    multiplayer/Judge.cpp
    core/ConfigManager.cpp
    core/SessionState.cpp
    core/PresetManager.cpp
    input/InputMapper.cpp
    input/MIDIDevice.cpp
    input/MIDIMessage.cpp
    input/MIDIController.cpp
    input/MIDIMapping.cpp
    input/MIDILearnMode.cpp
    input/ControllerProfiles.cpp
    library/TrackLibrary.cpp
    library/LibraryScanner.cpp
    library/TrackBrowser.cpp
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    # vendor/sqlite3_stub.cpp  # Using std::map implementation instead
)

set_target_properties(${DJ_ROOFRAT_TARGET} PROPERTIES
    OUTPUT_NAME "DJ-ROOFRAT"
)

target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

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

if(DJ_SIM_USE_LAME)
    target_compile_definitions(${DJ_ROOFRAT_TARGET} PRIVATE DJ_RECORDING_MP3_AVAILABLE=1)
    if(Lame_FOUND)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE Lame::Lame)
    else()
        target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${LAME_INCLUDE_DIR})
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE ${LAME_LIBRARY})
    endif()
endif()

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(${DJ_ROOFRAT_TARGET} PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

# Windows Multimedia API for MIDI input
if(WIN32)
    target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE winmm)
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
message(STATUS "DJ_SIM_USE_LAME=${DJ_SIM_USE_LAME}")
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
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
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
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
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
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
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
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(graphics_context_phase7_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(graphics_context_phase7_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(graphics_context_phase7_test PRIVATE d3d11 dxgi d3dcompiler)
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
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(crowd_renderer_phase8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(crowd_renderer_phase8_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(crowd_renderer_phase8_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

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
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(graphics_context_phase8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(graphics_context_phase8_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(graphics_context_phase8_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(graphics_context_phase8_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_context_phase8_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_context_phase8_test PROPERTIES OUTPUT_NAME "GraphicsContext_Phase8_test")

# GraphicsContext Phase 18 test (DirectX 11 Core Rendering Pipeline)
add_executable(graphics_context_phase18_test
    visuals/GraphicsContext_Phase18_test.cpp
    visuals/GraphicsContext.cpp
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(graphics_context_phase18_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(graphics_context_phase18_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(graphics_context_phase18_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(graphics_context_phase18_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_context_phase18_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_context_phase18_test PROPERTIES OUTPUT_NAME "GraphicsContext_Phase18_test")

# Lighting Phase 19 test (Advanced Lighting & Shaders)
add_executable(lighting_phase19_test
    visuals/Lighting_Phase19_test.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/GraphicsContext.cpp
    visuals/Shader.cpp
    visuals/RenderTarget.cpp
    visuals/PostProcessor.cpp
    visuals/Camera.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(lighting_phase19_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(lighting_phase19_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(lighting_phase19_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(lighting_phase19_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(lighting_phase19_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(lighting_phase19_test PROPERTIES OUTPUT_NAME "Lighting_Phase19_test")

# Particles Phase 20 test (Particle Systems & Crowd Enhancement)
add_executable(particles_phase20_test
    visuals/Particles_Phase20_test.cpp
    visuals/ParticleSystem.cpp
    visuals/ComputeShader.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
    visuals/CrowdRenderer.cpp
    visuals/GraphicsContext.cpp
    visuals/Shader.cpp
    visuals/RenderTarget.cpp
    visuals/PostProcessor.cpp
    visuals/Camera.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
)

target_include_directories(particles_phase20_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(particles_phase20_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(particles_phase20_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(particles_phase20_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(particles_phase20_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(particles_phase20_test PROPERTIES OUTPUT_NAME "Particles_Phase20_test")

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
    # vendor/sqlite3_stub.cpp  # Using std::map implementation instead
)

target_include_directories(tracklibrary_phase11_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

# Configure SQLite3 for test target - always enable with vendor stub
# target_compile_definitions(tracklibrary_phase11_test PRIVATE DJ_SIM_USE_SQLITE3=1)

if(SQLite3_FOUND)
    target_compile_definitions(tracklibrary_phase11_test PRIVATE DJ_SIM_USE_SQLITE3=1)
    if(SQLITE3_TARGET)
        target_link_libraries(tracklibrary_phase11_test PRIVATE ${SQLITE3_TARGET})
    endif()
    if(SQLITE3_INCLUDE_DIR)
        target_include_directories(tracklibrary_phase11_test PRIVATE ${SQLITE3_INCLUDE_DIR})
    elseif(SQLite3_INCLUDE_DIRS)
        target_include_directories(tracklibrary_phase11_test PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()
endif()

if(MSVC)
    target_compile_options(tracklibrary_phase11_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(tracklibrary_phase11_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(tracklibrary_phase11_test PROPERTIES OUTPUT_NAME "TrackLibrary_Phase11_test")

# MIDIController Phase 12 test (MIDI Controller Foundation)
add_executable(midicontroller_phase12_test
    input/MIDIController_Phase12_test.cpp
    input/MIDIDevice.cpp
    input/MIDIMessage.cpp
    input/MIDIController.cpp
)

target_include_directories(midicontroller_phase12_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Windows Multimedia API
if(WIN32)
    target_link_libraries(midicontroller_phase12_test PRIVATE winmm)
endif()

if(MSVC)
    target_compile_options(midicontroller_phase12_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(midicontroller_phase12_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(midicontroller_phase12_test PROPERTIES OUTPUT_NAME "MIDIController_Phase12_test")

# MIDIMapping Phase 13 test (MIDI Mapping System)
add_executable(midicontroller_phase13_test
    input/MIDIMapping_Phase13_test.cpp
    input/MIDIMapping.cpp
    input/MIDILearnMode.cpp
    input/ControllerProfiles.cpp
    input/MIDIDevice.cpp
    input/MIDIMessage.cpp
    input/MIDIController.cpp
)

target_include_directories(midicontroller_phase13_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Windows Multimedia API
if(WIN32)
    target_link_libraries(midicontroller_phase13_test PRIVATE winmm)
endif()

if(MSVC)
    target_compile_options(midicontroller_phase13_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(midicontroller_phase13_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(midicontroller_phase13_test PROPERTIES OUTPUT_NAME "MIDIController_Phase13_test")

# Recorder Phase 14 test (Performance Recording & Export)
add_executable(recorder_phase14_test
    audio/Recorder_Phase14_test.cpp
    audio/Recorder.cpp
    audio/WAVExporter.cpp
    audio/MP3Exporter.cpp
    audio/SessionMetadata.cpp
)

target_include_directories(recorder_phase14_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Configure LAME for test if available
if(DJ_SIM_USE_LAME)
    target_compile_definitions(recorder_phase14_test PRIVATE DJ_RECORDING_MP3_AVAILABLE=1)
    if(Lame_FOUND)
        target_link_libraries(recorder_phase14_test PRIVATE Lame::Lame)
    else()
        target_include_directories(recorder_phase14_test PRIVATE ${LAME_INCLUDE_DIR})
        target_link_libraries(recorder_phase14_test PRIVATE ${LAME_LIBRARY})
    endif()
endif()

if(MSVC)
    target_compile_options(recorder_phase14_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(recorder_phase14_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(recorder_phase14_test PROPERTIES OUTPUT_NAME "Recorder_Phase14_test")

# Effects Phase 15 test (Time-Based Effects: Reverb & Delay)
add_executable(effects_phase15_test
    audio/Effects_Phase15_test.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/EffectChain.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
)

target_include_directories(effects_phase15_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(effects_phase15_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(effects_phase15_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(effects_phase15_test PROPERTIES OUTPUT_NAME "Effects_Phase15_test")

# Effects Phase 16 test (Modulation & Distortion Effects)
add_executable(effects_phase16_test
    audio/Effects_Phase16_test.cpp
    audio/Flanger.cpp
    audio/Phaser.cpp
    audio/Bitcrusher.cpp
    audio/RingModulator.cpp
    audio/AutoFilter.cpp
)

target_include_directories(effects_phase16_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(effects_phase16_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(effects_phase16_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(effects_phase16_test PROPERTIES OUTPUT_NAME "Effects_Phase16_test")

# Vinyl Phase 17 test (Vinyl Simulation & Scratching)
add_executable(vinyl_phase17_test
    audio/Vinyl_Phase17_test.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
)

target_include_directories(vinyl_phase17_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(vinyl_phase17_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(vinyl_phase17_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(vinyl_phase17_test PROPERTIES OUTPUT_NAME "Vinyl_Phase17_test")

# GameplaySystem Phase 21 test (Tutorial & Training System)
add_executable(gameplay_phase21_test
    gameplay/GameplaySystem_Phase21_test.cpp
    gameplay/TutorialSystem.cpp
    gameplay/Lesson.cpp
    gameplay/lessons/BeatmatchingLesson.cpp
    gameplay/lessons/EQMixingLesson.cpp
    gameplay/ProgressTracker.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/Mixer.cpp
    audio/TrackLoader.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
)

target_include_directories(gameplay_phase21_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(gameplay_phase21_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(gameplay_phase21_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(gameplay_phase21_test PROPERTIES OUTPUT_NAME "GameplaySystem_Phase21_test")

# GameplaySystem Phase 22 test (Mission & Challenge System)
add_executable(gameplay_phase22_test
    gameplay/GameplaySystem_Phase22_test.cpp
    gameplay/MissionSystem.cpp
    gameplay/Mission.cpp
    gameplay/missions/SustainedBeatmatchMission.cpp
    gameplay/missions/EnergySurvivalMission.cpp
    gameplay/missions/TransitionMission.cpp
    gameplay/Leaderboard.cpp
)

target_include_directories(gameplay_phase22_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(gameplay_phase22_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(gameplay_phase22_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(gameplay_phase22_test PROPERTIES OUTPUT_NAME "GameplaySystem_Phase22_test")

# GameplaySystem Phase 23 test (Career Mode Expansion)
add_executable(gameplay_phase23_test
    gameplay/GameplaySystem_Phase23_test.cpp
    gameplay/Venue.cpp
    gameplay/UnlockSystem.cpp
    gameplay/AchievementSystem.cpp
    gameplay/GameModes.cpp
)

target_include_directories(gameplay_phase23_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(gameplay_phase23_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(gameplay_phase23_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(gameplay_phase23_test PROPERTIES OUTPUT_NAME "GameplaySystem_Phase23_test")

# Multiplayer Phase 24 test (Terminal Hotseat Battles)
add_executable(multiplayer_phase24_test
    multiplayer/BattleSystem_Phase24_test.cpp
    multiplayer/BattleMode.cpp
    multiplayer/BattleRules.cpp
    multiplayer/Judge.cpp
    input/InputMapper.cpp
)

target_include_directories(multiplayer_phase24_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(multiplayer_phase24_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(multiplayer_phase24_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(multiplayer_phase24_test PROPERTIES OUTPUT_NAME "BattleSystem_Phase24_test")

# ConfigSystem Phase 25 test (Configuration & State Management)
add_executable(config_system_phase25_test
    core/ConfigSystem_Phase25_test.cpp
    core/ConfigManager.cpp
    core/SessionState.cpp
    core/PresetManager.cpp
)

target_include_directories(config_system_phase25_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(config_system_phase25_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(config_system_phase25_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(config_system_phase25_test PROPERTIES OUTPUT_NAME "ConfigSystem_Phase25_test")

# SessionAutoSave Phase 26 test (Session Auto-Save & Recovery)
add_executable(session_autosave_phase26_test
    core/SessionAutoSave_Phase26_test.cpp
    core/SessionState.cpp
    core/ConfigManager.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/Mixer.cpp
    audio/BPMDetector.cpp
    audio/TrackLoader.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/EffectChain.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/Flanger.cpp
    audio/Phaser.cpp
    audio/Bitcrusher.cpp
    audio/RingModulator.cpp
    audio/AutoFilter.cpp
    gameplay/GameModes.cpp
    gameplay/ProgressTracker.cpp
    gameplay/Venue.cpp
)

target_include_directories(session_autosave_phase26_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(session_autosave_phase26_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(session_autosave_phase26_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(session_autosave_phase26_test PROPERTIES OUTPUT_NAME "SessionAutoSave_Phase26_test")

# PresetHotkeys Phase 27 test (F1-F12 Preset Hotkey System)
add_executable(preset_hotkeys_phase27_test
    input/PresetHotkeys_Phase27_test.cpp
    input/InputMapper.cpp
    core/PresetManager.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/BPMDetector.cpp
    audio/TrackLoader.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/EffectChain.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/Flanger.cpp
    audio/Phaser.cpp
    audio/Bitcrusher.cpp
    audio/RingModulator.cpp
    audio/AutoFilter.cpp
)

target_include_directories(preset_hotkeys_phase27_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(preset_hotkeys_phase27_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(preset_hotkeys_phase27_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(preset_hotkeys_phase27_test PROPERTIES OUTPUT_NAME "PresetHotkeys_Phase27_test")

# Enhanced Recording Phase 29 test (Multi-track Recording with Cue Markers)
add_executable(enhanced_recording_phase29_test
    audio/EnhancedRecording_Phase29_test.cpp
    audio/Recorder.cpp
    audio/WAVExporter.cpp
    audio/SessionMetadata.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/BPMDetector.cpp
    audio/TrackLoader.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/EffectChain.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/Mixer.cpp
)

target_include_directories(enhanced_recording_phase29_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(enhanced_recording_phase29_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced_recording_phase29_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced_recording_phase29_test PROPERTIES OUTPUT_NAME "EnhancedRecording_Phase29_test")

# Spectrum Analyzer Phase 30 test (Real-time FFT & Frequency Analysis)
add_executable(spectrum_analyzer_phase30_test
    audio/SpectrumAnalyzer_Phase30_test.cpp
    audio/SpectrumAnalyzer.cpp
    visuals/SpectrumRenderer.cpp
)

target_include_directories(spectrum_analyzer_phase30_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(spectrum_analyzer_phase30_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(spectrum_analyzer_phase30_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(spectrum_analyzer_phase30_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(spectrum_analyzer_phase30_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(spectrum_analyzer_phase30_test PROPERTIES OUTPUT_NAME "SpectrumAnalyzer_Phase30_test")

# Camelot Analyzer Phase 31 test (Harmonic Mixing - Camelot Wheel)
add_executable(camelot_analyzer_phase31_test
    audio/CamelotAnalyzer_Phase31_test.cpp
    audio/CamelotAnalyzer.cpp
    visuals/CamelotWheel.cpp
    library/TrackBrowser.cpp
    library/TrackLibrary.cpp
    library/LibraryScanner.cpp
)

target_include_directories(camelot_analyzer_phase31_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link SQLite3 if available (TrackLibrary dependency)
if(SQLite3_FOUND)
    target_compile_definitions(camelot_analyzer_phase31_test PRIVATE DJ_SIM_USE_SQLITE3=1)
    if(SQLITE3_TARGET)
        target_link_libraries(camelot_analyzer_phase31_test PRIVATE ${SQLITE3_TARGET})
    endif()
    if(SQLITE3_INCLUDE_DIR)
        target_include_directories(camelot_analyzer_phase31_test PRIVATE ${SQLITE3_INCLUDE_DIR})
    elseif(SQLite3_INCLUDE_DIRS)
        target_include_directories(camelot_analyzer_phase31_test PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()
endif()

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(camelot_analyzer_phase31_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(camelot_analyzer_phase31_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(camelot_analyzer_phase31_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(camelot_analyzer_phase31_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(camelot_analyzer_phase31_test PROPERTIES OUTPUT_NAME "CamelotAnalyzer_Phase31_test")

# BeatGrid Phase 32 test (Phrase Detection & Beat Grid Editor)
add_executable(beat_grid_phase32_test
    audio/BeatGrid_Phase32_test.cpp
    audio/OnsetDetector.cpp
    audio/BeatGrid.cpp
    visuals/BeatGridRenderer.cpp
)

target_include_directories(beat_grid_phase32_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(beat_grid_phase32_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(beat_grid_phase32_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(beat_grid_phase32_test PROPERTIES OUTPUT_NAME "BeatGrid_Phase32_test")

# Energy Analyzer Phase 33 test (Track Energy Rating System)
add_executable(energy_analyzer_phase33_test
    audio/EnergyAnalyzer_Phase33_test.cpp
    audio/EnergyAnalyzer.cpp
    audio/CamelotAnalyzer.cpp
    visuals/EnergyHistogram.cpp
    library/TrackBrowser.cpp
    library/TrackLibrary.cpp
    library/LibraryScanner.cpp
)

target_include_directories(energy_analyzer_phase33_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link SQLite3 if available (TrackLibrary dependency)
if(SQLite3_FOUND)
    target_compile_definitions(energy_analyzer_phase33_test PRIVATE DJ_SIM_USE_SQLITE3=1)
    if(SQLITE3_TARGET)
        target_link_libraries(energy_analyzer_phase33_test PRIVATE ${SQLITE3_TARGET})
    endif()
    if(SQLITE3_INCLUDE_DIR)
        target_include_directories(energy_analyzer_phase33_test PRIVATE ${SQLITE3_INCLUDE_DIR})
    elseif(SQLite3_INCLUDE_DIRS)
        target_include_directories(energy_analyzer_phase33_test PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()
endif()

if(MSVC)
    target_compile_options(energy_analyzer_phase33_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(energy_analyzer_phase33_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(energy_analyzer_phase33_test PROPERTIES OUTPUT_NAME "EnergyAnalyzer_Phase33_test")

# FFT Phase 34 test (Cooley-Tukey FFT Implementation)
add_executable(fft_phase34_test
    audio/FFT_Phase34_test.cpp
    audio/FFTEngine.cpp
    audio/SpectrumAnalyzer.cpp
)

target_include_directories(fft_phase34_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(fft_phase34_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(fft_phase34_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(fft_phase34_test PROPERTIES OUTPUT_NAME "FFT_Phase34_test")

# Spectrum Display Phase 35 test (Real-time Spectrum Visualization)
add_executable(spectrum_display_phase35_test
    audio/SpectrumDisplay_Phase35_test.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    visuals/SpectrumRenderer.cpp
)

target_include_directories(spectrum_display_phase35_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(spectrum_display_phase35_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(spectrum_display_phase35_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(spectrum_display_phase35_test PROPERTIES OUTPUT_NAME "SpectrumDisplay_Phase35_test")

# Beat Grid Editor Phase 36 test (Beat Grid Nudge Editor)
add_executable(beatgrid_editor_phase36_test
    audio/BeatGridEditor_Phase36_test.cpp
    audio/BeatGridEditor.cpp
    audio/BeatGrid.cpp
    visuals/BeatMarkerOverlay.cpp
)

target_include_directories(beatgrid_editor_phase36_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(beatgrid_editor_phase36_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(beatgrid_editor_phase36_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(beatgrid_editor_phase36_test PROPERTIES OUTPUT_NAME "BeatGridEditor_Phase36_test")

# Energy Curve & Mix Quality Scoring Phase 37 test (Final Arc IX Phase)
add_executable(energycurve_phase37_test
    gameplay/EnergyCurve_Phase37_test.cpp
    gameplay/EnergyCurve.cpp
    gameplay/MixQualityAnalyzer.cpp
    visuals/EnergyCurveRenderer.cpp
    audio/CamelotAnalyzer.cpp
)

target_include_directories(energycurve_phase37_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(energycurve_phase37_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(energycurve_phase37_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(energycurve_phase37_test PROPERTIES OUTPUT_NAME "EnergyCurve_Phase37_test")

# Phase Alignment Engine Phase 38 test (Arc X Phase 1)
add_executable(phasealigner_phase38_test
    audio/PhaseAligner_Phase38_test.cpp
    audio/PhaseAligner.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/BeatGrid.cpp
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/Flanger.cpp
    audio/Phaser.cpp
    audio/Bitcrusher.cpp
    audio/RingModulator.cpp
    audio/AutoFilter.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/BPMDetector.cpp
    audio/KeyDetector.cpp
    audio/MetadataParser.cpp
)

target_include_directories(phasealigner_phase38_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(phasealigner_phase38_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(phasealigner_phase38_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(phasealigner_phase38_test PROPERTIES OUTPUT_NAME "PhaseAligner_Phase38_test")
