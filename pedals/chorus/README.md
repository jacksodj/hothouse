# Chorus Effect Pedal

## Description
Chorus effect that creates a rich, shimmering sound by modulating a short delay with an LFO (Low Frequency Oscillator).

## Parameters
- **Rate** (0.1-10.0 Hz): Speed of the LFO modulation
- **Depth** (0.0-1.0): Amount of delay time modulation
- **Mix** (0.0-1.0): Balance between dry and wet signal

## Usage
```cpp
Chorus chorus(48000);
chorus.setRate(1.5f);   // 1.5 Hz LFO rate
chorus.setDepth(0.6f);  // Moderate depth
chorus.setMix(0.5f);    // 50/50 mix

float output = chorus.process(inputSample);
```

## Implementation Notes
- Uses time-varying delay line modulated by triangle wave LFO
- Delay range: 10-25ms (typical chorus range)
- Memory requirement: ~19KB for delay buffer
- Creates the classic "doubling" effect heard on many recordings
