/**
 * Cleveland Sound Hothouse Pedal
 * Base Effect Interface and Utilities
 * 
 * Common header file for all effect implementations
 */

#ifndef HOTHOUSE_H
#define HOTHOUSE_H

// Utility function to constrain values
inline float constrain(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * Base class for all effect pedal implementations
 * All effects must inherit from this class and implement the process() method
 */
class HothouseEffect {
public:
    virtual ~HothouseEffect() {}
    
    /**
     * Process a single audio sample through the effect
     * @param inputSample Input audio sample (typically -1.0 to 1.0)
     * @return Processed output sample
     */
    virtual float process(float inputSample) = 0;
    
    /**
     * Reset the effect state (clear buffers, reset phase, etc.)
     */
    virtual void reset() = 0;
};

/**
 * Configuration for Cleveland Sound Hothouse hardware
 */
struct HothouseConfig {
    int sampleRate;          // Audio sample rate (default: 48000 Hz)
    int bufferSize;          // Audio buffer size (default: 128 samples)
    int adcResolution;       // ADC resolution in bits (default: 24)
    int dacResolution;       // DAC resolution in bits (default: 24)
    
    HothouseConfig() {
        sampleRate = 48000;
        bufferSize = 128;
        adcResolution = 24;
        dacResolution = 24;
    }
};

/**
 * Cleveland Sound Hothouse Pedal Controller
 * Manages effect processing and hardware interface
 */
class HothousePedal {
private:
    HothouseEffect* currentEffect;
    HothouseConfig config;
    bool bypassed;
    
public:
    HothousePedal(HothouseConfig cfg = HothouseConfig()) 
        : currentEffect(nullptr), config(cfg), bypassed(false) {}
    
    void setEffect(HothouseEffect* effect) {
        currentEffect = effect;
    }
    
    void bypass(bool enable) {
        bypassed = enable;
    }
    
    bool isBypassed() const {
        return bypassed;
    }
    
    float process(float inputSample) {
        if (bypassed || currentEffect == nullptr) {
            return inputSample;  // Pass through
        }
        return currentEffect->process(inputSample);
    }
    
    void processBuffer(float* inputBuffer, float* outputBuffer, int numSamples) {
        for (int i = 0; i < numSamples; i++) {
            outputBuffer[i] = process(inputBuffer[i]);
        }
    }
    
    HothouseConfig getConfig() const {
        return config;
    }
};

#endif // HOTHOUSE_H
