/**
 * Fuzz Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Classic fuzz with asymmetric clipping
 *
 * Hardware Control Mapping:
 *   KNOB_1: Fuzz (fuzz intensity)
 *   KNOB_2: Tone (high-cut filter)
 *   KNOB_3: Gate (noise gate threshold)
 *   KNOB_4: Level (output volume)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend)
 *   TOGGLESWITCH_1: Character (UP=vintage, MIDDLE=modern, DOWN=octave)
 */

#include "hothouse.h"
#include <math.h>

#define MAX_FUZZ_GAIN 200.0f

class Fuzz : public HothouseEffect {
private:
    // Smoothed parameters
    ParameterSmoother smoothFuzz;
    ParameterSmoother smoothTone;
    ParameterSmoother smoothGate;
    ParameterSmoother smoothLevel;
    ParameterSmoother smoothMix;

    float previousSample;
    float dcBlocker;

    // Character mode (0=vintage, 1=modern, 2=octave)
    int character;

    // Asymmetric clipping for vintage fuzz
    float vintageClip(float sample) {
        if (sample > 0.5f) {
            return 0.5f + (sample - 0.5f) * 0.1f;
        } else if (sample < -0.6f) {
            return -0.6f + (sample + 0.6f) * 0.15f;
        }
        return sample;
    }

    // Harder clipping for modern fuzz
    float modernClip(float sample) {
        if (sample > 0.4f) return 0.4f;
        if (sample < -0.4f) return -0.4f;
        return sample;
    }

    // Octave fuzz (full-wave rectification + clipping)
    float octaveClip(float sample) {
        float rectified = fabsf(sample);
        if (rectified > 0.5f) rectified = 0.5f;
        return rectified * (sample > 0 ? 1.0f : -1.0f) * 0.5f + sample * 0.5f;
    }

    // DC blocker
    float dcBlock(float sample) {
        float output = sample - dcBlocker;
        dcBlocker = sample * 0.995f;
        return output;
    }

    // One-pole lowpass for tone
    float lowPass(float sample, float alpha) {
        float filtered = alpha * sample + (1.0f - alpha) * previousSample;
        previousSample = filtered;
        return filtered;
    }

public:
    Fuzz(int sampleRate = 48000)
        : smoothFuzz(20.0f, (float)sampleRate, 0.7f),
          smoothTone(20.0f, (float)sampleRate, 0.5f),
          smoothGate(20.0f, (float)sampleRate, 0.0f),
          smoothLevel(20.0f, (float)sampleRate, 0.7f),
          smoothMix(20.0f, (float)sampleRate, 1.0f) {
        previousSample = 0.0f;
        dcBlocker = 0.0f;
        character = 0;
    }

    void updateFromControls(const HothouseControls& controls) override {
        smoothFuzz.setTarget(controls.knobs[KNOB_1]);
        smoothTone.setTarget(controls.knobs[KNOB_2]);
        smoothGate.setTarget(controls.knobs[KNOB_3]);
        smoothLevel.setTarget(controls.knobs[KNOB_4]);
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Character mode
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                character = 0;  // Vintage
                break;
            case TOGGLESWITCH_MIDDLE:
                character = 1;  // Modern
                break;
            case TOGGLESWITCH_DOWN:
                character = 2;  // Octave
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        return 1.0f;
    }

    float process(float inputSample) override {
        float fuzz = smoothFuzz.process();
        float tone = smoothTone.process();
        float gate = smoothGate.process();
        float level = smoothLevel.process();
        float mix = smoothMix.process();

        // Noise gate
        float gateThreshold = gate * 0.1f;
        if (fabsf(inputSample) < gateThreshold) {
            inputSample = 0.0f;
        }

        // Heavy pre-gain
        float amplified = inputSample * (1.0f + fuzz * (MAX_FUZZ_GAIN - 1.0f));

        // Apply clipping based on character
        float clipped;
        switch (character) {
            case 0:  // Vintage
                clipped = vintageClip(amplified);
                break;
            case 1:  // Modern
                clipped = modernClip(amplified);
                break;
            default: // Octave
                clipped = octaveClip(amplified);
                break;
        }

        // Remove DC offset
        clipped = dcBlock(clipped);

        // Apply tone control
        float toneAlpha = 0.2f + tone * 0.79f;
        float toned = lowPass(clipped, toneAlpha);

        // Mix dry/wet and apply level
        float output = inputSample * (1.0f - mix) + toned * mix;
        return output * level * 0.8f;
    }

    void reset() override {
        previousSample = 0.0f;
        dcBlocker = 0.0f;
    }
};
