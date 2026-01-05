# Delay Effect Pedal

## Description
Digital delay effect with adjustable delay time, feedback, and wet/dry mix.

## Parameters
- **Time** (0.0-1.0): Delay time from 0 to 1 second
- **Feedback** (0.0-0.95): Amount of delayed signal fed back into the delay line
- **Mix** (0.0-1.0): Balance between dry (original) and wet (delayed) signal

## Usage
```cpp
Delay delay(48000);  // 48kHz sample rate
delay.setTime(0.5f);      // 500ms delay
delay.setFeedback(0.4f);  // Moderate feedback
delay.setMix(0.5f);       // 50/50 mix

float output = delay.process(inputSample);
```

## Implementation Notes
- Uses circular buffer for delay line (1 second maximum)
- Feedback is limited to 0.95 to prevent runaway oscillation
- Sample rate configurable (default 48kHz)
- Memory requirement: ~192KB for delay buffer
