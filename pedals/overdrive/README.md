# Overdrive Effect Pedal

## Description
Classic tube-style overdrive effect with soft clipping for warm, natural distortion.

## Parameters
- **Drive** (0.0-1.0): Controls the amount of overdrive/gain
- **Tone** (0.0-1.0): Tone control from dark to bright
- **Level** (0.0-1.0): Output volume level

## Usage
```cpp
Overdrive od;
od.setDrive(0.6f);
od.setTone(0.7f);
od.setLevel(0.8f);

float output = od.process(inputSample);
```

## Implementation Notes
- Uses soft clipping (tanh approximation) for smooth, musical distortion
- Simple one-pole low-pass filter for tone control
- Optimized for real-time audio processing on embedded systems
