#include <FastLED.h>
#include <arduinoFFT.h>

#define DATA_PIN 18
#define MIC_PIN 34

#define NUM_COLS 8
#define NUM_ROWS 8
#define NUM_LEDS 64

#define SAMPLES 256
#define SAMPLING_FREQ 8000

CRGB leds[NUM_LEDS];
double vReal[SAMPLES];
double vImag[SAMPLES];

// Updated constructor for v2.0
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);

uint8_t spectrum[NUM_COLS];
uint8_t peak[NUM_COLS];

unsigned long sampling_period_us;

int XY(int x, int y) {
  // Map x,y to Serpentine Layout of 8x8 matrix
  if (x % 2 == 0) return x * NUM_ROWS + y;
  else return x * NUM_ROWS + (NUM_ROWS - 1 - y);
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(40); // Keep this low for USB power

  sampling_period_us = round(1000000.0 / SAMPLING_FREQ);
}

void loop() {
  unsigned long t = micros();
  
  // 1. Sampling
  for (int i = 0; i < SAMPLES; i++) {
    int raw = analogRead(MIC_PIN);
    vReal[i] = raw - 2048; // Remove DC Offset (approx 3.3V/2)
    vImag[i] = 0;

    // Precise timing
    while (micros() - t < sampling_period_us);
    t += sampling_period_us;
  }

  // 2. FFT Processing (FIXED: lowercase function names)
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // 3. Grouping Frequencies into 8 Columns
  for (int i = 0; i < NUM_COLS; i++) {
    // Logarithmic grouping (more bands for bass)
    int start = pow(2, i);
    int end = pow(2, i + 1);
    if (end > (SAMPLES/2)) end = SAMPLES/2; 

    double sum = 0;
    for (int j = start; j < end; j++) sum += vReal[j];
    
    // Average the energy in this band
    sum = sum / (end - start);

    // Map to LED Height (Adjust '3000' if too sensitive/insensitive)
    int level = map(sum, 20, 3000, 0, NUM_ROWS);
    level = constrain(level, 0, NUM_ROWS);

    spectrum[i] = level;

    // Peak Hold Physics
    if (level > peak[i]) peak[i] = level;
    else if (peak[i] > 0 && (millis() % 60 == 0)) peak[i]--; // Gravity decay
  }

  // 4. Drawing
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  for (int x = 0; x < NUM_COLS; x++) {
    // Draw Bar
    for (int y = 0; y < spectrum[x]; y++) {
      leds[XY(x, y)] = CHSV(map(x, 0, 7, 0, 224), 255, 200); // Rainbow gradient
    }
    // Draw Peak Dot
    if (peak[x] > 0)
      leds[XY(x, peak[x] - 1)] = CRGB::White;
  }

  FastLED.show();
}
