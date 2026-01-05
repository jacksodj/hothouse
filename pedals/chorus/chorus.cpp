/**
 * Chorus Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Chorus effect using time-varying delay with LFO
 *
 * Hardware Control Mapping:
 *   KNOB_1: Rate (LFO speed 0.1-5 Hz)
 *   KNOB_2: Depth (modulation amount)
 *   KNOB_3: (unused)
 *   KNOB_4: (unused)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend)
 *   TOGGLESWITCH_1: Waveform (UP=sine, MIDDLE=triangle, DOWN=square)
 */

#include "hothouse.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MAX_CHORUS_DELAY 4800  // 100ms at 48kHz

class Chorus : public HothouseEffect {
private:
    float delayBuffer[MAX_CHORUS_DELAY];
    int writeIndex;
    float lfoPhase;
    float sampleRate;

    // Smoothed parameters
    ParameterSmoother smoothRate;
    ParameterSmoother smoothDepth;
    ParameterSmoother smoothMix;

    // Waveform selection (0=sine, 1=triangle, 2=square)
    int waveform;

    // Generate LFO based on current waveform selection
    float getLFO() {
        switch (waveform) {
            case 1:  // Triangle
                return 2.0f * fabsf(2.0f * (lfoPhase - floorf(lfoPhase + 0.5f))) - 1.0f;
            case 2:  // Square
                return lfoPhase < 0.5f ? 1.0f : -1.0f;
            default: // Sine
                return sinf(2.0f * M_PI * lfoPhase);
        }
    }

public:
    Chorus(int sr = 48000)
        : sampleRate((float)sr),
          smoothRate(20.0f, (float)sr, 1.0f),
          smoothDepth(20.0f, (float)sr, 0.5f),
          smoothMix(20.0f, (float)sr, 0.5f) {
        writeIndex = 0;
        lfoPhase = 0.0f;
        waveform = 0;

        for (int i = 0; i < MAX_CHORUS_DELAY; i++) {
            delayBuffer[i] = 0.0f;
        }
    }

    void updateFromControls(const HothouseControls& controls) override {
        // KNOB_1: Rate (0.1 to 5 Hz)
        float rate = 0.1f + controls.knobs[KNOB_1] * 4.9f;
        smoothRate.setTarget(rate);

        // KNOB_2: Depth (0.0 to 1.0)
        smoothDepth.setTarget(controls.knobs[KNOB_2]);

        // KNOB_6: Mix (0.0 to 1.0)
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Waveform select
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                waveform = 0;  // Sine
                break;
            case TOGGLESWITCH_MIDDLE:
                waveform = 1;  // Triangle
                break;
            case TOGGLESWITCH_DOWN:
                waveform = 2;  // Square
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        // Pulse LED with LFO rate
        return (getLFO() + 1.0f) * 0.5f;
    }

    float process(float inputSample) override {
        // Get smoothed parameter values
        float rate = smoothRate.process();
        float depth = smoothDepth.process();
        float mix = smoothMix.process();

        // Update LFO phase
        lfoPhase += rate / sampleRate;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Calculate modulated delay time
        float lfoValue = getLFO();
        float delayTime = 10.0f + depth * 15.0f;  // 10-25ms delay range
        float modulatedDelay = delayTime + lfoValue * depth * 5.0f;
        int delaySamples = (int)(modulatedDelay * sampleRate / 1000.0f);

        // Constrain to buffer size
        if (delaySamples >= MAX_CHORUS_DELAY) delaySamples = MAX_CHORUS_DELAY - 1;
        if (delaySamples < 1) delaySamples = 1;

        // Calculate read position
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
