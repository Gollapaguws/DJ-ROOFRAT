# Plan: Functional 3D DJ Controller

A fully interactive 3D DJ controller rendered in the DirectX 11 scene that maps mouse interactions to deck/mixer controls. The controller will display jog wheels, pitch faders, EQ knobs, crossfader, and buttons with real-time visual feedback synchronized to deck state.

**Visual Style:** Minimalist wireframe (flat colors, clean lines) for performance and clarity.

## Phase 1: DJControllerGeometry Foundation
- **Objective:** Create procedural geometry class that generates 3D primitives for all controller components positioned on a flat controller surface
- **Files/Functions to Modify/Create:**
  - Create visuals/DJControllerGeometry.h and visuals/DJControllerGeometry.cpp
  - Declare class with methods: generateMesh(), getVertices(), getIndices(), getControlBounds()
  - Add primitive generators: createKnob(), createFader(), createButton(), createJogWheel()
- **Tests to Write:**
  - test_DJControllerGeometry_GeneratesMesh - verify vertex/index buffer creation
  - test_DJControllerGeometry_KnobHasCorrectShape - check cylinder has ~20 vertices
  - test_DJControllerGeometry_ControlPositionsNonOverlapping - verify component bounding boxes don't intersect
  - test_DJControllerGeometry_AllControlsGenerated - count all controls
- **Steps:**
  1. Write test file tests/DJControllerGeometry_Phase1_test.cpp with all 4 tests
  2. Run tests to confirm RED state
  3. Implement DJControllerGeometry class with Vertex/Index generation
  4. Position components: center crossfader at (0, 0.1, 0), deck A at (-0.5, 0.1, 0), deck B at (0.5, 0.1, 0)
  5. Run tests to confirm GREEN state
  6. Format and lint code

## Phase 2: Scene Integration and Rendering
- **Objective:** Integrate DJControllerGeometry into Enhanced3DScene and render it with PBR lighting
- **Files/Functions to Modify/Create:**
  - Modify visuals/Enhanced3DScene.h - add std::unique_ptr<DJControllerGeometry> m_controllerGeometry
  - Modify visuals/Enhanced3DScene.cpp - initialize in constructor, call renderController()
  - Create renderController(Camera&) method
  - Update CMakeLists.txt - add visuals/DJControllerGeometry.cpp
- **Tests to Write:**
  - test_Enhanced3DScene_ControllerInitialized
  - test_Enhanced3DScene_ControllerRendersWithoutCrash
  - test_DJControllerGeometry_TransformApplied
- **Steps:**
  1. Write test file tests/Enhanced3DScene_Phase2_test.cpp
  2. Run tests to confirm RED state
  3. Add DJControllerGeometry member to Enhanced3DScene
  4. Implement renderController() method
  5. Add source file to CMakeLists.txt
  6. Run tests to confirm GREEN state
  7. Manually verify rendering

## Phase 3: Mouse Raycasting and Picking
- **Objective:** Implement 3D raycasting from mouse screen coordinates to detect which controller component is under cursor
- **Files/Functions to Modify/Create:**
  - Create visuals/RayCaster.h and visuals/RayCaster.cpp
  - Methods: screenToWorldRay(), intersectSphere(), intersectBox()
  - Modify visuals/DJControllerGeometry.h - add pickControl(const Ray& ray) method
  - Add enum class ControlID
- **Tests to Write:**
  - test_RayCaster_ScreenToWorldRay
  - test_RayCaster_IntersectSphereHit
  - test_RayCaster_IntersectSphereMiss
  - test_DJControllerGeometry_PickControl
  - test_DJControllerGeometry_PickControlMiss
- **Steps:**
  1. Write test file tests/RayCaster_Phase3_test.cpp
  2. Run tests to confirm RED state
  3. Implement RayCaster class with intersection math
  4. Implement DJControllerGeometry::pickControl()
  5. Update CMakeLists.txt
  6. Run tests to confirm GREEN state

## Phase 4: Interactive Control Binding
- **Objective:** Connect mouse drag events to deck/mixer parameter updates
- **Files/Functions to Modify/Create:**
  - Create visuals/ControllerInteraction.h and visuals/ControllerInteraction.cpp
  - Methods: handleMouseDown(), handleMouseDrag(), handleMouseUp()
  - Modify src/main.cpp - add mouse polling in game loop
- **Tests to Write:**
  - test_ControllerInteraction_CrossfaderDrag
  - test_ControllerInteraction_EQKnobDrag
  - test_ControllerInteraction_PitchFaderDrag
  - test_ControllerInteraction_ButtonClick
  - test_ControllerInteraction_JogWheelDrag
- **Steps:**
  1. Write test file tests/ControllerInteraction_Phase4_test.cpp
  2. Run tests to confirm RED state
  3. Implement ControllerInteraction class
  4. Integrate mouse polling in main.cpp
  5. Map mouse deltas to control ranges
  6. Update CMakeLists.txt
  7. Run tests to confirm GREEN state
  8. Manual test in running app

## Phase 5: Visual Feedback and Sync
- **Objective:** Update controller visuals in real-time to reflect current deck/mixer state
- **Files/Functions to Modify/Create:**
  - Modify visuals/DJControllerGeometry.cpp - add updateVisuals() method
  - Update vertex colors and transforms based on state
  - Modify visuals/Enhanced3DScene.cpp - call updateVisuals() every frame
- **Tests to Write:**
  - test_DJControllerGeometry_CrossfaderPositionSync
  - test_DJControllerGeometry_PlayButtonGlow
  - test_DJControllerGeometry_EQKnobRotation
  - test_DJControllerGeometry_PitchFaderSync
- **Steps:**
  1. Write test file tests/DJControllerGeometry_Phase5_test.cpp
  2. Run tests to confirm RED state
  3. Implement updateVisuals() method
  4. Add rotation matrices for knobs
  5. Update vertex colors for buttons
  6. Run tests to confirm GREEN state
  7. Manual test: watch 3D controls update

## Phase 6: Advanced Interactions and Polish
- **Objective:** Add jog wheel scratching, hover highlights, smoothing animations
- **Files/Functions to Modify/Create:**
  - Modify visuals/ControllerInteraction.cpp - add handleJogWheelScratch()
  - Modify visuals/DJControllerGeometry.cpp - add setHoverHighlight()
  - Add lerp smoothing to visual updates
- **Tests to Write:**
  - test_ControllerInteraction_JogWheelScratching
  - test_DJControllerGeometry_HoverHighlightApplied
  - test_DJControllerGeometry_VisualSmoothingLerp
- **Steps:**
  1. Write test file tests/ControllerInteraction_Phase6_test.cpp
  2. Run tests to confirm RED state
  3. Implement jog wheel velocity calculation
  4. Implement hover highlight
  5. Add lerp interpolation
  6. Run tests to confirm GREEN state
  7. Manual test: scratch, observe hover effects
