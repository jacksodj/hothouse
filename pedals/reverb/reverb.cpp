/**
 * Reverb Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Simple reverb using Schroeder reverberator algorithm
 *
 * Hardware Control Mapping:
 *   KNOB_1: Size (room size / decay time)
 *   KNOB_2: Damping (high frequency damping)
 *   KNOB_3: Pre-delay (initial delay before reverb)
 *   KNOB_4: Level (reverb level)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend)
 *   TOGGLESWITCH_1: Room type (UP=small, MIDDLE=medium, DOWN=hall)
 */

#include "hothouse.h"

#define NUM_COMB_FILTERS 4
#define NUM_ALLPASS_FILTERS 2
#define MAX_PREDELAY 4800  // 100ms at 48kHz

// Base comb filter delay times (in samples at 48kHz)
const int baseCombDelays[NUM_COMB_FILTERS] = {1557, 1617, 1491, 1422};
const int baseAllpassDelays[NUM_ALLPASS_FILTERS] = {225, 556};

class CombFilter {
private:
    float* buffer;
    int bufferSize;
    int index;
    float feedback;
    float damping;
    float dampState;

public:
    CombFilter(int size) : bufferSize(size), index(0), feedback(0.7f), damping(0.5f), dampState(0.0f) {
        buffer = new float[bufferSize];
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
    }

    ~CombFilter() { delete[] buffer; }

    void setFeedback(float fb) { feedback = fb; }
    void setDamping(float d) { damping = d; }

    float process(float input) {
        float output = buffer[index];

        // Apply damping (lowpass in feedback path)
        dampState = output * (1.0f - damping) + dampState * damping;

        buffer[index] = input + dampState * feedback;
        index = (index + 1) % bufferSize;
        return output;
    }

    void clear() {
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
        index = 0;
        dampState = 0.0f;
    }
};

class AllpassFilter {
private:
    float* buffer;
    int bufferSize;
    int index;
    float gain;

public:
    AllpassFilter(int size) : bufferSize(size), index(0), gain(0.5f) {
        buffer = new float[bufferSize];
        for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0f;
    }

    ~AllpassFilter() { delete[] buffer; }

    float process(float input) {
        float bufOut = buffer[index];
        float output = -input + bufOut;
        buffer[index] = input + bufOut * gain;
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

    // Pre-delay buffer
    float predelayBuffer[MAX_PREDELAY];
    int predelayWriteIndex;

    // Smoothed parameters
    ParameterSmoother smoothSize;
    ParameterSmoother smoothDamping;
    ParameterSmoother smoothPredelay;
    ParameterSmoother smoothLevel;
    ParameterSmoother smoothMix;

    // Room type (affects feedback and delay scaling)
    int roomType;
    float sizeMultiplier;

public:
    Reverb(int sampleRate = 48000)
        : smoothSize(20.0f, (float)sampleRate, 0.5f),
          smoothDamping(20.0f, (float)sampleRate, 0.5f),
          smoothPredelay(20.0f, (float)sampleRate, 0.0f),
          smoothLevel(20.0f, (float)sampleRate, 1.0f),
          smoothMix(20.0f, (float)sampleRate, 0.3f) {
        roomType = 1;
        sizeMultiplier = 1.0f;
        predelayWriteIndex = 0;

        // Initialize pre-delay buffer
        for (int i = 0; i < MAX_PREDELAY; i++) {
            predelayBuffer[i] = 0.0f;
        }

        // Initialize filters
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combFilters[i] = new CombFilter(baseCombDelays[i]);
        }

        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) {
            allpassFilters[i] = new AllpassFilter(baseAllpassDelays[i]);
        }
    }

    ~Reverb() {
        for (int i = 0; i < NUM_COMB_FILTERS; i++) delete combFilters[i];
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) delete allpassFilters[i];
    }

    void updateFromControls(const HothouseControls& controls) override {
        smoothSize.setTarget(controls.knobs[KNOB_1]);
        smoothDamping.setTarget(controls.knobs[KNOB_2]);
        smoothPredelay.setTarget(controls.knobs[KNOB_3]);
        smoothLevel.setTarget(controls.knobs[KNOB_4]);
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Room type
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                roomType = 0;
                sizeMultiplier = 0.5f;   // Small room
                break;
            case TOGGLESWITCH_MIDDLE:
                roomType = 1;
                sizeMultiplier = 1.0f;   // Medium room
                break;
            case TOGGLESWITCH_DOWN:
                roomType = 2;
                sizeMultiplier = 1.5f;   // Hall
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        return 1.0f;
    }

    float process(float inputSample) override {
        float size = smoothSize.process();
        float damping = smoothDamping.process();
        float predelay = smoothPredelay.process();
        float level = smoothLevel.process();
        float mix = smoothMix.process();

        // Calculate feedback based on size
        float feedback = 0.5f + size * sizeMultiplier * 0.35f;
        if (feedback > 0.95f) feedback = 0.95f;

        // Update comb filter parameters
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combFilters[i]->setFeedback(feedback);
            combFilters[i]->setDamping(damping);
        }

        // Pre-delay
        int predelaySamples = (int)(predelay * MAX_PREDELAY);
        if (predelaySamples < 1) predelaySamples = 1;
        if (predelaySamples >= MAX_PREDELAY) predelaySamples = MAX_PREDELAY - 1;

        predelayBuffer[predelayWriteIndex] = inputSample;
        int predelayReadIndex = predelayWriteIndex - predelaySamples;
        if (predelayReadIndex < 0) predelayReadIndex += MAX_PREDELAY;
        float predelayedSample = predelayBuffer[predelayReadIndex];

        predelayWriteIndex++;
        if (predelayWriteIndex >= MAX_PREDELAY) predelayWriteIndex = 0;

        // Parallel comb filters
        float combOut = 0.0f;
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combOut += combFilters[i]->process(predelayedSample);
        }
        combOut /= NUM_COMB_FILTERS;

        // Series allpass filters
        float output = combOut;
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) {
            output = allpassFilters[i]->process(output);
        }

        // Apply level and mix dry/wet
        float wetSignal = output * level;
        return inputSample * (1.0f - mix) + wetSignal * mix;
    }

    void reset() override {
        for (int i = 0; i < NUM_COMB_FILTERS; i++) {
            combFilters[i]->clear();
        }
        for (int i = 0; i < NUM_ALLPASS_FILTERS; i++) {
            allpassFilters[i]->clear();
        }
        for (int i = 0; i < MAX_PREDELAY; i++) {
            predelayBuffer[i] = 0.0f;
        }
        predelayWriteIndex = 0;
    }
};
