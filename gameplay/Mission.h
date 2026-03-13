#pragma once

#include <string>

namespace dj {

class Mission {
public:
    virtual ~Mission() = default;
    
    // Mission lifecycle
    virtual void setup() = 0;
    virtual void update(int blockNumber) = 0;
    virtual void teardown() = 0;
    
    // Mission status
    virtual bool isComplete() const = 0;
    virtual bool isFailed() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    
    // Score/progress
    virtual float getScore() const = 0;
    virtual float getProgress() const = 0;  // 0.0-1.0
};

}  // namespace dj
