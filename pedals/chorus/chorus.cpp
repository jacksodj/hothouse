/**
 * Chorus Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Chorus effect using time-varying delay with LFO
 */

#include "hothouse.h"
#include <math.h>

#define MAX_CHORUS_DELAY 4800  // 100ms at 48kHz

class Chorus : public HothouseEffect {
private:
    float delayBuffer[MAX_CHORUS_DELAY];
    int writeIndex;
    float lfoPhase;
    float sampleRate;
    
    float rate;       // LFO rate in Hz
    float depth;      // Modulation depth (0.0 to 1.0)
    float mix;        // Dry/wet mix
    
    // Generate triangle wave LFO (-1 to +1)
    // Uses phase wrapping and absolute value to create symmetric triangle
    float getLFO() {
        // Triangle wave formula: converts sawtooth to triangle
        // by taking absolute value of centered sawtooth
        float value = 2.0f * fabsf(2.0f * (lfoPhase - floorf(lfoPhase + 0.5f))) - 1.0f;
        return value;
    }
    
public:
    Chorus(int sr = 48000) : sampleRate((float)sr) {
        rate = 1.0f;
        depth = 0.5f;
        mix = 0.5f;
        writeIndex = 0;
        lfoPhase = 0.0f;
        
        for (int i = 0; i < MAX_CHORUS_DELAY; i++) {
            delayBuffer[i] = 0.0f;
        }
    }
    
    void setRate(float r) { rate = constrain(r, 0.1f, 10.0f); }
    void setDepth(float d) { depth = constrain(d, 0.0f, 1.0f); }
    void setMix(float m) { mix = constrain(m, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Update LFO phase
        lfoPhase += rate / sampleRate;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        
        // Calculate modulated delay time
        float lfoValue = getLFO();
        float delayTime = 10.0f + depth * 15.0f;  // 10-25ms delay range
        float modulatedDelay = delayTime + lfoValue * depth * 5.0f;
        int delaySamples = (int)(modulatedDelay * sampleRate / 1000.0f);
        
        // Calculate read position with linear interpolation
        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0) readIndex += MAX_CHORUS_DELAY;
        
        // Read delayed sample
        float delayedSample = delayBuffer[readIndex];
        
        // Write current sample to buffer
        delayBuffer[writeIndex] = inputSample;
        writeIndex++;
        if (writeIndex >= MAX_CHORUS_DELAY) writeIndex = 0;
        
        // Mix dry and wet signals
        return inputSample * (1.0f - mix) + delayedSample * mix;
    }
    
    void reset() override {
        writeIndex = 0;
        lfoPhase = 0.0f;
        for (int i = 0; i < MAX_CHORUS_DELAY; i++) {
            delayBuffer[i] = 0.0f;
        }
    }
};
