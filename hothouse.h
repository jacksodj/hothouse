/**
 * Cleveland Sound Hothouse Pedal
 * Base Effect Interface and Utilities
 *
 * Common header file for all effect implementations
 *
 * Hardware Controls (matching official Hothouse API):
 *   - 6 potentiometers (KNOB_1-6): 0.0-1.0
 *   - 3 toggle switches (TOGGLESWITCH_1-3): UP/MIDDLE/DOWN enum
 *   - 2 footswitches (FOOTSWITCH_1-2): RisingEdge() detection
 *   - 2 LEDs (LED_1-2): 0.0-1.0 PWM (update in main loop, not audio callback)
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
 * Toggle switch position enum (ON-OFF-ON switches)
 * Matches the official Hothouse API
 */
enum ToggleswitchPosition {
    TOGGLESWITCH_UP,      // Switch pushed up (position 1)
    TOGGLESWITCH_MIDDLE,  // Switch in center/OFF position
    TOGGLESWITCH_DOWN,    // Switch pushed down (position 2)
    TOGGLESWITCH_UNKNOWN  // For ON-ON switches (no middle position)
};

/**
 * Knob identifiers
 */
enum Knob {
    KNOB_1 = 0,
    KNOB_2,
    KNOB_3,
    KNOB_4,
    KNOB_5,
    KNOB_6,
    KNOB_COUNT
};

/**
 * Toggle switch identifiers
 */
enum Toggleswitch {
    TOGGLESWITCH_1 = 0,
    TOGGLESWITCH_2,
    TOGGLESWITCH_3,
    TOGGLESWITCH_COUNT
};

/**
 * Footswitch identifiers
 */
enum Footswitch {
    FOOTSWITCH_1 = 0,
    FOOTSWITCH_2,
    FOOTSWITCH_COUNT
};

/**
 * LED identifiers
 */
enum Led {
    LED_1 = 0,
    LED_2,
    LED_COUNT
};

/**
 * Hardware control inputs from the Hothouse pedal
 */
struct HothouseControls {
    // Potentiometers (0.0 to 1.0)
    float knobs[KNOB_COUNT];

    // Toggle switches (enum positions)
    ToggleswitchPosition toggles[TOGGLESWITCH_COUNT];

    // Footswitch rising edge detection (true on press)
    bool footswitchRisingEdge[FOOTSWITCH_COUNT];

    // Footswitch current state (for level reading if needed)
    bool footswitchPressed[FOOTSWITCH_COUNT];

    HothouseControls() {
        for (int i = 0; i < KNOB_COUNT; i++) {
            knobs[i] = 0.5f;
        }
        for (int i = 0; i < TOGGLESWITCH_COUNT; i++) {
            toggles[i] = TOGGLESWITCH_MIDDLE;
        }
        for (int i = 0; i < FOOTSWITCH_COUNT; i++) {
            footswitchRisingEdge[i] = false;
            footswitchPressed[i] = false;
        }
    }
};

/**
 * Parameter smoother to prevent zipper noise when knobs are turned
 * Uses one-pole lowpass filter for smooth parameter transitions
 */
class ParameterSmoother {
private:
    float currentValue;
    float targetValue;
    float coefficient;

public:
    /**
     * @param smoothingMs Smoothing time in milliseconds
     * @param sampleRate Audio sample rate
     * @param initialValue Starting value
     */
    ParameterSmoother(float smoothingMs = 20.0f, float sampleRate = 48000.0f, float initialValue = 0.5f) {
        currentValue = initialValue;
        targetValue = initialValue;
        setSmoothing(smoothingMs, sampleRate);
    }

    void setSmoothing(float smoothingMs, float sampleRate) {
        float samples = (smoothingMs / 1000.0f) * sampleRate;
        if (samples < 1.0f) samples = 1.0f;
        coefficient = 1.0f - (1.0f / samples);
    }

    void setTarget(float value) {
        targetValue = value;
    }

    void setImmediate(float value) {
        currentValue = value;
        targetValue = value;
    }

    float process() {
        currentValue = currentValue * coefficient + targetValue * (1.0f - coefficient);
        return currentValue;
    }

    float getValue() const {
        return currentValue;
    }
};

/**
 * Base class for all effect pedal implementations
 * All effects must inherit from this class and implement the required methods
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

    /**
     * Update effect parameters from hardware controls
     * Called before processing each audio buffer
     * @param controls Current state of all hardware controls
     */
    virtual void updateFromControls(const HothouseControls& controls) = 0;

    /**
     * Get the current LED state for visual feedback
     * Note: LEDs should be updated in the main loop, not audio callback
     * @return LED brightness (0.0 = off, 1.0 = full brightness)
     */
    virtual float getLedState() { return 1.0f; }
};

/**
 * Configuration for Cleveland Sound Hothouse hardware
 */
struct HothouseConfig {
    int sampleRate;          // Audio sample rate (default: 48000 Hz)
    int bufferSize;          // Audio buffer size (default: 4 samples for low latency)
    int adcResolution;       // ADC resolution in bits (default: 24)
    int dacResolution;       // DAC resolution in bits (default: 24)

    HothouseConfig() {
        sampleRate = 48000;
        bufferSize = 4;      // Hothouse uses 4-sample blocks
        adcResolution = 24;
        dacResolution = 24;
    }
};

/**
 * LED output state
 * Note: Update LEDs in main loop at ~1kHz, not in audio callback
 */
struct HothouseLeds {
    float brightness[LED_COUNT];

    HothouseLeds() {
        for (int i = 0; i < LED_COUNT; i++) {
            brightness[i] = 0.0f;
        }
    }

    void set(Led led, float value) {
        if (led < LED_COUNT) {
            brightness[led] = constrain(value, 0.0f, 1.0f);
        }
    }

    float get(Led led) const {
        if (led < LED_COUNT) {
            return brightness[led];
        }
        return 0.0f;
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
    HothouseControls controls;
    HothouseLeds leds;
    bool bypassed;

public:
    HothousePedal(HothouseConfig cfg = HothouseConfig())
        : currentEffect(nullptr), config(cfg), bypassed(false) {}

    void setEffect(HothouseEffect* effect) {
        currentEffect = effect;
    }

    /**
     * Update hardware controls - call this at start of audio callback
     * In actual implementation, this calls hw.ProcessAllControls()
     * @param newControls Current state of hardware controls
     */
    void updateControls(const HothouseControls& newControls) {
        controls = newControls;

        // Handle bypass footswitch (toggle on rising edge)
        if (controls.footswitchRisingEdge[FOOTSWITCH_1]) {
            bypassed = !bypassed;
        }

        // Update effect parameters
        if (currentEffect != nullptr) {
            currentEffect->updateFromControls(controls);
        }

        // Update LED states
        if (bypassed) {
            leds.set(LED_1, 0.0f);  // LED off when bypassed
        } else if (currentEffect != nullptr) {
            leds.set(LED_1, currentEffect->getLedState());
        } else {
            leds.set(LED_1, 0.0f);
        }
    }

    void bypass(bool enable) {
        bypassed = enable;
    }

    bool isBypassed() const {
        return bypassed;
    }

    /**
     * Get current LED states for hardware output
     * Call this from main loop, not audio callback
     */
    const HothouseLeds& getLeds() const {
        return leds;
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

    const HothouseControls& getControls() const {
        return controls;
    }
};

#endif // HOTHOUSE_H
