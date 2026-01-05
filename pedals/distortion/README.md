# Distortion Effect Pedal

## Description
Hard clipping distortion effect with aggressive, high-gain character. Includes DC blocking and tone control.

## Parameters
- **Gain** (0.0-1.0): Amount of pre-clipping gain (up to 100x)
- **Tone** (0.0-1.0): Tone control from dark to bright
- **Level** (0.0-1.0): Output volume level

## Usage
```cpp
Distortion dist;
dist.setGain(0.7f);   // High gain
dist.setTone(0.5f);   // Medium tone
dist.setLevel(0.7f);  // Output level

float output = dist.process(inputSample);
```

## Implementation Notes
- Uses hard clipping for aggressive distortion character
- DC blocking filter prevents offset buildup
- Single-pole low-pass filter for tone shaping
- Lower computational cost than overdrive
- Ideal for heavy rock and metal tones
