#include "core/SessionState.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace dj {

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
    oss << "    \"trackPath\": \"" << state.deckA.trackPath << "\",\n";
    oss << "    \"playbackPosition\": " << state.deckA.playbackPosition << ",\n";
    oss << "    \"tempoBend\": " << state.deckA.tempoBend << ",\n";
    oss << "    \"isPlaying\": " << (state.deckA.isPlaying ? "true" : "false") << ",\n";
    oss << "    \"lowGain\": " << state.deckA.lowGain << ",\n";
    oss << "    \"midGain\": " << state.deckA.midGain << ",\n";
    oss << "    \"highGain\": " << state.deckA.highGain << "\n";
    oss << "  },\n";
    
    // Deck B
    oss << "  \"deckB\": {\n";
    oss << "    \"trackPath\": \"" << state.deckB.trackPath << "\",\n";
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
    oss << "  \"venueId\": \"" << state.venueId << "\"\n";
    
    oss << "}\n";
    
    return oss.str();
}

std::optional<SessionState> SessionManager::deserializeFromJSON(const std::string& json) {
    try {
        SessionState state;
        
        // Parse deckA.trackPath
        {
            size_t pos = json.find("\"trackPath\": \"");
            if (pos != std::string::npos) {
                size_t valueStart = pos + 14;  // Right after opening quote
                size_t quoteEnd = json.find("\"", valueStart);
                if (quoteEnd != std::string::npos) {
                    state.deckA.trackPath = json.substr(valueStart, quoteEnd - valueStart);
                }
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
                size_t pos = json.find("\"trackPath\": \"", deckBStart);
                if (pos != std::string::npos) {
                    size_t valueStart = pos + 14;
                    size_t quoteEnd = json.find("\"", valueStart);
                    if (quoteEnd != std::string::npos) {
                        state.deckB.trackPath = json.substr(valueStart, quoteEnd - valueStart);
                    }
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
            size_t pos = json.find("\"venueId\": \"");
            if (pos != std::string::npos) {
                size_t valueStart = pos + 12;  // "venueId": " = 12 characters
                size_t quoteEnd = json.find("\"", valueStart);
                if (quoteEnd != std::string::npos) {
                    state.venueId = json.substr(valueStart, quoteEnd - valueStart);
                }
            }
        }
        
        return state;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace dj
