/**
 * Delay Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Digital delay with feedback control
 *
 * Hardware Control Mapping:
 *   KNOB_1: Time (delay time 50ms-1000ms)
 *   KNOB_2: Feedback (0-90%)
 *   KNOB_3: Filter (high-cut on feedback path)
 *   KNOB_4: Level (output level)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend)
 *   TOGGLESWITCH_1: Time mode (UP=short, MIDDLE=medium, DOWN=long)
 */

#include "hothouse.h"

#define MAX_DELAY_SAMPLES 48000  // 1 second at 48kHz

class Delay : public HothouseEffect {
private:
    float delayBuffer[MAX_DELAY_SAMPLES];
    int writeIndex;
    int sampleRate;

    // Smoothed parameters
    ParameterSmoother smoothTime;
    ParameterSmoother smoothFeedback;
    ParameterSmoother smoothFilter;
    ParameterSmoother smoothLevel;
    ParameterSmoother smoothMix;

    // Filter state for feedback path
    float filterState;

    // Time multiplier based on switch position
    float timeMultiplier;

public:
    Delay(int sr = 48000)
        : sampleRate(sr),
          smoothTime(20.0f, (float)sr, 0.5f),
          smoothFeedback(20.0f, (float)sr, 0.5f),
          smoothFilter(20.0f, (float)sr, 0.7f),
          smoothLevel(20.0f, (float)sr, 1.0f),
          smoothMix(20.0f, (float)sr, 0.5f) {
        writeIndex = 0;
        filterState = 0.0f;
        timeMultiplier = 1.0f;

        for (int i = 0; i < MAX_DELAY_SAMPLES; i++) {
            delayBuffer[i] = 0.0f;
        }
    }

    void updateFromControls(const HothouseControls& controls) override {
        // TOGGLESWITCH_1: Time mode affects range
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                timeMultiplier = 0.25f;  // Short: 12.5-250ms
                break;
            case TOGGLESWITCH_MIDDLE:
                timeMultiplier = 0.5f;   // Medium: 25-500ms
                break;
            case TOGGLESWITCH_DOWN:
                timeMultiplier = 1.0f;   // Long: 50-1000ms
                break;
            default:
                break;
        }

        // KNOB_1: Time (scaled by mode)
        float time = controls.knobs[KNOB_1] * timeMultiplier;
        smoothTime.setTarget(time);

        // KNOB_2: Feedback (0 to 0.9)
        smoothFeedback.setTarget(controls.knobs[KNOB_2] * 0.9f);

        // KNOB_3: Filter (high-cut frequency)
        smoothFilter.setTarget(controls.knobs[KNOB_3]);

        // KNOB_4: Level
        smoothLevel.setTarget(controls.knobs[KNOB_4]);

        // KNOB_6: Mix
        smoothMix.setTarget(controls.knobs[KNOB_6]);
    }

    float getLedState() override {
        return 1.0f;
    }

    float process(float inputSample) override {
        // Get smoothed parameter values
        float time = smoothTime.process();
        float feedback = smoothFeedback.process();
        float filter = smoothFilter.process();
        float level = smoothLevel.process();
        float mix = smoothMix.process();

        // Calculate delay in samples (50ms to 1000ms range)
        int delaySamples = (int)((0.05f + time * 0.95f) * sampleRate);
        if (delaySamples < 1) delaySamples = 1;
        if (delaySamples >= MAX_DELAY_SAMPLES) delaySamples = MAX_DELAY_SAMPLES - 1;

        // Calculate read index
        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0) readIndex += MAX_DELAY_SAMPLES;

        // Read delayed sample
        float delayedSample = delayBuffer[readIndex];

        // Apply high-cut filter to feedback (one-pole lowpass)
        float filterCoeff = 0.1f + filter * 0.89f;
        filterState = filterState * (1.0f - filterCoeff) + delayedSample * filterCoeff;
        float filteredFeedback = filterState;

        // Write to buffer with feedback
        delayBuffer[writeIndex] = inputSample + (filteredFeedback * feedback);

        // Clip to prevent runaway
        if (delayBuffer[writeIndex] > 1.0f) delayBuffer[writeIndex] = 1.0f;
        if (delayBuffer[writeIndex] < -1.0f) delayBuffer[writeIndex] = -1.0f;

        // Increment write index
        writeIndex++;
        if (writeIndex >= MAX_DELAY_SAMPLES) writeIndex = 0;

        // Mix dry and wet signals with level control
        float wetSignal = delayedSample * level;
        return inputSample * (1.0f - mix) + wetSignal * mix;
    }

    void reset() override {
        writeIndex = 0;
        filterState = 0.0f;
        for (int i = 0; i < MAX_DELAY_SAMPLES; i++) {
            delayBuffer[i] = 0.0f;
        }
    }
};
