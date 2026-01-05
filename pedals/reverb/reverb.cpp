/**
 * Reverb Effect Pedal
 * Cleveland Sound Hothouse Implementation
 * 
 * Simple reverb using Schroeder reverberator algorithm
 */

#include "hothouse.h"

#define NUM_COMB_FILTERS 4
#define NUM_ALLPASS_FILTERS 2

// Comb filter delay times (in samples at 48kHz)
const int combDelays[NUM_COMB_FILTERS] = {1557, 1617, 1491, 1422};
const int allpassDelays[NUM_ALLPASS_FILTERS] = {225, 556};

class CombFilter {
private:
    float* buffer;
    int bufferSize;
    int index;
    float feedback;
    
public:
    CombFilter(int size) : bufferSize(size), index(0), feedback(0.7f) {
        buffer = new float[bufferSize];
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
    }
    
    ~CombFilter() { delete[] buffer; }
    
    void setFeedback(float fb) { feedback = fb; }
    
    float process(float input) {
        float output = buffer[index];
        buffer[index] = input + output * feedback;
        index = (index + 1) % bufferSize;
        return output;
    }
    
    void clear() {
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
        index = 0;
    }
};

class AllpassFilter {
private:
    float* buffer;
    int bufferSize;
    int index;
    
public:
    AllpassFilter(int size) : bufferSize(size), index(0) {
        buffer = new float[bufferSize];
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
    }
    
    ~AllpassFilter() { delete[] buffer; }
    
    float process(float input) {
        float bufOut = buffer[index];
        float output = -input + bufOut;
        buffer[index] = input + bufOut * 0.5f;
        index = (index + 1) % bufferSize;
        return output;
    }
    
    void clear() {
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
        index = 0;
    }
};

class Reverb : public HothouseEffect {
private:
    CombFilter* combFilters[NUM_COMB_FILTERS];
    AllpassFilter* allpassFilters[NUM_ALLPASS_FILTERS];
    float roomSize;
    float damping;
    float mix;
    
public:
    Reverb() {
        roomSize = 0.5f;
        damping = 0.5f;
        mix = 0.3f;
        
        // Initialize filters
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combFilters[i] = new CombFilter(combDelays[i]);
            combFilters[i]->setFeedback(0.7f);
        }
        
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) {
            allpassFilters[i] = new AllpassFilter(allpassDelays[i]);
        }
    }
    
    ~Reverb() {
        for (int i = 0; i < NUM_COMB_FILTERS; i++) delete combFilters[i];
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) delete allpassFilters[i];
    }
    
    void setRoomSize(float size) {
        roomSize = constrain(size, 0.0f, 1.0f);
        float feedback = 0.5f + roomSize * 0.4f;
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combFilters[i]->setFeedback(feedback);
        }
    }
    
    void setDamping(float damp) { damping = constrain(damp, 0.0f, 1.0f); }
    void setMix(float m) { mix = constrain(m, 0.0f, 1.0f); }
    
    float process(float inputSample) override {
        // Parallel comb filters
        float combOut = 0.0f;
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combOut += combFilters[i]->process(inputSample);
        }
        combOut /= NUM_COMB_FILTERS;
        
        // Series allpass filters
        float output = combOut;
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) {
            output = allpassFilters[i]->process(output);
        }
        
        // Mix dry and wet
        return inputSample * (1.0f - mix) + output * mix;
    }
    
    void reset() override {
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combFilters[i]->clear();
        }
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) {
            allpassFilters[i]->clear();
        }
    }
};
