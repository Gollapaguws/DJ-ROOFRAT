#include "ControllerProfiles.h"

namespace dj::input {

// ============================================================================
// ControllerProfile Implementation
// ============================================================================

ControllerProfile::ControllerProfile(const std::string& name) : name_(name) {}

std::optional<MIDIMapping> ControllerProfile::getMappingForFunction(const std::string& functionName) const {
    for (const auto& mapping : mappings_) {
        if (mapping.getTargetFunction() == functionName) {
            return mapping;
        }
    }
    return std::nullopt;
}

std::vector<std::string> ControllerProfile::getAvailableFunctions() const {
    std::vector<std::string> functions;
    for (const auto& mapping : mappings_) {
        functions.push_back(mapping.getTargetFunction());
    }
    return functions;
}

// ============================================================================
// ControllerProfiles Factory Implementation
// ============================================================================

std::shared_ptr<ControllerProfile> ControllerProfiles::getPioneerDDJ400Profile() {
    static auto profile = buildPioneerDDJ400();
    return profile;
}

std::shared_ptr<ControllerProfile> ControllerProfiles::getNumarkMixtractProfile() {
    static auto profile = buildNumarkMixtrack();
    return profile;
}

std::shared_ptr<ControllerProfile> ControllerProfiles::getNITraktorS2Profile() {
    static auto profile = buildTraktorS2();
    return profile;
}

std::shared_ptr<ControllerProfile> ControllerProfiles::getDefaultProfile() {
    static auto profile = buildDefault();
    return profile;
}

// ============================================================================
// Pioneer DDJ-400 Profile
// ============================================================================

std::shared_ptr<ControllerProfile> ControllerProfiles::buildPioneerDDJ400() {
    auto profile = std::make_shared<ControllerProfile>("Pioneer DDJ-400");
    
    // Crossfader (CC14)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(14);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);  // -1 = Deck A, 1 = Deck B
        m.setTargetFunction("crossfader");
        profile->addMapping(m);
    }
    
    // Deck A Volume (CC7)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(7);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_volume");
        profile->addMapping(m);
    }
    
    // Deck B Volume (CC7, channel 1)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(7);
        m.setChannel(1);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_volume");
        profile->addMapping(m);
    }
    
    // Deck A Master Tempo / Pitch (PitchBend, channel 0)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::PitchBend);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("deckA_pitch");
        m.setJogWheelAcceleration(true);
        profile->addMapping(m);
    }
    
    // Deck B Master Tempo / Pitch (PitchBend, channel 1)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::PitchBend);
        m.setChannel(1);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("deckB_pitch");
        m.setJogWheelAcceleration(true);
        profile->addMapping(m);
    }
    
    // Deck A EQ Low (CC17)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(17);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_eq_low");
        profile->addMapping(m);
    }
    
    // Deck A EQ Mid (CC18)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(18);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_eq_mid");
        profile->addMapping(m);
    }
    
    // Deck A EQ High (CC19)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(19);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_eq_high");
        profile->addMapping(m);
    }
    
    // Deck B EQ Low (CC17, channel 1)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(17);
        m.setChannel(1);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_eq_low");
        profile->addMapping(m);
    }
    
    // Deck B EQ Mid (CC18, channel 1)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(18);
        m.setChannel(1);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_eq_mid");
        profile->addMapping(m);
    }
    
    // Deck B EQ High (CC19, channel 1)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(19);
        m.setChannel(1);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_eq_high");
        profile->addMapping(m);
    }
    
    return profile;
}

// ============================================================================
// Numark Mixtrack Profile
// ============================================================================

std::shared_ptr<ControllerProfile> ControllerProfiles::buildNumarkMixtrack() {
    auto profile = std::make_shared<ControllerProfile>("Numark Mixtrack");
    
    // Crossfader (CC13)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(13);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("crossfader");
        profile->addMapping(m);
    }
    
    // Deck A Volume (CC5)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(5);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_volume");
        profile->addMapping(m);
    }
    
    // Deck B Volume (CC6)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(6);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_volume");
        profile->addMapping(m);
    }
    
    // Deck A Pitch (CC24)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(24);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("deckA_pitch");
        profile->addMapping(m);
    }
    
    // Deck B Pitch (CC25)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(25);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("deckB_pitch");
        profile->addMapping(m);
    }
    
    return profile;
}

// ============================================================================
// Native Instruments Traktor S2 Profile
// ============================================================================

std::shared_ptr<ControllerProfile> ControllerProfiles::buildTraktorS2() {
    auto profile = std::make_shared<ControllerProfile>("Native Instruments Traktor S2");
    
    // Crossfader (CC15)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(15);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("crossfader");
        profile->addMapping(m);
    }
    
    // Channel A Volume (CC20)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(20);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_volume");
        profile->addMapping(m);
    }
    
    // Channel B Volume (CC21)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(21);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_volume");
        profile->addMapping(m);
    }
    
    return profile;
}

// ============================================================================
// Default / Generic Profile
// ============================================================================

std::shared_ptr<ControllerProfile> ControllerProfiles::buildDefault() {
    auto profile = std::make_shared<ControllerProfile>("Generic/Default");
    
    // Basic crossfader (CC1)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(1);
        m.setChannel(0);
        m.setCurveType(CurveType::Linear);
        m.setValueRange(-1.0f, 1.0f);
        m.setTargetFunction("crossfader");
        profile->addMapping(m);
    }
    
    // Basic Deck A Volume (CC7)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(7);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckA_volume");
        profile->addMapping(m);
    }
    
    // Basic Deck B Volume (CC8)
    {
        MIDIMapping m;
        m.setMessageType(MIDICurveType::ControlChange);
        m.setControllerNumber(8);
        m.setChannel(0);
        m.setCurveType(CurveType::Logarithmic);
        m.setValueRange(0.0f, 1.0f);
        m.setTargetFunction("deckB_volume");
        profile->addMapping(m);
    }
    
    return profile;
}

} // namespace dj::input
