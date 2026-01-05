/**
 * Tremolo Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Amplitude modulation effect
 *
 * Hardware Control Mapping:
 *   KNOB_1: Rate (LFO speed 0.5-20 Hz)
 *   KNOB_2: Depth (modulation depth)
 *   KNOB_3: Shape (LFO waveform morph)
 *   KNOB_4: Level (output volume)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend)
 *   TOGGLESWITCH_1: Mode (UP=classic, MIDDLE=harmonic, DOWN=opto)
 */

#include "hothouse.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class Tremolo : public HothouseEffect {
private:
    // Smoothed parameters
    ParameterSmoother smoothRate;
    ParameterSmoother smoothDepth;
    ParameterSmoother smoothShape;
    ParameterSmoother smoothLevel;
    ParameterSmoother smoothMix;

    float phase;
    float sampleRate;

    // Mode (0=classic, 1=harmonic, 2=opto)
    int mode;

    // Opto mode smoothing state
    float optoState;

    // Get LFO value based on shape parameter
    float getLFO(float shape) {
        // Morph between sine (0), triangle (0.5), and square (1)
        float sine = sinf(2.0f * M_PI * phase);
        float triangle = 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
        float square = phase < 0.5f ? 1.0f : -1.0f;

        if (shape < 0.5f) {
            // Morph sine to triangle
            float t = shape * 2.0f;
            return sine * (1.0f - t) + triangle * t;
        } else {
            // Morph triangle to square
            float t = (shape - 0.5f) * 2.0f;
            return triangle * (1.0f - t) + square * t;
        }
    }

public:
    Tremolo(int sr = 48000)
        : sampleRate((float)sr),
          smoothRate(20.0f, (float)sr, 0.3f),
          smoothDepth(20.0f, (float)sr, 0.5f),
          smoothShape(20.0f, (float)sr, 0.0f),
          smoothLevel(20.0f, (float)sr, 1.0f),
          smoothMix(20.0f, (float)sr, 1.0f) {
        phase = 0.0f;
        mode = 0;
        optoState = 1.0f;
    }

    void updateFromControls(const HothouseControls& controls) override {
        // KNOB_1: Rate (0.5 to 20 Hz)
        smoothRate.setTarget(0.5f + controls.knobs[KNOB_1] * 19.5f);

        // KNOB_2: Depth
        smoothDepth.setTarget(controls.knobs[KNOB_2]);

        // KNOB_3: Shape
        smoothShape.setTarget(controls.knobs[KNOB_3]);

        // KNOB_4: Level
        smoothLevel.setTarget(controls.knobs[KNOB_4]);

        // KNOB_6: Mix
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Mode
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                mode = 0;  // Classic
                break;
            case TOGGLESWITCH_MIDDLE:
                mode = 1;  // Harmonic
                break;
            case TOGGLESWITCH_DOWN:
                mode = 2;  // Opto
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        // Pulse LED with tremolo rate
        float lfo = sinf(2.0f * M_PI * phase);
        return (lfo + 1.0f) * 0.5f;
    }

    float process(float inputSample) override {
        float rate = smoothRate.process();
        float depth = smoothDepth.process();
        float shape = smoothShape.process();
        float level = smoothLevel.process();
        float mix = smoothMix.process();

        // Get LFO value (-1 to 1)
        float lfo = getLFO(shape);

        // Calculate amplitude modulation based on mode
        float amplitude;

        switch (mode) {
            case 0:  // Classic - symmetric modulation
                amplitude = 1.0f - (depth * 0.5f * (1.0f + lfo));
                break;

            case 1:  // Harmonic - only attenuates, never boosts
                amplitude = 1.0f - (depth * (lfo + 1.0f) * 0.5f);
                break;

            default: // Opto - asymmetric response with smoothing
            {
                float target = 1.0f - (depth * (lfo + 1.0f) * 0.5f);
                // Asymmetric smoothing: fast attack, slow release
                float coeff = target < optoState ? 0.99f : 0.995f;
                optoState = optoState * coeff + target * (1.0f - coeff);
                amplitude = optoState;
                break;
            }
        }

        // Constrain amplitude
        if (amplitude < 0.0f) amplitude = 0.0f;
        if (amplitude > 1.0f) amplitude = 1.0f;

        // Update phase
        phase += rate / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;

        // Apply amplitude modulation, mix, and level
        float modulated = inputSample * amplitude;
        float output = inputSample * (1.0f - mix) + modulated * mix;
        return output * level;
    }

    void reset() override {
        phase = 0.0f;
        optoState = 1.0f;
    }
};
