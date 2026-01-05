/**
 * Tremolo Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Amplitude modulation effect
 */

#include "hothouse.h"
#include <math.h>

class Tremolo : public HothouseEffect {
private:
    float rate;       // LFO rate in Hz
    float depth;      // Modulation depth
    float phase;      // LFO phase
    float sampleRate;
    
    float getLFO() {
        // Sine wave LFO
        return sinf(2.0f * 3.14159265f * phase);
    }
    
public:
    Tremolo(int sr = 48000) : sampleRate((float)sr) {
        rate = 4.0f;
        depth = 0.5f;
        phase = 0.0f;
    }
    
    void setRate(float r) { rate = constrain(r, 0.5f, 20.0f); }
    void setDepth(float d) { depth = constrain(d, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Get LFO value (-1 to 1)
        float lfo = getLFO();
        
        // Convert to amplitude multiplier (0 to 1)
        float amplitude = 1.0f - (depth * 0.5f * (1.0f + lfo));
        
        // Update phase
        phase += rate / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        
        // Apply amplitude modulation
        return inputSample * amplitude;
    }
    
    void reset() override {
        phase = 0.0f;
    }
};
