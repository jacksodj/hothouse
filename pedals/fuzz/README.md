# Fuzz Effect Pedal

## Description
Classic fuzz effect with extreme gain and asymmetric clipping for vintage fuzz tones.

## Parameters
- **Fuzz** (0.0-1.0): Amount of fuzz/gain (up to 200x)
- **Level** (0.0-1.0): Output volume level

## Usage
```cpp
Fuzz fuzz;
fuzz.setFuzz(0.8f);   // Heavy fuzz
fuzz.setLevel(0.7f);  // Output level

float output = fuzz.process(inputSample);
```

## Implementation Notes
- Asymmetric clipping mimics vintage germanium transistor behavior
- Extremely high gain (up to 200x) for classic fuzz character
- Simple design with minimal CPU usage
- Ideal for vintage rock and psychedelic tones
- Famous examples: Fuzz Face, Tone Bender
