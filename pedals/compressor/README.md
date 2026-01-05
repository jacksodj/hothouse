# Compressor Effect Pedal

## Description
Dynamic range compressor that reduces the volume of loud signals and can increase quiet signals, creating more consistent dynamics.

## Parameters
- **Threshold** (0.0-1.0): Level above which compression is applied
- **Ratio** (1.0-20.0): Amount of gain reduction (4:1 is common)
- **Attack** (0.5-0.99): How quickly compression responds to signal increases
- **Release** (0.9-0.999): How quickly compression releases after signal decreases
- **Makeup Gain** (0.5-10.0): Output gain to compensate for compression

## Usage
```cpp
Compressor comp;
comp.setThreshold(0.5f);
comp.setRatio(4.0f);
comp.setAttack(0.9f);
comp.setRelease(0.999f);
comp.setMakeupGain(2.0f);

float output = comp.process(inputSample);
```

## Implementation Notes
- Uses envelope follower for level detection
- Separate attack and release times for natural sound
- Makeup gain compensates for compression loss
- Essential for evening out playing dynamics
- Common in studio and live guitar rigs
