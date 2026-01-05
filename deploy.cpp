/**
 * Cleveland Sound Hothouse Pedal
 * Example Deployment Code
 * 
 * This example shows how to deploy effects to the Hothouse pedal
 */

#include "hothouse.h"
#include "pedals/overdrive/overdrive.cpp"
#include "pedals/delay/delay.cpp"
#include "pedals/reverb/reverb.cpp"
#include "pedals/chorus/chorus.cpp"
#include "pedals/distortion/distortion.cpp"
#include "pedals/fuzz/fuzz.cpp"
#include "pedals/tremolo/tremolo.cpp"
#include "pedals/compressor/compressor.cpp"

// Example: Simple effect chain
class EffectChain {
private:
    HothouseEffect** effects;
    int numEffects;
    
public:
    EffectChain(HothouseEffect** fx, int count) 
        : effects(fx), numEffects(count) {}
    
    float process(float input) {
        float output = input;
        for (int i = 0; i < numEffects; i++) {
            if (effects[i] != nullptr) {
                output = effects[i]->process(output);
            }
        }
        return output;
    }
    
    void reset() {
        for (int i = 0; i < numEffects; i++) {
            if (effects[i] != nullptr) {
                effects[i]->reset();
            }
        }
    }
};

// Main deployment code
int main() {
    // Configure hardware
    HothouseConfig config;
    config.sampleRate = 48000;
    config.bufferSize = 128;
    
    // Create pedal controller
    HothousePedal pedal(config);
    
    // Example 1: Single effect (Overdrive)
    Overdrive overdrive;
    overdrive.setDrive(0.6f);
    overdrive.setTone(0.7f);
    overdrive.setLevel(0.8f);
    pedal.setEffect(&overdrive);
    
    // Example 2: Effect chain (Compressor -> Overdrive -> Delay)
    Compressor comp;
    comp.setThreshold(0.5f);
    comp.setRatio(4.0f);
    
    Overdrive od;
    od.setDrive(0.7f);
    
    Delay delay(config.sampleRate);
    delay.setTime(0.4f);
    delay.setFeedback(0.3f);
    delay.setMix(0.3f);
    
    HothouseEffect* chain[] = {&comp, &od, &delay};
    EffectChain effectChain(chain, 3);
    
    // Example 3: Process audio buffer
    float inputBuffer[128];
    float outputBuffer[128];
    
    // In a real implementation, this would be in an audio callback
    // that receives input from ADC and sends output to DAC
    for (int i = 0; i < config.bufferSize; i++) {
        // Read from ADC (example: just using 0 for now)
        inputBuffer[i] = 0.0f;  // Replace with actual ADC read
        
        // Process through effect
        outputBuffer[i] = pedal.process(inputBuffer[i]);
        
        // Write to DAC (example: placeholder)
        // DAC_write(outputBuffer[i]);
    }
    
    // Alternative: Process entire buffer at once
    pedal.processBuffer(inputBuffer, outputBuffer, config.bufferSize);
    
    return 0;
}
