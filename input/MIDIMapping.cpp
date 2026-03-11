#include "MIDIMapping.h"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace dj::input {

MIDIMapping::MIDIMapping() = default;

void MIDIMapping::setValueRange(float minValue, float maxValue) {
    minValue_ = std::min(minValue, maxValue);
    maxValue_ = std::max(minValue, maxValue);
}

float MIDIMapping::applyLinearCurve(float normalized) const {
    return minValue_ + (normalized * (maxValue_ - minValue_));
}

float MIDIMapping::applyLogarithmicCurve(float normalized) const {
    // Log curve: compress using 10^(x-1)
    // Good for volume perception (human hearing is logarithmic)
    // At x=0: 10^(-1) = 0.1, gradually ramping to x=1: 10^0 = 1
    // For normalized [0..1], we want output where lower CC values feel less responsive
    if (normalized <= 0.0f) {
        return minValue_;
    }
    if (normalized >= 1.0f) {
        return maxValue_;
    }
    
    // Using: y = 10^(x - 1) to get values from 0.1 to 1, then scale to output range
    float logNorm = std::pow(10.0f, normalized - 1.0f);
    return minValue_ + (logNorm * (maxValue_ - minValue_));
}

float MIDIMapping::applyExponentialCurve(float normalized) const {
    // Exponential curve: compress lower values, emphasize higher values
    // For pitch bend: preserve sign and apply acceleration
    if (normalized >= 1.0f) {
        return maxValue_;
    }
    if (normalized <= -1.0f) {
        return minValue_;
    }
    if (std::abs(normalized) < 0.0001f) {
        return 0.0f;
    }
    
    // Apply curve while preserving sign: y = sign(x) * x^2
    float absNorm = std::abs(normalized);
    float expNorm = absNorm * absNorm;
    if (normalized < 0.0f) {
        expNorm = -expNorm;
    }
    
    return minValue_ + (expNorm * (maxValue_ - minValue_));
}

float MIDIMapping::applyMapping(int midiValue) const {
    // Normalize MIDI value to [0, 1]
    float normalized = 0.0f;
    
    if (messageType_ == MIDICurveType::PitchBend) {
        // 14-bit pitch bend: 0-16383, neutral at 8192
        // Map to [-1.0, 1.0]
        if (jogWheelAcceleration_) {
            // Use acceleration for velocity sensitivity with dead zone
            const int deadZone = 200;
            int adjustedValue = midiValue - 8192;
            
            if (std::abs(adjustedValue) <= deadZone) {
                normalized = 0.0f;
            } else {
                // Scale outside dead zone with acceleration
                // Calculate distance from center and apply exponential curve
                int absAdjusted = std::abs(adjustedValue);
                float distance = static_cast<float>(absAdjusted - deadZone);
                float maxDistance = 8192.0f - deadZone;
                
                // Normalize to [0, 1]
                float normDist = distance / maxDistance;
                normDist = std::clamp(normDist, 0.0f, 1.0f);
                
                // Apply acceleration curve: quadratic to emphasize larger movements
                float accelerated = normDist * normDist;
                
                // Apply sign based on direction
                normalized = (adjustedValue > 0) ? accelerated : -accelerated;
            }
        } else {
            // Linear pitch bend
            normalized = (static_cast<float>(midiValue) - 8192.0f) / 8192.0f;
            normalized = std::clamp(normalized, -1.0f, 1.0f);
        }
    } else {
        // CC or Note: 0-127
        normalized = static_cast<float>(midiValue) / 127.0f;
        normalized = std::clamp(normalized, 0.0f, 1.0f);
    }

    // Apply curve transformation (skip for pitch bend with acceleration)
    if (messageType_ == MIDICurveType::PitchBend && jogWheelAcceleration_) {
        // Already handled acceleration in pitch bend section
        return minValue_ + (normalized * (maxValue_ - minValue_));
    }
    
    float result = 0.0f;
    switch (curveType_) {
    case CurveType::Linear:
        result = applyLinearCurve(normalized);
        break;
    case CurveType::Logarithmic:
        result = applyLogarithmicCurve(normalized);
        break;
    case CurveType::Exponential:
        result = applyExponentialCurve(normalized);
        break;
    }

    return result;
}

std::string MIDIMapping::toJSON() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"messageType\":" << static_cast<int>(messageType_) << ",";
    oss << "\"channel\":" << channel_ << ",";
    oss << "\"controllerNumber\":" << controllerNumber_ << ",";
    oss << "\"note\":" << note_ << ",";
    oss << "\"curveType\":" << static_cast<int>(curveType_) << ",";
    oss << "\"minValue\":" << minValue_ << ",";
    oss << "\"maxValue\":" << maxValue_ << ",";
    oss << "\"targetFunction\":\"" << targetFunction_ << "\",";
    oss << "\"jogWheelAcceleration\":" << (jogWheelAcceleration_ ? "true" : "false");
    oss << "}";
    return oss.str();
}

std::optional<MIDIMapping> MIDIMapping::fromJSON(const std::string& json) {
    // Simple JSON parser (not using external library)
    // Expects format: {"messageType":0,"channel":0,...}
    
    MIDIMapping mapping;
    
    // Extract messageType
    size_t pos = json.find("\"messageType\":");
    if (pos != std::string::npos) {
        int val = std::stoi(json.substr(pos + 14));
        mapping.messageType_ = static_cast<MIDICurveType>(val);
    }
    
    // Extract channel
    pos = json.find("\"channel\":");
    if (pos != std::string::npos) {
        mapping.channel_ = std::stoi(json.substr(pos + 10));
    }
    
    // Extract controllerNumber
    pos = json.find("\"controllerNumber\":");
    if (pos != std::string::npos) {
        mapping.controllerNumber_ = std::stoi(json.substr(pos + 19));
    }
    
    // Extract curveType
    pos = json.find("\"curveType\":");
    if (pos != std::string::npos) {
        int val = std::stoi(json.substr(pos + 12));
        mapping.curveType_ = static_cast<CurveType>(val);
    }
    
    // Extract minValue
    pos = json.find("\"minValue\":");
    if (pos != std::string::npos) {
        mapping.minValue_ = std::stof(json.substr(pos + 11));
    }
    
    // Extract maxValue
    pos = json.find("\"maxValue\":");
    if (pos != std::string::npos) {
        mapping.maxValue_ = std::stof(json.substr(pos + 11));
    }
    
    // Extract targetFunction
    pos = json.find("\"targetFunction\":\"");
    if (pos != std::string::npos) {
        size_t endpos = json.find("\"", pos + 18);
        if (endpos != std::string::npos) {
            mapping.targetFunction_ = json.substr(pos + 18, endpos - (pos + 18));
        }
    }
    
    // Extract jogWheelAcceleration
    pos = json.find("\"jogWheelAcceleration\":");
    if (pos != std::string::npos) {
        std::string val = json.substr(pos + 23, 4);  // "true" or "fals"
        mapping.jogWheelAcceleration_ = (val == "true");
    }
    
    return mapping;
}

} // namespace dj::input
