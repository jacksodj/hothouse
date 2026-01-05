/**
 * Overdrive Effect Pedal
 * Cleveland Sound Hothouse Implementation
 *
 * Classic tube-style overdrive with soft clipping
 *
 * Hardware Control Mapping:
 *   KNOB_1: Drive (overdrive amount)
 *   KNOB_2: Tone (high-cut filter)
 *   KNOB_3: Bass (low-end control)
 *   KNOB_4: Level (output volume)
 *   KNOB_5: (unused)
 *   KNOB_6: Mix (dry/wet blend)
 *   TOGGLESWITCH_1: Voicing (UP=warm, MIDDLE=neutral, DOWN=bright)
 */

#include "hothouse.h"

class Overdrive : public HothouseEffect {
private:
    // Smoothed parameters
    ParameterSmoother smoothDrive;
    ParameterSmoother smoothTone;
    ParameterSmoother smoothBass;
    ParameterSmoother smoothLevel;
    ParameterSmoother smoothMix;

    float previousSample;
    float bassState;

    // Voicing mode (0=warm, 1=neutral, 2=bright)
    int voicing;

    // Soft clipping function (tanh approximation)
    float softClip(float sample, float drive) {
        float x = sample * (1.0f + drive * 9.0f);
        // Fast tanh approximation
        if (x > 1.0f) return 0.76159f;
        if (x < -1.0f) return -0.76159f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Simple one-pole low-pass filter for tone control
    float toneLowPass(float sample, float alpha) {
        float filtered = alpha * sample + (1.0f - alpha) * previousSample;
        previousSample = filtered;
        return filtered;
    }

public:
    Overdrive(int sampleRate = 48000)
        : smoothDrive(20.0f, (float)sampleRate, 0.5f),
          smoothTone(20.0f, (float)sampleRate, 0.7f),
          smoothBass(20.0f, (float)sampleRate, 0.5f),
          smoothLevel(20.0f, (float)sampleRate, 0.8f),
          smoothMix(20.0f, (float)sampleRate, 1.0f) {
        previousSample = 0.0f;
        bassState = 0.0f;
        voicing = 1;
    }

    void updateFromControls(const HothouseControls& controls) override {
        smoothDrive.setTarget(controls.knobs[KNOB_1]);
        smoothTone.setTarget(controls.knobs[KNOB_2]);
        smoothBass.setTarget(controls.knobs[KNOB_3]);
        smoothLevel.setTarget(controls.knobs[KNOB_4]);
        smoothMix.setTarget(controls.knobs[KNOB_6]);

        // TOGGLESWITCH_1: Voicing
        switch (controls.toggles[TOGGLESWITCH_1]) {
            case TOGGLESWITCH_UP:
                voicing = 0;  // Warm
                break;
            case TOGGLESWITCH_MIDDLE:
                voicing = 1;  // Neutral
                break;
            case TOGGLESWITCH_DOWN:
                voicing = 2;  // Bright
                break;
            default:
                break;
        }
    }

    float getLedState() override {
        return 1.0f;
    }

    float process(float inputSample) override {
        float drive = smoothDrive.process();
        float tone = smoothTone.process();
        float bass = smoothBass.process();
        float level = smoothLevel.process();
        float mix = smoothMix.process();

        // Bass control (low shelf)
        float bassCoeff = 0.05f;
        bassState = bassState * (1.0f - bassCoeff) + inputSample * bassCoeff;
        float bassBoost = (bass - 0.5f) * 2.0f;
        float sample = inputSample + bassState * bassBoost;

        // Apply overdrive with soft clipping
        float driven = softClip(sample, drive);

        // Apply voicing-adjusted tone control
        float toneBase;
        switch (voicing) {
            case 0:  // Warm - more lowpass
                toneBase = 0.3f;
                break;
            case 2:  // Bright - less lowpass
                toneBase = 0.7f;
                break;
            default: // Neutral
                toneBase = 0.5f;
                break;
        }
        float toneAlpha = toneBase + tone * (1.0f - toneBase) * 0.98f;
        float toned = toneLowPass(driven, toneAlpha);

        // Mix and apply output level
        float output = inputSample * (1.0f - mix) + toned * mix;
        return output * level;
    }

    void reset() override {
        previousSample = 0.0f;
        bassState = 0.0f;
    }
};
