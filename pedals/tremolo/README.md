# Tremolo Effect Pedal

## Description
Classic tremolo effect that modulates the amplitude of the signal with an LFO, creating a pulsing, rhythmic effect.

## Parameters
- **Rate** (0.5-20.0 Hz): Speed of the amplitude modulation
- **Depth** (0.0-1.0): Amount of amplitude modulation

## Usage
```cpp
Tremolo trem(48000);
trem.setRate(5.0f);    // 5 Hz tremolo
trem.setDepth(0.7f);   // Deep modulation

float output = trem.process(inputSample);
```

## Implementation Notes
- Uses sine wave LFO for smooth amplitude modulation
- Very low CPU usage (just sine calculation and multiplication)
- No memory buffers required
- Classic effect found on many vintage amplifiers
- Often confused with vibrato (which modulates pitch, not amplitude)
