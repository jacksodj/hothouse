/**
 * Delay Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Digital delay with feedback control
 */

#include "hothouse.h"

#define MAX_DELAY_SAMPLES 48000  // 1 second at 48kHz

// Delay parameters
struct DelayParams {
    float time;       // 0.0 to 1.0 - delay time (0 to MAX_DELAY_SAMPLES)
    float feedback;   // 0.0 to 1.0 - amount of delayed signal fed back
    float mix;        // 0.0 to 1.0 - dry/wet mix
};

class Delay : public HothouseEffect {
private:
    DelayParams params;
    float delayBuffer[MAX_DELAY_SAMPLES];
    int writeIndex;
    int sampleRate;
    
public:
    Delay(int sampleRate = 48000) : sampleRate(sampleRate) {
        params.time = 0.5f;
        params.feedback = 0.5f;
        params.mix = 0.5f;
        writeIndex = 0;
        
        // Initialize buffer to zero
        for (int i = 0; i < MAX_DELAY_SAMPLES; i++) {
            delayBuffer[i] = 0.0f;
        }
    }
    
    void setTime(float time) { params.time = constrain(time, 0.0f, 1.0f); }
    void setFeedback(float feedback) { params.feedback = constrain(feedback, 0.0f, 0.95f); }
    void setMix(float mix) { params.mix = constrain(mix, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Calculate delay in samples
        int delaySamples = (int)(params.time * MAX_DELAY_SAMPLES);
        if (delaySamples < 1) delaySamples = 1;
        
        // Calculate read index
        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0) readIndex += MAX_DELAY_SAMPLES;
        
        // Read delayed sample
        float delayedSample = delayBuffer[readIndex];
        
        // Write to buffer with feedback
        delayBuffer[writeIndex] = inputSample + (delayedSample * params.feedback);
        
        // Increment write index
        writeIndex++;
        if (writeIndex >= MAX_DELAY_SAMPLES) writeIndex = 0;
        
        // Mix dry and wet signals
        return inputSample * (1.0f - params.mix) + delayedSample * params.mix;
    }
    
    void reset() override {
        writeIndex = 0;
        for (int i = 0; i < MAX_DELAY_SAMPLES; i++) {
            delayBuffer[i] = 0.0f;
        }
    }
};
