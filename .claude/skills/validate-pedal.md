# Validate Pedal Implementation

Validates that a Hothouse DSP pedal implementation correctly integrates with the physical hardware controls based on the official Cleveland Music Co. Hothouse API.

## Usage

```
/validate-pedal [pedal-name]
```

If no pedal name is provided, validate all pedals in the `pedals/` directory.

## Hothouse Hardware Reference

The Cleveland Music Co. Hothouse pedal has these physical controls:

### Inputs (Read from hardware)

| Control | Count | API Method | Return Type |
|---------|-------|------------|-------------|
| Potentiometers | 6 | `hw.knobs[KNOB_1..6].Process()` | float 0.0-1.0 |
| Toggle Switches | 3 | `hw.GetToggleswitchPosition(TOGGLESWITCH_1..3)` | ToggleswitchPosition enum |
| Footswitches | 2 | `hw.switches[FOOTSWITCH_1..2].RisingEdge()` | bool |

### Toggle Switch Positions (ON-OFF-ON)

The toggle switches are 3-position ON-OFF-ON switches. The position is returned as an enum:

```cpp
enum ToggleswitchPosition {
    TOGGLESWITCH_UP,      // Switch pushed up
    TOGGLESWITCH_MIDDLE,  // Switch in center (OFF position)
    TOGGLESWITCH_DOWN,    // Switch pushed down
    TOGGLESWITCH_UNKNOWN  // For ON-ON switches (no middle)
};
```

**Important**: Each physical toggle switch uses two GPIO pins (up and down). The `GetToggleswitchPosition()` method reads both pins to determine the position.

### Outputs (Write to hardware)

| Control | Count | API | Notes |
|---------|-------|-----|-------|
| LEDs | 2 | `Led` class with `.Set()` and `.Update()` | PWM controlled 0.0-1.0 |

### LED Control Pattern

LEDs must be updated in the **main loop**, not the audio callback:

```cpp
// In main loop (not audio callback)
led.Set(brightness);  // 0.0 = off, 1.0 = full brightness
led.Update();         // Actually writes to hardware
```

## Validation Checklist

For each pedal, verify:

### 1. Hardware Control Mapping
- [ ] Each user-adjustable parameter is mapped to a knob (KNOB_1-6)
- [ ] Toggle switches use `ToggleswitchPosition` enum (not float values)
- [ ] Bypass footswitch uses `.RisingEdge()` for toggle detection
- [ ] LED feedback updated in main loop, not audio callback

### 2. Parameter Smoothing
- [ ] Knob values are smoothed to prevent zipper noise/clicks
- [ ] Recommended: 10-20ms smoothing time for parameter changes

### 3. Standard Control Assignments

| Knob | Common Use |
|------|------------|
| KNOB_1 | Primary effect amount (drive, rate, time, etc.) |
| KNOB_2 | Secondary parameter (tone, depth, feedback, etc.) |
| KNOB_3 | Third parameter (blend, filter, etc.) |
| KNOB_4 | Output level |
| KNOB_5 | Additional parameter |
| KNOB_6 | Dry/wet mix |

### 4. Required Interface Methods

Each pedal class should implement:

```cpp
// Called each audio frame to update parameters from hardware
void updateFromControls(const HothouseControls& controls);

// Get LED state for feedback (called from main loop)
float getLedState();
```

## Example Compliant Implementation

```cpp
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out, size_t size) {
    // Process all controls at start of callback
    hw.ProcessAllControls();

    // Read knobs (0.0 to 1.0)
    float rate = hw.knobs[Hothouse::KNOB_1].Process();
    float depth = hw.knobs[Hothouse::KNOB_2].Process();

    // Read toggle switch position (enum value)
    Hothouse::ToggleswitchPosition mode =
        hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);

    switch (mode) {
        case Hothouse::TOGGLESWITCH_UP:
            // Mode A
            break;
        case Hothouse::TOGGLESWITCH_MIDDLE:
            // Mode B (center/off position)
            break;
        case Hothouse::TOGGLESWITCH_DOWN:
            // Mode C
            break;
        default:
            break;
    }

    // Handle bypass footswitch (toggle on rising edge)
    if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
        bypass = !bypass;
    }

    // Process audio
    for (size_t i = 0; i < size; i++) {
        if (bypass) {
            out[0][i] = out[1][i] = in[0][i];
        } else {
            out[0][i] = out[1][i] = effect.Process(in[0][i]);
        }
    }
}

// Main loop - update LEDs here (not in audio callback)
int main() {
    // ... initialization ...

    Led led_bypass;
    led_bypass.Init(hw.seed.GetPin(23), false);

    while (true) {
        // Update LED state
        led_bypass.Set(bypass ? 0.0f : 1.0f);
        led_bypass.Update();

        // Small delay to prevent excessive LED updates
        System::Delay(1);
    }
}
```

## Switch Index Reference

| Enum | Index | Physical Control |
|------|-------|------------------|
| TOGGLESWITCH_1 | 0-1 | Toggle switch 1 (pins D9/D10) |
| TOGGLESWITCH_2 | 2-3 | Toggle switch 2 (pins D7/D8) |
| TOGGLESWITCH_3 | 4-5 | Toggle switch 3 (pins D5/D6) |
| FOOTSWITCH_1 | 6 | Left footswitch (pin D25) |
| FOOTSWITCH_2 | 7 | Right footswitch (pin D26) |

## LED Pin Reference

| LED | Pin | Enum |
|-----|-----|------|
| LED_1 | 22 | Hothouse::LED_1 |
| LED_2 | 23 | Hothouse::LED_2 |

## Validation Output

When validating, report:
1. **PASS** - Pedal correctly implements hardware controls
2. **FAIL** - List specific issues:
   - Using float values for switch positions instead of enum
   - LED updates in audio callback instead of main loop
   - Missing parameter smoothing
   - Incorrect knob/switch mapping
3. **WARN** - Suggestions for improvement

## Common Mistakes

1. **Using float for switch positions**: Toggle switches return enum values, not 0.0/0.5/1.0
2. **LED in audio callback**: LEDs should be updated in main loop at ~1kHz, not at audio rate
3. **Missing .RisingEdge()**: Footswitches need edge detection to toggle, not level reading
4. **No ProcessAllControls()**: Must call at start of audio callback to update control states
