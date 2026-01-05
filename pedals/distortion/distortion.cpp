/**
 * Distortion Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Hard clipping distortion with pre and post filtering
 */

#include "hothouse.h"

class Distortion : public HothouseEffect {
private:
    float gain;
    float tone;
    float level;
    float previousSample;
    float dcBlocker;
    
    // Hard clipping function
    float hardClip(float sample, float threshold) {
        if (sample > threshold) return threshold;
        if (sample < -threshold) return -threshold;
        return sample;
    }
    
    // Simple high-pass filter to remove DC offset
    float dcBlock(float sample) {
        float output = sample - dcBlocker;
        dcBlocker = sample * 0.995f;
        return output;
    }
    
    // One-pole low-pass filter
    float lowPass(float sample) {
        float alpha = tone;
        float filtered = alpha * sample + (1.0f - alpha) * previousSample;
        previousSample = filtered;
        return filtered;
    }
    
public:
    Distortion() {
        gain = 0.5f;
        tone = 0.6f;
        level = 0.7f;
        previousSample = 0.0f;
        dcBlocker = 0.0f;
    }
    
    void setGain(float g) { gain = constrain(g, 0.0f, 1.0f); }
    void setTone(float t) { tone = constrain(t, 0.0f, 1.0f); }
    void setLevel(float l) { level = constrain(l, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Remove DC offset
        float sample = dcBlock(inputSample);
        
        // Apply gain
        float amplified = sample * (1.0f + gain * 99.0f);
        
        // Hard clipping
        float clipped = hardClip(amplified, 0.7f);
        
        // Apply tone control (low-pass filter)
        float toned = lowPass(clipped);
        
        // Apply output level
        return toned * level;
    }
    
    void reset() override {
        previousSample = 0.0f;
        dcBlocker = 0.0f;
    }
};
