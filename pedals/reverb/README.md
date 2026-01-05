# Reverb Effect Pedal

## Description
Reverb effect using Schroeder reverberator algorithm with parallel comb filters and series allpass filters.

## Parameters
- **Room Size** (0.0-1.0): Size of the simulated space (affects decay time)
- **Damping** (0.0-1.0): High-frequency damping (simulates absorption)
- **Mix** (0.0-1.0): Balance between dry and wet signal

## Usage
```cpp
Reverb reverb;
reverb.setRoomSize(0.7f);  // Large room
reverb.setDamping(0.5f);   // Medium damping
reverb.setMix(0.3f);       // 30% wet signal

float output = reverb.process(inputSample);
```

## Implementation Notes
- Based on Schroeder reverberator design
- Uses 4 parallel comb filters and 2 series allpass filters
- Optimized delay times for natural-sounding reverb
- Memory requirement: ~16KB for all delay buffers
- Computational cost: Moderate (6 filters per sample)
