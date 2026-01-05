/**
 * Distortion Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Hard clipping distortion with pre and post filtering
 *
 * Hardware Control Mapping:
 *   KNOB_1: Gain (distortion amount)
 *   KNOB_2: Tone (high-cut filter)
 *   KNOB_3: Bass (low-end boost/cut)
 *   KNOB_4: Level (output volume)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend for parallel distortion)
 *   TOGGLESWITCH_1: Clipping mode (UP=hard, MIDDLE=medium, DOWN=soft)
 */

#include "hothouse.h"
#include <math.h>

#define MAX_DISTORTION_GAIN 100.0f

class Distortion : public HothouseEffect {
private:
    // Smoothed parameters
    ParameterSmoother smoothGain;
    ParameterSmoother smoothTone;
    ParameterSmoother smoothBass;
    ParameterSmoother smoothLevel;
    ParameterSmoother smoothMix;

    float previousSample;
    float dcBlocker;
    float bassState;

    // Clipping mode (0=hard, 1=medium, 2=soft)
    int clipMode;

    // Hard clipping function
    float hardClip(float sample, float threshold) {
        if (sample > threshold) return threshold;
        if (sample < -threshold) return -threshold;
        return sample;
    }

    // Soft clipping (tanh approximation)
    float softClip(float sample) {
        if (sample > 1.0f) return 0.76159f;
        if (sample < -1.0f) return -0.76159f;
        float x2 = sample * sample;
        return sample * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Simple high-pass filter to remove DC offset
    float dcBlock(float sample) {
        float output = sample - dcBlocker;
        dcBlocker = sample * 0.995f;
        return output;
    }

    // One-pole low-pass filter for tone
    float lowPass(float sample, float alpha) {
        float filtered = alpha * sample + (1.0f - alpha) * previousSample;
        previousSample = filtered;
        return filtered;
    }

public:
    Distortion(int sampleRate = 48000)
        : smoothGain(20.0f, (float)sampleRate, 0.5f),
          smoothTone(20.0f, (float)sampleRate, 0.6f),
          smoothBass(20.0f, (float)sampleRate, 0.5f),
          smoothLevel(20.0f, (float)sampleRate, 0.7f),
          smoothMix(20.0f, (float)sampleRate, 1.0f) {
        previousSample = 0.0f;
        dcBlocker = 0.0f;
        bassState = 0.0f;
        clipMode = 0;
    }

    void updateFromControls(const HothouseControls& controls) override {
        smoothGain.setTarget(controls.knobs[KNOB_1]);
        smoothTone.setTarget(controls.knobs[KNOB_2]);
        smoothBass.setTarget(controls.knobs[KNOB_3]);
        smoothLevel.setTarget(controls.knobs[KNOB_4]);
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Clipping mode
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                clipMode = 0;  // Hard
                break;
            case TOGGLESWITCH_MIDDLE:
                clipMode = 1;  // Medium
                break;
            case TOGGLESWITCH_DOWN:
                clipMode = 2;  // Soft
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        return 1.0f;
    }

    float process(float inputSample) override {
        float gain = smoothGain.process();
        float tone = smoothTone.process();
        float bass = smoothBass.process();
        float level = smoothLevel.process();
        float mix = smoothMix.process();

        // Remove DC offset
        float sample = dcBlock(inputSample);

        // Bass boost/cut (shelf-like behavior)
        float bassCoeff = 0.05f;
        bassState = bassState * (1.0f - bassCoeff) + sample * bassCoeff;
        float bassBoost = (bass - 0.5f) * 2.0f;  // -1 to +1
        sample = sample + bassState * bassBoost;

        // Apply gain
        float amplified = sample * (1.0f + gain * (MAX_DISTORTION_GAIN - 1.0f));

        // Apply clipping based on mode
        float clipped;
        switch (clipMode) {
            case 0:  // Hard
                clipped = hardClip(amplified, 0.7f);
                break;
            case 1:  // Medium
                clipped = hardClip(amplified, 0.85f);
                clipped = softClip(clipped * 0.8f);
                break;
            default: // Soft
                clipped = softClip(amplified * 0.5f);
                break;
        }

        // Apply tone control (low-pass filter)
        float toneAlpha = 0.3f + tone * 0.69f;
        float toned = lowPass(clipped, toneAlpha);

        // Mix dry/wet and apply output level
        float output = inputSample * (1.0f - mix) + toned * mix;
        return output * level;
    }

    void reset() override {
        previousSample = 0.0f;
        dcBlocker = 0.0f;
        bassState = 0.0f;
    }
};
