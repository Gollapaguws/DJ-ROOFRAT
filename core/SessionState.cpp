#include "core/SessionState.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace dj {

namespace {

std::string escapeJSONString(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.size());

    for (char ch : input) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }

    return escaped;
}

std::optional<std::string> parseJSONStringField(const std::string& json,
                                                const std::string& fieldPrefix,
                                                size_t searchFrom = 0) {
    const size_t prefixPos = json.find(fieldPrefix, searchFrom);
    if (prefixPos == std::string::npos) {
        return std::nullopt;
    }

    const size_t valueStart = prefixPos + fieldPrefix.size();
    std::string value;
    value.reserve(64);

    bool escaping = false;
    for (size_t i = valueStart; i < json.size(); ++i) {
        const char ch = json[i];

        if (escaping) {
            switch (ch) {
            case '\\':
                value.push_back('\\');
                break;
            case '"':
                value.push_back('"');
                break;
            case 'n':
                value.push_back('\n');
                break;
            case 'r':
                value.push_back('\r');
                break;
            case 't':
                value.push_back('\t');
                break;
            default:
                value.push_back(ch);
                break;
            }
            escaping = false;
            continue;
        }

        if (ch == '\\') {
            escaping = true;
            continue;
        }

        if (ch == '"') {
            return value;
        }

        value.push_back(ch);
    }

    return std::nullopt;
}

} // namespace

SessionManager::SessionManager()
    : autoSaveEnabled_(false)
    , autoSaveIntervalSeconds_(120)
{
}

bool SessionManager::saveSession(const std::string& filePath, const SessionState& state) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << serializeToJSON(state);
    file.close();
    
    return true;
}

std::optional<SessionState> SessionManager::loadSession(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return deserializeFromJSON(buffer.str());
}

void SessionManager::enableAutoSave(bool enabled) {
    autoSaveEnabled_ = enabled;
}

bool SessionManager::isAutoSaveEnabled() const {
    return autoSaveEnabled_;
}

void SessionManager::setAutoSaveInterval(int seconds) {
    autoSaveIntervalSeconds_ = std::max(10, std::min(seconds, 3600));
}

std::string SessionManager::serializeToJSON(const SessionState& state) const {
    std::ostringstream oss;
    
    oss << "{\n";
    
    // Deck A
    oss << "  \"deckA\": {\n";
    oss << "    \"trackPath\": \"" << escapeJSONString(state.deckA.trackPath) << "\",\n";
    oss << "    \"playbackPosition\": " << state.deckA.playbackPosition << ",\n";
    oss << "    \"tempoBend\": " << state.deckA.tempoBend << ",\n";
    oss << "    \"isPlaying\": " << (state.deckA.isPlaying ? "true" : "false") << ",\n";
    oss << "    \"lowGain\": " << state.deckA.lowGain << ",\n";
    oss << "    \"midGain\": " << state.deckA.midGain << ",\n";
    oss << "    \"highGain\": " << state.deckA.highGain << "\n";
    oss << "  },\n";
    
    // Deck B
    oss << "  \"deckB\": {\n";
    oss << "    \"trackPath\": \"" << escapeJSONString(state.deckB.trackPath) << "\",\n";
    oss << "    \"playbackPosition\": " << state.deckB.playbackPosition << ",\n";
    oss << "    \"tempoBend\": " << state.deckB.tempoBend << ",\n";
    oss << "    \"isPlaying\": " << (state.deckB.isPlaying ? "true" : "false") << ",\n";
    oss << "    \"lowGain\": " << state.deckB.lowGain << ",\n";
    oss << "    \"midGain\": " << state.deckB.midGain << ",\n";
    oss << "    \"highGain\": " << state.deckB.highGain << "\n";
    oss << "  },\n";
    
    // Session state
    oss << "  \"crossfader\": " << state.crossfader << ",\n";
    oss << "  \"currentCareerTier\": " << state.currentCareerTier << ",\n";
    oss << "  \"crowdEnergy\": " << state.crowdEnergy << ",\n";
    oss << "  \"venueId\": \"" << escapeJSONString(state.venueId) << "\"\n";
    
    oss << "}\n";
    
    return oss.str();
}

std::optional<SessionState> SessionManager::deserializeFromJSON(const std::string& json) {
    try {
        SessionState state;
        
        // Parse deckA.trackPath
        {
            auto trackPath = parseJSONStringField(json, "\"trackPath\": \"");
            if (trackPath.has_value()) {
                state.deckA.trackPath = *trackPath;
            }
        }
        
        // Parse deckA.playbackPosition
        {
            size_t pos = json.find("\"playbackPosition\": ");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        state.deckA.playbackPosition = std::stod(valueStr);
                    }
                }
            }
        }
        
        // Parse deckA.tempoBend
        {
            size_t pos = json.find("\"tempoBend\": ");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        state.deckA.tempoBend = std::clamp(std::stof(valueStr), -0.1f, 0.1f);
                    }
                }
            }
        }
        
        // Parse deckA.isPlaying
        {
            size_t pos = json.find("\"isPlaying\": ");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        state.deckA.isPlaying = (valueStr == "true");
                    }
                }
            }
        }
        
        // Parse deckA.lowGain
        {
            size_t deckAEnd = json.find("\"deckB\":");
            if (deckAEnd != std::string::npos) {
                size_t pos = json.rfind("\"lowGain\": ", deckAEnd);
                if (pos != std::string::npos && pos < deckAEnd) {
                    size_t colonPos = json.find(":", pos);
                    if (colonPos != std::string::npos) {
                        size_t commaPos = json.find(",", colonPos);
                        if (commaPos != std::string::npos) {
                            std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                            valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                            valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                            state.deckA.lowGain = std::stof(valueStr);
                        }
                    }
                }
            }
        }
        
        // Parse deckA.midGain
        {
            size_t deckAEnd = json.find("\"deckB\":");
            if (deckAEnd != std::string::npos) {
                size_t pos = json.rfind("\"midGain\": ", deckAEnd);
                if (pos != std::string::npos && pos < deckAEnd) {
                    size_t colonPos = json.find(":", pos);
                    if (colonPos != std::string::npos) {
                        size_t commaPos = json.find(",", colonPos);
                        if (commaPos != std::string::npos) {
                            std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                            valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                            valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                            state.deckA.midGain = std::stof(valueStr);
                        }
                    }
                }
            }
        }
        
        // Parse deckA.highGain
        {
            size_t deckAEnd = json.find("\"deckB\":");
            if (deckAEnd != std::string::npos) {
                size_t pos = json.rfind("\"highGain\": ", deckAEnd);
                if (pos != std::string::npos && pos < deckAEnd) {
                    size_t colonPos = json.find(":", pos);
                    if (colonPos != std::string::npos) {
                        size_t commaPos = json.find(",", colonPos);
                        if (commaPos == std::string::npos) {
                            commaPos = json.find("\n", colonPos);
                        }
                        if (commaPos != std::string::npos) {
                            std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                            valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                            valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                            state.deckA.highGain = std::stof(valueStr);
                        }
                    }
                }
            }
        }
        
        // Parse deckB.trackPath
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                auto trackPath = parseJSONStringField(json, "\"trackPath\": \"", deckBStart);
                if (trackPath.has_value()) {
                    state.deckB.trackPath = *trackPath;
                }
            }
        }
        
        // Parse deckB.playbackPosition
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                size_t pos = json.find("\"playbackPosition\": ", deckBStart);
                if (pos != std::string::npos) {
                    size_t colonPos = json.find(":", pos);
                    if (colonPos != std::string::npos) {
                        size_t commaPos = json.find(",", colonPos);
                        if (commaPos != std::string::npos) {
                            std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                            valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                            valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                            state.deckB.playbackPosition = std::stod(valueStr);
                        }
                    }
                }
            }
        }
        
        // Parse deckB.tempoBend
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                size_t pos = json.find("\"tempoBend\": ", deckBStart);
                if (pos != std::string::npos) {
                    size_t colonPos = json.find(":", pos);
                    if (colonPos != std::string::npos) {
                        size_t commaPos = json.find(",", colonPos);
                        if (commaPos != std::string::npos) {
                            std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                            valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                            valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                            state.deckB.tempoBend = std::clamp(std::stof(valueStr), -0.1f, 0.1f);
                        }
                    }
                }
            }
        }
        
        // Parse deckB.isPlaying
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                size_t pos = json.find("\"isPlaying\": ", deckBStart);
                if (pos != std::string::npos) {
                    size_t colonPos = json.find(":", pos);
                    if (colonPos != std::string::npos) {
                        size_t commaPos = json.find(",", colonPos);
                        if (commaPos != std::string::npos) {
                            std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                            valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                            valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                            state.deckB.isPlaying = (valueStr == "true");
                        }
                    }
                }
            }
        }
        
        // Parse deckB.lowGain
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                size_t endBrace = json.find("}", deckBStart);
                if (endBrace != std::string::npos) {
                    size_t pos = json.find("\"lowGain\": ", deckBStart);
                    if (pos != std::string::npos && pos < endBrace) {
                        size_t colonPos = json.find(":", pos);
                        if (colonPos != std::string::npos) {
                            size_t commaPos = json.find(",", colonPos);
                            if (commaPos != std::string::npos) {
                                std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                                valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                state.deckB.lowGain = std::stof(valueStr);
                            }
                        }
                    }
                }
            }
        }
        
        // Parse deckB.midGain
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                size_t endBrace = json.find("}", deckBStart);
                if (endBrace != std::string::npos) {
                    size_t pos = json.find("\"midGain\": ", deckBStart);
                    if (pos != std::string::npos && pos < endBrace) {
                        size_t colonPos = json.find(":", pos);
                        if (colonPos != std::string::npos) {
                            size_t commaPos = json.find(",", colonPos);
                            if (commaPos != std::string::npos) {
                                std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                                valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                state.deckB.midGain = std::stof(valueStr);
                            }
                        }
                    }
                }
            }
        }
        
        // Parse deckB.highGain
        {
            size_t deckBStart = json.find("\"deckB\":");
            if (deckBStart != std::string::npos) {
                size_t endBrace = json.find("}", deckBStart);
                if (endBrace != std::string::npos) {
                    size_t pos = json.find("\"highGain\": ", deckBStart);
                    if (pos != std::string::npos && pos < endBrace) {
                        size_t colonPos = json.find(":", pos);
                        if (colonPos != std::string::npos) {
                            size_t commaPos = json.find(",", colonPos);
                            if (commaPos == std::string::npos) {
                                commaPos = json.find("\n", colonPos);
                            }
                            if (commaPos != std::string::npos) {
                                std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                                valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                state.deckB.highGain = std::stof(valueStr);
                            }
                        }
                    }
                }
            }
        }
        
        // Parse crossfader
        {
            size_t pos = json.find("\"crossfader\": ");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        state.crossfader = std::clamp(std::stof(valueStr), -1.0f, 1.0f);
                    }
                }
            }
        }
        
        // Parse currentCareerTier
        {
            size_t pos = json.find("\"currentCareerTier\": ");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        state.currentCareerTier = std::max(1, std::stoi(valueStr));
                    }
                }
            }
        }
        
        // Parse crowdEnergy
        {
            size_t pos = json.find("\"crowdEnergy\": ");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        state.crowdEnergy = std::clamp(std::stof(valueStr), 0.0f, 1.0f);
                    }
                }
            }
        }
        
        // Parse venueId
        {
            auto venueId = parseJSONStringField(json, "\"venueId\": \"");
            if (venueId.has_value()) {
                state.venueId = *venueId;
            }
        }
        
        return state;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace dj
