# Cleveland Sound Hothouse

Effect pedals code for Cleveland Sound Hothouse guitar effects pedal platform.

## Overview

This project provides a complete library of guitar effect pedal implementations designed for deployment to the Cleveland Sound Hothouse pedal. Each effect is optimized for real-time audio processing on embedded systems.

## Project Structure

```
hothouse/
├── hothouse.h              # Base effect interface and utilities
├── deploy.cpp              # Example deployment code
├── build.sh               # Build script
├── pedals/                # Effect pedal implementations
│   ├── overdrive/         # Tube-style overdrive with soft clipping
│   ├── delay/             # Digital delay with feedback
│   ├── reverb/            # Schroeder reverberator
│   ├── chorus/            # Modulated delay chorus
│   ├── distortion/        # Hard clipping distortion
│   ├── fuzz/              # Classic fuzz with asymmetric clipping
│   ├── tremolo/           # Amplitude modulation effect
│   └── compressor/        # Dynamic range compressor
```

## Available Effects

### Distortion Effects
- **Overdrive**: Warm, tube-style overdrive with soft clipping
- **Distortion**: Aggressive hard clipping distortion
- **Fuzz**: Classic vintage fuzz with asymmetric clipping

### Modulation Effects
- **Chorus**: Rich, shimmering modulated delay
- **Tremolo**: Rhythmic amplitude modulation

### Time-Based Effects
- **Delay**: Digital delay with adjustable time and feedback
- **Reverb**: Schroeder reverberator for spatial effects

### Dynamic Effects
- **Compressor**: Dynamic range compression with envelope follower

## Quick Start

### Building

```bash
./build.sh
```

### Using Effects

```cpp
#include "hothouse.h"
#include "pedals/overdrive/overdrive.cpp"

int main() {
    // Create and configure an effect
    Overdrive overdrive;
    overdrive.setDrive(0.6f);
    overdrive.setTone(0.7f);
    overdrive.setLevel(0.8f);
    
    // Process audio
    float output = overdrive.process(inputSample);
    
    return 0;
}
```

### Creating Effect Chains

```cpp
// Chain multiple effects together
Compressor comp;
Overdrive od;
Delay delay(48000);

HothouseEffect* chain[] = {&comp, &od, &delay};
EffectChain effectChain(chain, 3);

float output = effectChain.process(input);
```

## Hardware Configuration

The Hothouse pedal is configured with the following specifications:
- Sample Rate: 48kHz
- Buffer Size: 128 samples
- ADC/DAC Resolution: 24-bit

## Effect Details

Each effect folder contains:
- **Implementation file** (`.cpp`): Complete effect algorithm
- **README.md**: Documentation with parameters and usage examples

See individual effect folders for detailed documentation.

## Deployment

The `deploy.cpp` file provides examples of:
1. Single effect deployment
2. Effect chain creation
3. Audio buffer processing
4. Hardware integration

Modify this file to deploy your desired effect configuration to the Hothouse pedal.

## Development

### Adding New Effects

1. Create a new folder in `pedals/`
2. Implement your effect class inheriting from `HothouseEffect`
3. Override `process()` and `reset()` methods
4. Add documentation in a README.md file

### Effect Interface

```cpp
class MyEffect : public HothouseEffect {
public:
    float process(float inputSample) override {
        // Your effect processing here
        return processedSample;
    }
    
    void reset() override {
        // Clear buffers, reset state
    }
};
```

## Performance Considerations

- All effects are optimized for real-time processing
- Memory usage is clearly documented for each effect
- No dynamic memory allocation in processing loops
- Fixed-point arithmetic can be used for further optimization

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please submit pull requests with new effects or improvements to existing ones.
