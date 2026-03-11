## Plan: Arc III - Advanced Audio Processing (Phases 15-17)

Implement professional-grade mixing effects (reverb, delay, modulation) and realistic vinyl simulation with scratching to match industry DJ software capabilities.

**Phases: 3 phases**

### **Phase 15: Time-Based Effects (Reverb & Delay)**
- **Objective:** Add reverb and delay effects with tempo sync and routing flexibility
- **Files/Functions to Modify/Create:**
  - audio/Reverb.h: Freeverb algorithm interface
  - audio/Reverb.cpp: Schroeder reverb implementation (comb + allpass filters)
  - audio/Delay.h: Stereo delay interface with feedback
  - audio/Delay.cpp: Circular buffer delay with tempo-sync
  - audio/EffectChain.h: Serial and parallel effect routing
  - audio/EffectChain.cpp: Effect processing pipeline with pre/post EQ insertion
  - audio/Deck.h: Add effect send levels and routing
  - audio/Deck.cpp: Integrate effect chain into audio path
  - input/InputMapper.h: Add effect control commands
  - src/main.cpp: Add effect parameter controls
- **Tests to Write:**
  - test_Reverb_RoomSize: Verify room size parameter affects decay time
  - test_Reverb_Damping: Test high-frequency damping control
  - test_Delay_TempoSync: Validate delay time syncs to BPM (1/4, 1/8, 1/16 notes)
  - test_Delay_Feedback: Test feedback loop stability (no runaway)
  - test_EffectChain_Serial: Verify serial routing (reverb → delay)
  - test_EffectChain_Parallel: Test parallel routing (dry + wet mix)
  - test_Deck_EffectSend: Confirm send level controls effect amount
- **Steps:**
  1. Write tests for reverb, delay, and routing (tests fail)
  2. Implement Freeverb with comb/allpass filter network
  3. Create Delay with circular buffer and tempo-sync calculation
  4. Build EffectChain for flexible routing
  5. Add effect send controls to Deck
  6. Integrate effect processing in Deck audio path (pre or post-EQ)
  7. Run tests to verify effect quality and CPU usage <5% per effect (tests pass)
  8. Lint/format

### **Phase 16: Modulation & Distortion Effects**
- **Objective:** Add flanger, phaser, bitcrusher, and experimental effects for creative mixing
- **Files/Functions to Modify/Create:**
  - audio/Flanger.h: Flanger effect interface
  - audio/Flanger.cpp: Variable delay line with LFO modulation
  - audio/Phaser.h: Phaser effect interface
  - audio/Phaser.cpp: Allpass filter cascade with LFO
  - audio/Bitcrusher.h: Sample rate/bit depth reduction interface
  - audio/Bitcrusher.cpp: Quantization and downsampling implementation
  - audio/RingModulator.h: Ring modulation interface
  - audio/RingModulator.cpp: Carrier × modulator multiplication
  - audio/AutoFilter.h: Envelope-following filter interface
  - audio/AutoFilter.cpp: Peak detector + resonant filter
  - audio/EffectChain.cpp: Register new effects in factory
- **Tests to Write:**
  - test_Flanger_LFORate: Verify LFO rate control (0.1-10 Hz)
  - test_Flanger_Depth: Test modulation depth parameter
  - test_Phaser_Stages: Validate 4/6/8/12-stage configurations
  - test_Bitcrusher_BitDepth: Test 16→8→4-bit reduction
  - test_Bitcrusher_SampleRate: Verify downsampling (44.1k→22k→11k)
  - test_RingModulator_Frequency: Test carrier frequency sweep
  - test_AutoFilter_EnvelopeFollowing: Validate filter tracks input amplitude
- **Steps:**
  1. Write tests for all modulation/distortion effects (tests fail)
  2. Implement Flanger with sinusoidal LFO
  3. Create Phaser with cascaded allpass sections
  4. Implement Bitcrusher with sample-and-hold + quantization
  5. Add RingModulator for experimental sounds
  6. Create AutoFilter with envelope follower
  7. Run tests to verify effect character and stability (tests pass)
  8. Lint/format

### **Phase 17: Vinyl Simulation & Scratching**
- **Objective:** Realistic turntable physics and scratching for turntablist techniques
- **Files/Functions to Modify/Create:**
  - audio/VinylSimulator.h: Turntable physics model interface
  - audio/VinylSimulator.cpp: Inertia, friction, motor torque simulation
  - audio/ScratchDetector.h: Scratch analysis interface
  - audio/ScratchDetector.cpp: Direction, speed, pressure detection from input
  - audio/Deck.h: Add vinyl mode, motor state, platter velocity
  - audio/Deck.cpp: Integrate vinyl physics into playback rate
  - input/InputMapper.h: Add scratch control (mouse wheel, MIDI jogwheel)
  - input/InputMapper.cpp: Map scratch inputs to platter velocity
  - src/main.cpp: Add vinyl mode toggle, display platter speed
- **Tests to Write:**
  - test_VinylSimulator_Inertia: Verify platter spin-down with friction
  - test_VinylSimulator_MotorTorque: Test motor start/stop pitch ramp
  - test_VinylSimulator_Backspin: Validate reverse playback physics
  - test_ScratchDetector_Direction: Confirm forward/reverse detection
  - test_ScratchDetector_Speed: Test velocity-to-pitch mapping
  - test_Deck_VinylMode: Verify vinyl physics mode vs digital mode
  - test_Deck_NeedleDrop: Test instant cue point jump (simulated needle drop)
- **Steps:**
  1. Write tests for vinyl physics and scratching (tests fail)
  2. Implement VinylSimulator with mass-spring-damper model
  3. Create ScratchDetector analyzing input velocity
  4. Add motor start/stop simulation (pitch ramp over 0.5-2 seconds)
  5. Implement backspin and brake effects
  6. Integrate vinyl mode into Deck playback calculation
  7. Run tests to verify realistic feel and <5ms scratch latency (tests pass)
  8. Lint/format

**Open Questions:**
1. Reverb algorithm: Freeverb vs Dattorro vs convolution? **Freeverb (efficient, good quality)**
2. Delay max time: 2 seconds or 4 seconds? **4 seconds (allows creative long delays)**
3. Vinyl friction curve: Linear or exponential decay? **Exponential (more realistic)**
4. Scratch input source: Mouse, MIDI, or both? **Both (mouse for testing, MIDI for controllers)**
5. Motor start time: Fixed or adjustable? **Adjustable (33/45/78 RPM profiles)**

**Dependencies:**
- **Arc II Phase 13** recommended (MIDI jogwheel for realistic scratch control)
- No external libraries required (all DSP implemented in-house)
