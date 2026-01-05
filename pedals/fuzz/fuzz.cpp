/**
 * Fuzz Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Classic fuzz with asymmetric clipping
 */

#include "hothouse.h"

class Fuzz : public HothouseEffect {
private:
    float fuzz;
    float level;
    
    // Asymmetric clipping for classic fuzz sound
    float asymmetricClip(float sample) {
        // Different clipping thresholds for positive and negative
        if (sample > 0.5f) {
            return 0.5f + (sample - 0.5f) * 0.1f;
        } else if (sample < -0.6f) {
            return -0.6f + (sample + 0.6f) * 0.15f;
        }
        return sample;
    }
    
public:
    Fuzz() {
        fuzz = 0.7f;
        level = 0.7f;
    }
    
    void setFuzz(float f) { fuzz = constrain(f, 0.0f, 1.0f); }
    void setLevel(float l) { level = constrain(l, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Heavy pre-gain
        float amplified = inputSample * (1.0f + fuzz * 199.0f);
        
        // Asymmetric clipping
        float clipped = asymmetricClip(amplified);
        
        // Post-gain to normalize
        return clipped * level * 0.8f;
    }
    
    void reset() override {
        // No state to reset
    }
};
