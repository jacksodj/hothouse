/**
 * Overdrive Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Classic tube-style overdrive with soft clipping
 */

#include "hothouse.h"

// Overdrive parameters
struct OverdriveParams {
    float drive;      // 0.0 to 1.0 - amount of overdrive
    float tone;       // 0.0 to 1.0 - tone control (low-pass filter cutoff)
    float level;      // 0.0 to 1.0 - output level
};

class Overdrive : public HothouseEffect {
private:
    OverdriveParams params;
    float previousSample;
    
    // Soft clipping function (tanh approximation)
    float softClip(float sample) {
        float x = sample * (1.0f + params.drive * 9.0f);
        // Fast tanh approximation
        if (x > 1.0f) return 0.76159f;
        if (x < -1.0f) return -0.76159f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }
    
    // Simple one-pole low-pass filter for tone control
    float toneLowPass(float sample) {
        float alpha = 0.5f + params.tone * 0.49f;
        float filtered = alpha * sample + (1.0f - alpha) * previousSample;
        previousSample = filtered;
        return filtered;
    }
    
public:
    Overdrive() {
        params.drive = 0.5f;
        params.tone = 0.7f;
        params.level = 0.8f;
        previousSample = 0.0f;
    }
    
    void setDrive(float drive) { params.drive = constrain(drive, 0.0f, 1.0f); }
    void setTone(float tone) { params.tone = constrain(tone, 0.0f, 1.0f); }
    void setLevel(float level) { params.level = constrain(level, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Apply overdrive with soft clipping
        float driven = softClip(inputSample);
        
        // Apply tone control
        float toned = toneLowPass(driven);
        
        // Apply output level
        return toned * params.level;
    }
    
    void reset() override {
        previousSample = 0.0f;
    }
};
