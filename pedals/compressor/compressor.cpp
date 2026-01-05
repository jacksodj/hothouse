/**
 * Compressor Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Dynamic range compressor with envelope follower
 *
 * Hardware Control Mapping:
 *   KNOB_1: Threshold (compression threshold)
 *   KNOB_2: Ratio (compression ratio 1:1 to 20:1)
 *   KNOB_3: Attack (envelope attack time)
 *   KNOB_4: Release (envelope release time)
 *   KNOB_5: Makeup Gain (output gain compensation)
 *   KNOB_6: Mix (dry/wet for parallel compression)
 *   TOGGLESWITCH_1: Knee mode (UP=hard, MIDDLE=medium, DOWN=soft)
 */

#include "hothouse.h"
#include <math.h>

class Compressor : public HothouseEffect {
private:
    // Smoothed parameters
    ParameterSmoother smoothThreshold;
    ParameterSmoother smoothRatio;
    ParameterSmoother smoothAttack;
    ParameterSmoother smoothRelease;
    ParameterSmoother smoothMakeup;
    ParameterSmoother smoothMix;

    float envelope;
    float gainReductionDb;  // For LED metering

    // Knee mode (0=hard, 1=medium, 2=soft)
    int kneeMode;
    float kneeWidth;

    float getEnvelope(float sample, float attack, float release) {
        float rectified = fabsf(sample);

        if (rectified > envelope) {
            envelope = attack * envelope + (1.0f - attack) * rectified;
        } else {
            envelope = release * envelope + (1.0f - release) * rectified;
        }

        return envelope;
    }

    float computeGain(float envLevel, float threshold, float ratio) {
        if (envLevel < 0.0001f) return 1.0f;

        // Convert to dB
        float envDb = 20.0f * log10f(envLevel);
        float threshDb = 20.0f * log10f(threshold + 0.0001f);

        float gainDb = 0.0f;

        if (kneeMode == 0) {
            // Hard knee
            if (envDb > threshDb) {
                gainDb = threshDb + (envDb - threshDb) / ratio - envDb;
            }
        } else {
            // Soft knee
            float knee = kneeWidth;
            if (envDb < threshDb - knee / 2.0f) {
                gainDb = 0.0f;
            } else if (envDb > threshDb + knee / 2.0f) {
                gainDb = threshDb + (envDb - threshDb) / ratio - envDb;
            } else {
                // In the knee region
                float x = envDb - threshDb + knee / 2.0f;
                gainDb = ((1.0f / ratio - 1.0f) * x * x) / (2.0f * knee);
            }
        }

        gainReductionDb = -gainDb;  // Store for LED
        return powf(10.0f, gainDb / 20.0f);
    }

public:
    Compressor(int sampleRate = 48000)
        : smoothThreshold(20.0f, (float)sampleRate, 0.5f),
          smoothRatio(20.0f, (float)sampleRate, 0.25f),
          smoothAttack(20.0f, (float)sampleRate, 0.3f),
          smoothRelease(20.0f, (float)sampleRate, 0.5f),
          smoothMakeup(20.0f, (float)sampleRate, 0.5f),
          smoothMix(20.0f, (float)sampleRate, 1.0f) {
        envelope = 0.0f;
        gainReductionDb = 0.0f;
        kneeMode = 0;
        kneeWidth = 6.0f;
    }

    void updateFromControls(const HothouseControls& controls) override {
        // KNOB_1: Threshold (0.01 to 1.0)
        smoothThreshold.setTarget(0.01f + controls.knobs[KNOB_1] * 0.99f);

        // KNOB_2: Ratio (1:1 to 20:1)
        smoothRatio.setTarget(1.0f + controls.knobs[KNOB_2] * 19.0f);

        // KNOB_3: Attack (fast to slow)
        smoothAttack.setTarget(0.5f + controls.knobs[KNOB_3] * 0.49f);

        // KNOB_4: Release (fast to slow)
        smoothRelease.setTarget(0.9f + controls.knobs[KNOB_4] * 0.099f);

        // KNOB_5: Makeup gain (1x to 10x)
        smoothMakeup.setTarget(1.0f + controls.knobs[KNOB_5] * 9.0f);

        // KNOB_6: Mix (parallel compression)
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Knee mode
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                kneeMode = 0;
                kneeWidth = 0.0f;
                break;
            case TOGGLESWITCH_MIDDLE:
                kneeMode = 1;
                kneeWidth = 6.0f;
                break;
            case TOGGLESWITCH_DOWN:
                kneeMode = 2;
                kneeWidth = 12.0f;
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        // LED brightness shows gain reduction (brighter = more compression)
        float ledValue = gainReductionDb / 20.0f;  // Normalize to 0-1 range
        if (ledValue > 1.0f) ledValue = 1.0f;
        if (ledValue < 0.0f) ledValue = 0.0f;
        return 1.0f - ledValue * 0.8f;  // Dim when compressing hard
    }

    float process(float inputSample) override {
        float threshold = smoothThreshold.process();
        float ratio = smoothRatio.process();
        float attack = smoothAttack.process();
        float release = smoothRelease.process();
        float makeup = smoothMakeup.process();
        float mix = smoothMix.process();

        // Get envelope level
        float envLevel = getEnvelope(inputSample, attack, release);

        // Compute gain reduction
        float gain = computeGain(envLevel, threshold, ratio);

        // Apply compression and makeup gain
        float compressed = inputSample * gain * makeup;

        // Clip output to prevent extreme levels
        if (compressed > 1.0f) compressed = 1.0f;
        if (compressed < -1.0f) compressed = -1.0f;

        // Mix dry and compressed (parallel compression)
        return inputSample * (1.0f - mix) + compressed * mix;
    }

    void reset() override {
        envelope = 0.0f;
        gainReductionDb = 0.0f;
    }
};
