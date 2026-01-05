/**
 * Compressor Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Dynamic range compressor with envelope follower
 */

#include "hothouse.h"
#include <math.h>

class Compressor : public HothouseEffect {
private:
    float threshold;    // Compression threshold (0.0 to 1.0)
    float ratio;        // Compression ratio (1.0 to 20.0)
    float attack;       // Attack time coefficient
    float release;      // Release time coefficient
    float envelope;     // Current envelope level
    float makeupGain;   // Output gain compensation
    
    float getEnvelope(float sample) {
        float rectified = fabsf(sample);
        
        // Envelope follower
        if (rectified > envelope) {
            envelope = attack * envelope + (1.0f - attack) * rectified;
        } else {
            envelope = release * envelope + (1.0f - release) * rectified;
        }
        
        return envelope;
    }
    
    float computeGain(float envLevel) {
        if (envLevel < threshold) {
            return 1.0f;
        }
        
        // Calculate gain reduction
        float overThreshold = envLevel - threshold;
        float gainReduction = overThreshold * (1.0f - 1.0f / ratio);
        float targetGain = 1.0f / (1.0f + gainReduction);
        
        return targetGain;
    }
    
public:
    Compressor() {
        threshold = 0.5f;
        ratio = 4.0f;
        attack = 0.9f;      // Fast attack
        release = 0.999f;   // Slow release
        envelope = 0.0f;
        makeupGain = 2.0f;
    }
    
    void setThreshold(float t) { threshold = constrain(t, 0.0f, 1.0f); }
    void setRatio(float r) { ratio = constrain(r, 1.0f, 20.0f); }
    void setAttack(float a) { attack = constrain(a, 0.5f, 0.99f); }
    void setRelease(float r) { release = constrain(r, 0.9f, 0.999f); }
    void setMakeupGain(float g) { makeupGain = constrain(g, 0.5f, 10.0f); }
    
    float process(float inputSample) override {
        // Get envelope level
        float envLevel = getEnvelope(inputSample);
        
        // Compute gain reduction
        float gain = computeGain(envLevel);
        
        // Apply compression and makeup gain
        return inputSample * gain * makeupGain;
    }
    
    void reset() override {
        envelope = 0.0f;
    }
};
