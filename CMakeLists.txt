cmake_minimum_required(VERSION 3.20)

# vcpkg toolchain is set automatically via CMAKE_TOOLCHAIN_FILE env variable or command line

project(DJ_ROOFRAT VERSION 0.1.0 LANGUAGES CXX)

set(DJ_ROOFRAT_TARGET dj_roofrat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable parallel PDB writes for MSVC
if(MSVC)
    add_compile_options(/FS)
endif()

include(FetchContent)

# ===== OpenGL Dependencies (Phase 1 Migration) =====

# Fetch GLFW for windowing and OpenGL context
FetchContent_Declare(
    glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw3)

# Fetch GLM for mathematics (header-only)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.0
)
FetchContent_MakeAvailable(glm)

# GLAD will be added via vendor/glad/ (generated locally)
set(GLAD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include")
set(GLAD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/glad.c")

# Fetch ImGui for GUI
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library with backend selection
option(DJROOFRAT_OPENGL_MIGRATION "Use OpenGL3 backend for ImGui (Phase 7A)" OFF)

if(DJROOFRAT_OPENGL_MIGRATION)
    # OpenGL3 + GLFW backend configuration
    add_library(imgui_impl STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )
    target_include_directories(imgui_impl PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${GLAD_INCLUDE_DIR}
    )
    target_link_libraries(imgui_impl PUBLIC glfw glm::glm)
    if(EXISTS "${GLAD_SRC}")
        target_sources(imgui_impl PRIVATE "${GLAD_SRC}")
    endif()
elseif(WIN32)
    # DirectX 11 + Win32 backend configuration (original)
    add_library(imgui_impl STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    )
    target_include_directories(imgui_impl PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_impl PUBLIC d3d11 dxgi)
endif()

option(DJ_SIM_ENABLE_AUDIO_BACKEND "Enable PortAudio and libsndfile when available" ON)
option(DJROOFRAT_ENABLE_GRAPHICS "Enable DirectX 11 visuals" ON)

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
    if(TARGET SQLite::SQLite3)
        set(SQLITE3_TARGET SQLite::SQLite3)
    elseif(TARGET SQLite3::SQLite3)
        set(SQLITE3_TARGET SQLite3::SQLite3)
    elseif(SQLite3_LIBRARIES)
        set(SQLITE3_TARGET ${SQLite3_LIBRARIES})
    endif()

    if(SQLite3_INCLUDE_DIRS)
        set(SQLITE3_INCLUDE_DIR ${SQLite3_INCLUDE_DIRS})
    elseif(SQLite3_INCLUDE_DIR)
        set(SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDE_DIR})
        set(SQLITE3_INCLUDE_DIR ${SQLite3_INCLUDE_DIR})
    else()
        find_path(SQLite3_INCLUDE_DIRS NAMES sqlite3.h)
        if(SQLite3_INCLUDE_DIRS)
            set(SQLITE3_INCLUDE_DIR ${SQLite3_INCLUDE_DIRS})
        endif()
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
    message(STATUS "SQLite3 not found on system. Continuing with in-memory TrackLibrary fallback.")
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
    audio/SyncController.cpp
    audio/SyncUndoStack.cpp
    audio/EnergyAnalyzer.cpp
    audio/CamelotAnalyzer.cpp
    crowdAI/CrowdStateMachine.cpp
    visuals/WaveformRenderer.cpp
    visuals/SpectrumRenderer.cpp
    visuals/BeatGridRenderer.cpp
    visuals/BeatMarkerOverlay.cpp
    visuals/EnergyHistogram.cpp
    visuals/SplashScreenAnimator.cpp
    visuals/GraphicsContext.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
    visuals/CamelotWheel.cpp
    visuals/ParticleSystem.cpp
    visuals/RenderTarget.cpp
    visuals/PostProcessor.cpp
    visuals/ComputeShader.cpp
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
    gameplay/TransitionCoach.cpp
    visuals/EnergyCurveRenderer.cpp
    visuals/CoachingHUD.cpp
    visuals/SyncIndicator.cpp
    visuals/DJControllerGeometry.cpp
    visuals/ControllerInteraction.cpp
    visuals/RayCaster.cpp
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
    visuals/TextureManager.cpp
    visuals/StageGeometry.cpp
    visuals/TunnelGeometry.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
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
    target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${GLAD_INCLUDE_DIR})
    
    # Add GLFW, GLM for OpenGL support
    target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE glfw glm::glm)
    
    # Add GLAD source for OpenGL function loading
    if(EXISTS "${GLAD_SRC}")
        target_sources(${DJ_ROOFRAT_TARGET} PRIVATE "${GLAD_SRC}")
    endif()
    
    if(WIN32)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE d3d11 dxgi d3dcompiler imgui_impl opengl32)
        target_include_directories(${DJ_ROOFRAT_TARGET} PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    elseif(APPLE)
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE "-framework OpenGL")
    else()
        target_link_libraries(${DJ_ROOFRAT_TARGET} PRIVATE OpenGL::OpenGL)
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
    message(STATUS "SQLite3 not found - TrackLibrary database support is disabled")
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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

# Phase 7B: Particle GLSL compute shader test
add_executable(particleshader_phase7b_test
    tests/ParticleShader_Phase7B_test.cpp
)
target_include_directories(particleshader_phase7b_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GLAD_INCLUDE_DIR})
target_compile_definitions(particleshader_phase7b_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1 DJROOFRAT_OPENGL_MIGRATION=1)
target_link_libraries(particleshader_phase7b_test PRIVATE glfw glm::glm)
if(EXISTS "${GLAD_SRC}")
    target_sources(particleshader_phase7b_test PRIVATE "${GLAD_SRC}")
endif()
if(WIN32)
    target_link_libraries(particleshader_phase7b_test PRIVATE opengl32)
else()
    target_link_libraries(particleshader_phase7b_test PRIVATE OpenGL::OpenGL)
endif()
set_target_properties(particleshader_phase7b_test PROPERTIES OUTPUT_NAME "ParticleShader_Phase7B_test")

# Phase 7C: Post-Processing GLSL shaders test
add_executable(postprocessing_phase7c_test
    tests/PostProcessing_Phase7C_test.cpp
)
target_include_directories(postprocessing_phase7c_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GLAD_INCLUDE_DIR})
target_compile_definitions(postprocessing_phase7c_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1 DJROOFRAT_OPENGL_MIGRATION=1)
target_link_libraries(postprocessing_phase7c_test PRIVATE glfw glm::glm)
if(EXISTS "${GLAD_SRC}")
    target_sources(postprocessing_phase7c_test PRIVATE "${GLAD_SRC}")
endif()
if(WIN32)
    target_link_libraries(postprocessing_phase7c_test PRIVATE opengl32)
else()
    target_link_libraries(postprocessing_phase7c_test PRIVATE OpenGL::OpenGL)
endif()
set_target_properties(postprocessing_phase7c_test PROPERTIES OUTPUT_NAME "PostProcessing_Phase7C_test")

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
    visuals/LightingRig.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TextureManager.cpp
    visuals/TunnelGeometry.cpp
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
    visuals/LightingRig.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TextureManager.cpp
    visuals/TunnelGeometry.cpp
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
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TextureManager.cpp
    visuals/TunnelGeometry.cpp
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
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TextureManager.cpp
    visuals/TunnelGeometry.cpp
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
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TextureManager.cpp
    visuals/TunnelGeometry.cpp
    visuals/Shader.cpp
    visuals/RenderTarget.cpp
    visuals/PostProcessor.cpp
    visuals/Camera.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/StageGeometry.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/DJControllerGeometry.cpp
    visuals/RayCaster.cpp
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

# Enhanced3DScene Phase 1 test (Music-Reactive 3D Visuals Framework)
add_executable(enhanced3dscene_phase1_test
    visuals/Enhanced3DScene_Phase1_test.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TunnelGeometry.cpp
    visuals/TextureManager.cpp
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

target_include_directories(enhanced3dscene_phase1_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(enhanced3dscene_phase1_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(enhanced3dscene_phase1_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_phase1_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_phase1_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_phase1_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_Phase1_test")

# Enhanced3DScene Phase 2 test (Enhanced Shader and Material Buffer Integration)
add_executable(enhanced3dscene_phase2_test
    visuals/Enhanced3DScene_Phase2_test.cpp
    visuals/Enhanced3DScene.cpp
    visuals/DJControllerGeometry.cpp
    visuals/ShadowMap.cpp
    visuals/TunnelGeometry.cpp
    visuals/TextureManager.cpp
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

target_include_directories(enhanced3dscene_phase2_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(enhanced3dscene_phase2_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(enhanced3dscene_phase2_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_phase2_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_phase2_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_phase2_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_Phase2_test")

# Enhanced3DScene InputLayout test (Input Layout Binding for 3D Controller Visibility)
add_executable(enhanced3dscene_inputlayout_test
    tests/Enhanced3DScene_InputLayout_test.cpp
    visuals/Enhanced3DScene.cpp
    visuals/DJControllerGeometry.cpp
    visuals/ShadowMap.cpp
    visuals/TunnelGeometry.cpp
    visuals/TextureManager.cpp
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
    visuals/LightingRig.cpp
    visuals/ParticleSystem.cpp
    visuals/RenderTarget.cpp
    visuals/PostProcessor.cpp
    visuals/RayCaster.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/Mixer.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/MetadataParser.cpp
    audio/BPMDetector.cpp
    audio/KeyDetector.cpp
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
)

target_include_directories(enhanced3dscene_inputlayout_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(enhanced3dscene_inputlayout_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(enhanced3dscene_inputlayout_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_inputlayout_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_inputlayout_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_inputlayout_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_InputLayout_test")

# Enhanced3DScene Phase 3 test (UV-based texture mapping with procedural checkerboard)
add_executable(enhanced3dscene_phase3_test
    tests/Enhanced3DScene_Phase3_test.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TunnelGeometry.cpp
    visuals/GraphicsContext.cpp
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/TextureManager.cpp
    visuals/StageGeometry.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(enhanced3dscene_phase3_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(enhanced3dscene_phase3_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(enhanced3dscene_phase3_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_phase3_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_phase3_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_phase3_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_Phase3_test")

# Enhanced3DScene Phase 4 test (Procedural tunnel geometry with scrolling UVs and beat distortion)
add_executable(enhanced3dscene_phase4_test
    tests/Enhanced3DScene_Phase4_test.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TunnelGeometry.cpp
    visuals/GraphicsContext.cpp
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/TextureManager.cpp
    visuals/StageGeometry.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(enhanced3dscene_phase4_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(enhanced3dscene_phase4_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(enhanced3dscene_phase4_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_phase4_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_phase4_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_phase4_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_Phase4_test")

# Enhanced3DScene Phase 5 test (Shadow mapping with depth texture pass and PCF)
add_executable(enhanced3dscene_phase5_test
    tests/Enhanced3DScene_Phase5_test.cpp
    visuals/Enhanced3DScene.cpp
    visuals/ShadowMap.cpp
    visuals/TunnelGeometry.cpp
    visuals/GraphicsContext.cpp
    visuals/Camera.cpp
    visuals/Shader.cpp
    visuals/VertexBuffer.cpp
    visuals/IndexBuffer.cpp
    visuals/TextureManager.cpp
    visuals/StageGeometry.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdMesh.cpp
    visuals/CrowdAnimator.cpp
)

target_include_directories(enhanced3dscene_phase5_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(enhanced3dscene_phase5_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
    if(WIN32)
        target_link_libraries(enhanced3dscene_phase5_test PRIVATE d3d11 dxgi d3dcompiler)
    endif()
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_phase5_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_phase5_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_phase5_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_Phase5_test")

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
    audio/CamelotAnalyzer.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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
    audio/FFTEngine.cpp
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
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
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

# Auto-Tempo Matching & Sync Lock Phase 39 test (Arc X Phase 2)
add_executable(synccontroller_phase39_test
    audio/SyncController_Phase39_test.cpp
    audio/SyncController.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/BPMDetector.cpp
    audio/KeyDetector.cpp
    audio/MetadataParser.cpp
)

target_include_directories(synccontroller_phase39_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(synccontroller_phase39_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(synccontroller_phase39_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(synccontroller_phase39_test PROPERTIES OUTPUT_NAME "SyncController_Phase39_test")

# TransitionCoach Phase 40 test (Transition Coaching & Phrasing)
add_executable(transitioncoach_phase40_test
    gameplay/TransitionCoach_Phase40_test.cpp
    gameplay/TransitionCoach.cpp
    gameplay/EnergyCurve.cpp
    visuals/CoachingHUD.cpp
    audio/BeatGrid.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/CamelotAnalyzer.cpp
    audio/EnergyAnalyzer.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
)

target_include_directories(transitioncoach_phase40_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(transitioncoach_phase40_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(transitioncoach_phase40_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(transitioncoach_phase40_test PROPERTIES OUTPUT_NAME "TransitionCoach_Phase40_test")

# Beat Jump, Warp Grid & Polish Phase 41 test (Final Arc X Phase)
add_executable(beatjump_phase41_test
    audio/BeatJump_Phase41_test.cpp
    audio/SyncUndoStack.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/CamelotAnalyzer.cpp
    audio/EnergyAnalyzer.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/BeatGrid.cpp
    audio/SyncController.cpp
    audio/PhaseAligner.cpp
    visuals/SyncIndicator.cpp
)

target_include_directories(beatjump_phase41_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(beatjump_phase41_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(beatjump_phase41_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(beatjump_phase41_test PROPERTIES OUTPUT_NAME "BeatJump_Phase41_test")

# Graphics Visualization Panels Phase 1 test
add_executable(graphics_phase1_test
    tests/Graphics_Phase1_test.cpp
    audio/BeatGrid.cpp
    audio/BeatGridEditor.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/TrackLoader.cpp
    audio/BPMDetector.cpp
    audio/MetadataParser.cpp
    audio/KeyDetector.cpp
    audio/WaveformCache.cpp
    audio/CamelotAnalyzer.cpp
    audio/EnergyAnalyzer.cpp
    audio/EffectChain.cpp
    audio/Reverb.cpp
    audio/Delay.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    gameplay/EnergyCurve.cpp
    gameplay/TransitionCoach.cpp
    visuals/BeatGridRenderer.cpp
    visuals/EnergyHistogram.cpp
    visuals/CamelotWheel.cpp
    visuals/CoachingHUD.cpp
    visuals/SyncIndicator.cpp
)

target_include_directories(graphics_phase1_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(graphics_phase1_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_phase1_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_phase1_test PROPERTIES OUTPUT_NAME "Graphics_Phase1_test")

# Graphics 3D Crowd Visualization Phase 2 test
add_executable(graphics_phase2_test
    tests/Graphics_Phase2_test.cpp
    visuals/CrowdRenderer.cpp
    visuals/CrowdAnimator.cpp
    visuals/CrowdMesh.cpp
)

target_include_directories(graphics_phase2_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(graphics_phase2_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_phase2_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_phase2_test PROPERTIES OUTPUT_NAME "Graphics_Phase2_test")

# Graphics Phase 3 test (Lighting and Lasers)
add_executable(graphics_phase3_test
    tests/Graphics_Phase3_test.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
)

target_include_directories(graphics_phase3_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(graphics_phase3_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_phase3_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_phase3_test PROPERTIES OUTPUT_NAME "Graphics_Phase3_test")

# Graphics Phase 5 test (Performance and Career UI Panels)
add_executable(graphics_phase5_test
    tests/Graphics_Phase5_test.cpp
    gameplay/GameModes.cpp
    gameplay/UnlockSystem.cpp
    gameplay/AchievementSystem.cpp
    library/TrackLibrary.cpp
    library/TrackBrowser.cpp
    audio/CamelotAnalyzer.cpp
)

target_include_directories(graphics_phase5_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(graphics_phase5_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_phase5_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_phase5_test PROPERTIES OUTPUT_NAME "Graphics_Phase5_test")

# Graphics Phase 6 test (Graphics Settings and Hotkey Toggles)
add_executable(graphics_phase6_test
    tests/Graphics_Phase6_test.cpp
    input/InputMapper.cpp
)

target_include_directories(graphics_phase6_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(graphics_phase6_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphics_phase6_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_phase6_test PROPERTIES OUTPUT_NAME "Graphics_Phase6_test")

# Splash Screen Phase 42 test
add_executable(splashscreen_phase42_test
    visuals/SplashScreen_Phase42_test.cpp
    visuals/SplashScreenAnimator.cpp
)

target_include_directories(splashscreen_phase42_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(splashscreen_phase42_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(splashscreen_phase42_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(splashscreen_phase42_test PROPERTIES OUTPUT_NAME "SplashScreen_Phase42_test")

# ===== Benchmarks (Phase 2: Performance Profiling & Optimization) =====

# Benchmark: SpectrumAnalyzer Performance
add_executable(benchmark_spectrum_analyzer
    tests/benchmarks/Benchmark_SpectrumAnalyzer.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
)

target_include_directories(benchmark_spectrum_analyzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(benchmark_spectrum_analyzer PRIVATE /W4 /permissive-)
else()
    target_compile_options(benchmark_spectrum_analyzer PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(benchmark_spectrum_analyzer PROPERTIES OUTPUT_NAME "Benchmark_SpectrumAnalyzer")

# Benchmark: BPMDetector Performance
add_executable(benchmark_bpm_detector
    tests/benchmarks/Benchmark_BPMDetector.cpp
    audio/BPMDetector.cpp
    audio/AudioClip.cpp
)

target_include_directories(benchmark_bpm_detector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(benchmark_bpm_detector PRIVATE /W4 /permissive-)
else()
    target_compile_options(benchmark_bpm_detector PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(benchmark_bpm_detector PROPERTIES OUTPUT_NAME "Benchmark_BPMDetector")

# Benchmark: Main Audio Loop Performance (Mixer & Recorder hot paths)
add_executable(benchmark_main_loop_audio
    tests/benchmarks/Benchmark_MainLoopAudioPath.cpp
    audio/Recorder.cpp
)

target_include_directories(benchmark_main_loop_audio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(benchmark_main_loop_audio PRIVATE /W4 /permissive-)
else()
    target_compile_options(benchmark_main_loop_audio PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(benchmark_main_loop_audio PROPERTIES OUTPUT_NAME "Benchmark_MainLoopAudioPath")

# ===== Integration Tests (Phase 1: Polish & Optimization) =====

# Integration Smoke Tests
add_executable(smoke_test_main
    tests/integration/SmokeTest_main.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/Mixer.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    input/InputMapper.cpp
    crowdAI/CrowdStateMachine.cpp
)

target_include_directories(smoke_test_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(smoke_test_main PRIVATE /W4 /permissive-)
else()
    target_compile_options(smoke_test_main PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(smoke_test_main PROPERTIES OUTPUT_NAME "SmokeTest_main")

# Integration E2E: Load, Mix, Record Test
add_executable(e2e_load_mix_record_test
    tests/integration/E2E_LoadMixRecord_test.cpp
    audio/Deck.cpp
    audio/AudioClip.cpp
    audio/Mixer.cpp
    audio/Recorder.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
)

target_include_directories(e2e_load_mix_record_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(e2e_load_mix_record_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(e2e_load_mix_record_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(e2e_load_mix_record_test PROPERTIES OUTPUT_NAME "E2E_LoadMixRecord_test")

# Integration E2E: Session Round Trip Test
add_executable(e2e_session_roundtrip_test
    tests/integration/E2E_SessionRoundTrip_test.cpp
    core/SessionState.cpp
)

target_include_directories(e2e_session_roundtrip_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(e2e_session_roundtrip_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(e2e_session_roundtrip_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(e2e_session_roundtrip_test PROPERTIES OUTPUT_NAME "E2E_SessionRoundTrip_test")

# Integration E2E: Bass Clash Detection Test
add_executable(e2e_bassclash_detection_test
    tests/integration/E2E_BassClashDetection_test.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/CamelotAnalyzer.cpp
    gameplay/MixQualityAnalyzer.cpp
)

target_include_directories(e2e_bassclash_detection_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(e2e_bassclash_detection_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(e2e_bassclash_detection_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(e2e_bassclash_detection_test PROPERTIES OUTPUT_NAME "E2E_BassClashDetection_test")

# Integration E2E: Auto-Sync Test
add_executable(e2e_autosync_test
    tests/integration/E2E_AutoSync_test.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/BeatGrid.cpp
    visuals/SyncIndicator.cpp
)

target_include_directories(e2e_autosync_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(e2e_autosync_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(e2e_autosync_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(e2e_autosync_test PROPERTIES OUTPUT_NAME "E2E_AutoSync_test")

# Integration E2E: Battle Mode Test
add_executable(e2e_battlemode_test
    tests/integration/E2E_BattleMode_test.cpp
    multiplayer/BattleMode.cpp
    multiplayer/BattleRules.cpp
    multiplayer/Judge.cpp
)

target_include_directories(e2e_battlemode_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(e2e_battlemode_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(e2e_battlemode_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(e2e_battlemode_test PROPERTIES OUTPUT_NAME "E2E_BattleMode_test")

# Integration E2E: MIDI Effects Test
add_executable(e2e_midi_effects_test
    tests/integration/E2E_MIDIEffects_test.cpp
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
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    input/MIDIDevice.cpp
    input/MIDIMessage.cpp
    input/MIDIController.cpp
)

target_include_directories(e2e_midi_effects_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Windows Multimedia API for MIDI
if(WIN32)
    target_link_libraries(e2e_midi_effects_test PRIVATE winmm)
endif()

if(MSVC)
    target_compile_options(e2e_midi_effects_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(e2e_midi_effects_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(e2e_midi_effects_test PROPERTIES OUTPUT_NAME "E2E_MIDIEffects_test")

# DJ Controller Geometry Phase 1 test (Procedural 3D geometry for DJ controller components)
add_executable(djcontroller_geometry_phase1_test
    tests/DJControllerGeometry_Phase1_test.cpp
    visuals/DJControllerGeometry.cpp
)

target_include_directories(djcontroller_geometry_phase1_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(djcontroller_geometry_phase1_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(djcontroller_geometry_phase1_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(djcontroller_geometry_phase1_test PROPERTIES OUTPUT_NAME "DJControllerGeometry_Phase1_test")

# DJ Controller Geometry Phase 5 test (Visual Feedback and Sync)
add_executable(djcontroller_geometry_phase5_test
    tests/DJControllerGeometry_Phase5_test.cpp
    visuals/DJControllerGeometry.cpp
    visuals/DJControllerGeometry_Phase5_visuals.cpp
    visuals/RayCaster.cpp
    audio/Deck.cpp
    audio/Mixer.cpp
    audio/AudioClip.cpp
    audio/ThreeBandEQ.cpp
    audio/SpectrumAnalyzer.cpp
    audio/SyncController.cpp
    audio/BPMDetector.cpp
    audio/OnsetDetector.cpp
    audio/TrackLoader.cpp
    audio/KeyDetector.cpp
    audio/CamelotAnalyzer.cpp
    audio/EnergyAnalyzer.cpp
    audio/FFTEngine.cpp
    audio/EffectChain.cpp
    audio/Delay.cpp
    audio/Reverb.cpp
    audio/Phaser.cpp
    audio/Flanger.cpp
    audio/Bitcrusher.cpp
    audio/AutoFilter.cpp
    audio/RingModulator.cpp
    audio/VinylSimulator.cpp
    audio/ScratchDetector.cpp
    audio/PhaseAligner.cpp
    audio/MetadataParser.cpp
)

target_include_directories(djcontroller_geometry_phase5_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(djcontroller_geometry_phase5_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(djcontroller_geometry_phase5_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(djcontroller_geometry_phase5_test PROPERTIES OUTPUT_NAME "DJControllerGeometry_Phase5_test")

# RayCaster Phase 3 test (3D raycasting and mouse picking for DJ controller)
add_executable(raycaster_phase3_test
    tests/RayCaster_Phase3_test.cpp
    visuals/RayCaster.cpp
    visuals/Camera.cpp
    visuals/DJControllerGeometry.cpp
)

target_include_directories(raycaster_phase3_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(raycaster_phase3_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(raycaster_phase3_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(raycaster_phase3_test PROPERTIES OUTPUT_NAME "RayCaster_Phase3_test")

# ControllerInteraction Phase 4 test
add_executable(controller_interaction_phase4_test
    tests/ControllerInteraction_Phase4_test.cpp
    visuals/ControllerInteraction.cpp
    visuals/DJControllerGeometry.cpp
    visuals/RayCaster.cpp
    visuals/Camera.cpp
    audio/Deck.cpp
    audio/Mixer.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
)

target_include_directories(controller_interaction_phase4_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(controller_interaction_phase4_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(controller_interaction_phase4_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(controller_interaction_phase4_test PROPERTIES OUTPUT_NAME "ControllerInteraction_Phase4_test")

# ControllerInteraction Phase 6 test
add_executable(controller_interaction_phase6_test
    tests/ControllerInteraction_Phase6_test.cpp
    visuals/ControllerInteraction.cpp
    visuals/DJControllerGeometry.cpp
    visuals/RayCaster.cpp
    visuals/Camera.cpp
    audio/Deck.cpp
    audio/Mixer.cpp
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
    audio/PhaseAligner.cpp
    audio/SyncController.cpp
    audio/SpectrumAnalyzer.cpp
    audio/FFTEngine.cpp
)

target_include_directories(controller_interaction_phase6_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(controller_interaction_phase6_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(controller_interaction_phase6_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(controller_interaction_phase6_test PROPERTIES OUTPUT_NAME "ControllerInteraction_Phase6_test")

# ===== OpenGL Migration Phase 1: GLContext (Infrastructure Setup) =====

# GLContext Phase 1 test
add_executable(glcontext_phase1_test
    tests/GLContext_Phase1_test.cpp
)

target_include_directories(glcontext_phase1_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

# Link OpenGL libraries
target_link_libraries(glcontext_phase1_test PRIVATE 
    glfw
    glm::glm
)

# If GLAD source file exists, compile it into the test
if(EXISTS "${GLAD_SRC}")
    target_sources(glcontext_phase1_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(glcontext_phase1_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(glcontext_phase1_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(glcontext_phase1_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(glcontext_phase1_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(glcontext_phase1_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(glcontext_phase1_test PROPERTIES OUTPUT_NAME "GLContext_Phase1_test")

# ============================================================================
# VertexBuffer Phase 2 Test (OpenGL VBO)
# ============================================================================
add_executable(vertexbuffer_phase2_test
    tests/VertexBuffer_Phase2_test.cpp
    visuals/VertexBuffer.cpp
)

target_include_directories(vertexbuffer_phase2_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(vertexbuffer_phase2_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
endif()

target_link_libraries(vertexbuffer_phase2_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(vertexbuffer_phase2_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(vertexbuffer_phase2_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(vertexbuffer_phase2_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(vertexbuffer_phase2_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(vertexbuffer_phase2_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(vertexbuffer_phase2_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(vertexbuffer_phase2_test PROPERTIES OUTPUT_NAME "VertexBuffer_Phase2_test")

# ============================================================================
# Shader Phase 3 Test (OpenGL GLSL)
# ============================================================================
add_executable(shader_phase3_test
    tests/Shader_Phase3_test.cpp
    visuals/Shader.cpp
)

target_include_directories(shader_phase3_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(shader_phase3_test PRIVATE 
        DJROOFRAT_ENABLE_GRAPHICS=1
        DJROOFRAT_OPENGL_MIGRATION=1
    )
endif()

target_link_libraries(shader_phase3_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(shader_phase3_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(shader_phase3_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(shader_phase3_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(shader_phase3_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(shader_phase3_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(shader_phase3_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(shader_phase3_test PROPERTIES OUTPUT_NAME "Shader_Phase3_test")

# ============================================================================
# IndexBuffer Phase 2 Test (OpenGL EBO)
# ============================================================================
add_executable(indexbuffer_phase2_test
    tests/IndexBuffer_Phase2_test.cpp
    visuals/IndexBuffer.cpp
)

target_include_directories(indexbuffer_phase2_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

if(DJROOFRAT_ENABLE_GRAPHICS)
    target_compile_definitions(indexbuffer_phase2_test PRIVATE DJROOFRAT_ENABLE_GRAPHICS=1)
endif()

target_link_libraries(indexbuffer_phase2_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(indexbuffer_phase2_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(indexbuffer_phase2_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(indexbuffer_phase2_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(indexbuffer_phase2_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(indexbuffer_phase2_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(indexbuffer_phase2_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(indexbuffer_phase2_test PROPERTIES OUTPUT_NAME "IndexBuffer_Phase2_test")

# ============================================================================
# GraphicsContext Phase 4 Test (OpenGL Context Creation)
# ============================================================================
add_executable(graphicscontext_phase4_test
    tests/GraphicsContext_Phase4_test.cpp
    visuals/GraphicsContext.cpp
    visuals/LightingRig.cpp
    visuals/LaserController.cpp
    visuals/CrowdRenderer.cpp
)

target_include_directories(graphicscontext_phase4_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

target_compile_definitions(graphicscontext_phase4_test PRIVATE 
    DJROOFRAT_OPENGL_MIGRATION=1
)

target_link_libraries(graphicscontext_phase4_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(graphicscontext_phase4_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(graphicscontext_phase4_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(graphicscontext_phase4_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(graphicscontext_phase4_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(graphicscontext_phase4_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(graphicscontext_phase4_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphicscontext_phase4_test PROPERTIES OUTPUT_NAME "GraphicsContext_Phase4_test")

# ============================================================================
# Texture Phase 5 Test (OpenGL Texture System)
# ============================================================================
add_executable(texture_phase5_test
    tests/Texture_Phase5_test.cpp
    visuals/Texture.cpp
)

target_include_directories(texture_phase5_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

target_compile_definitions(texture_phase5_test PRIVATE 
    DJROOFRAT_ENABLE_GRAPHICS=1
    DJROOFRAT_OPENGL_MIGRATION=1
)

target_link_libraries(texture_phase5_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(texture_phase5_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(texture_phase5_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(texture_phase5_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(texture_phase5_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(texture_phase5_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(texture_phase5_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(texture_phase5_test PROPERTIES OUTPUT_NAME "Texture_Phase5_test")

# ============================================================================
# Enhanced3DScene Phase 6 Test (OpenGL 3D Rendering Pipeline)
# ============================================================================
add_executable(enhanced3dscene_phase6_test
    tests/Enhanced3DScene_Phase6_test.cpp
)

target_include_directories(enhanced3dscene_phase6_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

target_compile_definitions(enhanced3dscene_phase6_test PRIVATE 
    DJROOFRAT_ENABLE_GRAPHICS=1
    DJROOFRAT_OPENGL_MIGRATION=1
)

target_link_libraries(enhanced3dscene_phase6_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(enhanced3dscene_phase6_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(enhanced3dscene_phase6_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(enhanced3dscene_phase6_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(enhanced3dscene_phase6_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(enhanced3dscene_phase6_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(enhanced3dscene_phase6_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(enhanced3dscene_phase6_test PROPERTIES OUTPUT_NAME "Enhanced3DScene_Phase6_test")

# ============================================================================
# Framebuffer Phase 7 Test (Framebuffer Objects for Post-Processing)
# ============================================================================
add_executable(framebuffer_phase7_test
    tests/Framebuffer_Phase7_test.cpp
    visuals/Framebuffer.cpp
)

target_include_directories(framebuffer_phase7_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

target_compile_definitions(framebuffer_phase7_test PRIVATE 
    DJROOFRAT_ENABLE_GRAPHICS=1
    DJROOFRAT_OPENGL_MIGRATION=1
)

target_link_libraries(framebuffer_phase7_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(framebuffer_phase7_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(framebuffer_phase7_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(framebuffer_phase7_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(framebuffer_phase7_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(framebuffer_phase7_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(framebuffer_phase7_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(framebuffer_phase7_test PROPERTIES OUTPUT_NAME "Framebuffer_Phase7_test")

# ============================================================================
# ComputeShader Phase 7 Test (OpenGL Compute Shaders for GPGPU)
# ============================================================================
add_executable(computeshader_phase7_test
    tests/ComputeShader_Phase7_test.cpp
    visuals/ComputeShader.cpp
)

target_include_directories(computeshader_phase7_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
)

target_compile_definitions(computeshader_phase7_test PRIVATE 
    DJROOFRAT_ENABLE_GRAPHICS=1
    DJROOFRAT_OPENGL_MIGRATION=1
)

target_link_libraries(computeshader_phase7_test PRIVATE 
    glfw
    glm::glm
)

if(EXISTS "${GLAD_SRC}")
    target_sources(computeshader_phase7_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(computeshader_phase7_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(computeshader_phase7_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(computeshader_phase7_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(computeshader_phase7_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(computeshader_phase7_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(computeshader_phase7_test PROPERTIES OUTPUT_NAME "ComputeShader_Phase7_test")

# ============================================================================
# ImGui Phase 7A Test (OpenGL3 Backend Integration)
# ============================================================================
add_executable(imgui_phase7a_test
    tests/ImGui_Phase7A_test.cpp
)

target_include_directories(imgui_phase7a_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(imgui_phase7a_test PRIVATE 
    DJROOFRAT_OPENGL_MIGRATION=1
)

target_link_libraries(imgui_phase7a_test PRIVATE 
    glfw
    glm::glm
    imgui_impl
)

if(EXISTS "${GLAD_SRC}")
    target_sources(imgui_phase7a_test PRIVATE "${GLAD_SRC}")
endif()

if(WIN32)
    target_link_libraries(imgui_phase7a_test PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(imgui_phase7a_test PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(imgui_phase7a_test PRIVATE OpenGL::OpenGL)
endif()

if(MSVC)
    target_compile_options(imgui_phase7a_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(imgui_phase7a_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(imgui_phase7a_test PROPERTIES OUTPUT_NAME "ImGui_Phase7A_test")
