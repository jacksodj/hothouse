/**
 * Cleveland Sound Hothouse Pedal
 * Example Deployment Code
 *
 * This example shows how to deploy effects to the Hothouse pedal
 * with proper hardware control integration matching the official API.
 *
 * Hardware Controls:
 *   - 6 potentiometers (KNOB_1-6): Read via hw.knobs[].Process(), returns 0.0-1.0
 *   - 3 toggle switches (TOGGLESWITCH_1-3): Read via hw.GetToggleswitchPosition()
 *   - 2 footswitches (FOOTSWITCH_1-2): Read via hw.switches[].RisingEdge()
 *   - 2 LEDs (LED_1-2): Write via Led.Set() and Led.Update() in main loop
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

/**
 * Hardware abstraction layer - replace with actual Hothouse hardware reads
 * In a real implementation using libDaisy, these would use hw.knobs[].Process(),
 * hw.GetToggleswitchPosition(), hw.switches[].RisingEdge(), etc.
 */
namespace Hardware {
    // Simulated knob state (replace with actual ADC reads)
    float knobValues[KNOB_COUNT] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};

    // Simulated switch state (replace with actual GPIO reads)
    ToggleswitchPosition switchPositions[TOGGLESWITCH_COUNT] = {
        TOGGLESWITCH_MIDDLE, TOGGLESWITCH_MIDDLE, TOGGLESWITCH_MIDDLE
    };

    // Simulated footswitch state
    bool footswitchPressed[FOOTSWITCH_COUNT] = {false, false};
    bool lastFootswitchState[FOOTSWITCH_COUNT] = {false, false};

    // Read knob value (0.0 to 1.0)
    // In real implementation: return hw.knobs[knob].Process();
    float readKnob(Knob knob) {
        return knobValues[knob];
    }

    // Read toggle switch position
    // In real implementation: return hw.GetToggleswitchPosition(sw);
    ToggleswitchPosition readToggleswitch(Toggleswitch sw) {
        return switchPositions[sw];
    }

    // Detect footswitch rising edge (press)
    // In real implementation: return hw.switches[FOOTSWITCH_1 + fs].RisingEdge();
    bool readFootswitchRisingEdge(Footswitch fs) {
        bool current = footswitchPressed[fs];
        bool rising = current && !lastFootswitchState[fs];
        lastFootswitchState[fs] = current;
        return rising;
    }

    // Write LED brightness (call in main loop, not audio callback)
    // In real implementation:
    //   led.Set(brightness);
    //   led.Update();
    void writeLed(Led led, float brightness) {
        // TODO: Replace with actual LED PWM write
        (void)led;
        (void)brightness;
    }

    // Read all hardware controls into a HothouseControls struct
    HothouseControls readAllControls() {
        HothouseControls controls;

        // Read knobs
        for (int i = 0; i < KNOB_COUNT; i++) {
            controls.knobs[i] = readKnob((Knob)i);
        }

        // Read toggle switches
        for (int i = 0; i < TOGGLESWITCH_COUNT; i++) {
            controls.toggles[i] = readToggleswitch((Toggleswitch)i);
        }

        // Read footswitch rising edges
        for (int i = 0; i < FOOTSWITCH_COUNT; i++) {
            controls.footswitchRisingEdge[i] = readFootswitchRisingEdge((Footswitch)i);
            controls.footswitchPressed[i] = footswitchPressed[i];
        }

        return controls;
    }

    // Write LED states to hardware (call from main loop)
    void writeLeds(const HothouseLeds& leds) {
        writeLed(LED_1, leds.brightness[LED_1]);
        writeLed(LED_2, leds.brightness[LED_2]);
    }
}

/**
 * Audio callback - called by the audio system for each buffer
 * In real implementation, this has signature:
 *   void AudioCallback(AudioHandle::InputBuffer in,
 *                      AudioHandle::OutputBuffer out, size_t size)
 */
void audioCallback(float* inputBuffer, float* outputBuffer, int numSamples,
                   HothousePedal& pedal) {
    // Read hardware controls (call hw.ProcessAllControls() in real implementation)
    HothouseControls controls = Hardware::readAllControls();

    // Update pedal with current control values
    pedal.updateControls(controls);

    // Process audio buffer
    pedal.processBuffer(inputBuffer, outputBuffer, numSamples);

    // Note: LED updates should happen in main loop, not here
}

/**
 * Main deployment code
 */
int main() {
    // Configure hardware
    HothouseConfig config;
    config.sampleRate = 48000;
    config.bufferSize = 4;  // Hothouse uses 4-sample blocks for low latency

    // Create pedal controller
    HothousePedal pedal(config);

    // Select which effect to use
    // Uncomment the effect you want to deploy:

    Overdrive overdrive(config.sampleRate);
    pedal.setEffect(&overdrive);

    // Delay delay(config.sampleRate);
    // pedal.setEffect(&delay);

    // Reverb reverb(config.sampleRate);
    // pedal.setEffect(&reverb);

    // Chorus chorus(config.sampleRate);
    // pedal.setEffect(&chorus);

    // Distortion distortion(config.sampleRate);
    // pedal.setEffect(&distortion);

    // Fuzz fuzz(config.sampleRate);
    // pedal.setEffect(&fuzz);

    // Tremolo tremolo(config.sampleRate);
    // pedal.setEffect(&tremolo);

    // Compressor compressor(config.sampleRate);
    // pedal.setEffect(&compressor);

    // Audio buffers
    float inputBuffer[4];
    float outputBuffer[4];

    // Main loop
    // In real implementation, audio processing is handled by DMA/interrupts
    // and this loop just handles LED updates
    while (true) {
        // TODO: In real implementation, audio callback runs via DMA interrupt
        // For now, simulate it:
        for (int i = 0; i < config.bufferSize; i++) {
            inputBuffer[i] = 0.0f;  // Replace with ADC read
        }
        audioCallback(inputBuffer, outputBuffer, config.bufferSize, pedal);

        // Update LEDs in main loop (not audio callback)
        // In real implementation:
        //   led_bypass.Set(pedal.isBypassed() ? 0.0f : 1.0f);
        //   led_bypass.Update();
        Hardware::writeLeds(pedal.getLeds());

        // Small delay to prevent excessive LED updates (~1kHz is fine)
        // In real implementation: System::Delay(1);
    }

    return 0;
}

/**
 * Effect Selection Guide
 *
 * Each effect uses the hardware controls as follows:
 *
 * OVERDRIVE:
 *   KNOB_1=Drive, KNOB_2=Tone, KNOB_3=Bass, KNOB_4=Level, KNOB_6=Mix
 *   TOGGLESWITCH_1: Voicing (UP=warm, MIDDLE=neutral, DOWN=bright)
 *
 * DISTORTION:
 *   KNOB_1=Gain, KNOB_2=Tone, KNOB_3=Bass, KNOB_4=Level, KNOB_6=Mix
 *   TOGGLESWITCH_1: Clipping (UP=hard, MIDDLE=medium, DOWN=soft)
 *
 * FUZZ:
 *   KNOB_1=Fuzz, KNOB_2=Tone, KNOB_3=Gate, KNOB_4=Level, KNOB_6=Mix
 *   TOGGLESWITCH_1: Character (UP=vintage, MIDDLE=modern, DOWN=octave)
 *
 * DELAY:
 *   KNOB_1=Time, KNOB_2=Feedback, KNOB_3=Filter, KNOB_4=Level, KNOB_6=Mix
 *   TOGGLESWITCH_1: Time mode (UP=short, MIDDLE=medium, DOWN=long)
 *
 * REVERB:
 *   KNOB_1=Size, KNOB_2=Damping, KNOB_3=Pre-delay, KNOB_4=Level, KNOB_6=Mix
 *   TOGGLESWITCH_1: Room type (UP=small, MIDDLE=medium, DOWN=hall)
 *
 * CHORUS:
 *   KNOB_1=Rate, KNOB_2=Depth, KNOB_6=Mix
 *   TOGGLESWITCH_1: Waveform (UP=sine, MIDDLE=triangle, DOWN=square)
 *
 * TREMOLO:
 *   KNOB_1=Rate, KNOB_2=Depth, KNOB_3=Shape, KNOB_4=Level, KNOB_6=Mix
 *   TOGGLESWITCH_1: Mode (UP=classic, MIDDLE=harmonic, DOWN=opto)
 *
 * COMPRESSOR:
 *   KNOB_1=Threshold, KNOB_2=Ratio, KNOB_3=Attack, KNOB_4=Release,
 *   KNOB_5=Makeup, KNOB_6=Mix
 *   TOGGLESWITCH_1: Knee mode (UP=hard, MIDDLE=medium, DOWN=soft)
 *
 * Common to all effects:
 *   - FOOTSWITCH_1: Toggles effect bypass (LED_1 off when bypassed)
 *   - LED_1: Shows effect state (on/off, or effect-specific feedback)
 */
