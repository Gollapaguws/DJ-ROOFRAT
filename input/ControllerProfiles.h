#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "MIDIMapping.h"

namespace dj::input {

/// Represents a preset MIDI controller profile with standard mappings
class ControllerProfile {
public:
    explicit ControllerProfile(const std::string& name);
    ~ControllerProfile() = default;

    const std::string& getName() const { return name_; }

    /// Add a mapping to the profile
    void addMapping(const MIDIMapping& mapping) {
        mappings_.push_back(mapping);
    }

    /// Get all mappings
    const std::vector<MIDIMapping>& getMappings() const { return mappings_; }

    /// Find a mapping by target function
    /// @param functionName The target function (e.g., "deckA_volume")
    /// @return Mapping if found, std::nullopt otherwise
    std::optional<MIDIMapping> getMappingForFunction(const std::string& functionName) const;

    /// Get all mapping function names in this profile
    std::vector<std::string> getAvailableFunctions() const;

private:
    std::string name_;
    std::vector<MIDIMapping> mappings_;
};

/// Factory for preset controller profiles
class ControllerProfiles {
public:
    /// Get Pioneer DDJ-400 preset profile
    /// Standard mappings: crossfader, volume A/B, EQ low/mid/high A/B, pitch A/B
    static std::shared_ptr<ControllerProfile> getPioneerDDJ400Profile();

    /// Get Numark Mixtrack preset profile
    static std::shared_ptr<ControllerProfile> getNumarkMixtractProfile();

    /// Get Native Instruments Traktor S2 preset profile
    static std::shared_ptr<ControllerProfile> getNITraktorS2Profile();

    /// Get default/generic mapping for basic controllers
    /// Minimal set of essential mappings
    static std::shared_ptr<ControllerProfile> getDefaultProfile();

private:
    // Helper functions to build specific profiles
    static std::shared_ptr<ControllerProfile> buildPioneerDDJ400();
    static std::shared_ptr<ControllerProfile> buildNumarkMixtrack();
    static std::shared_ptr<ControllerProfile> buildTraktorS2();
    static std::shared_ptr<ControllerProfile> buildDefault();
};

} // namespace dj::input
