#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <FastLED.h>
#include <arduinoFFT.h>

// IMPORT THE DASHBOARD
#include "web_interface.h" 

// --- CONFIG ---
#define MIC_PIN 34
#define LED_PIN 18
#define NUM_LEDS 64
#define NUM_COLS 8
#define NUM_ROWS 8

#define SAMPLES 256
#define SAMPLING_FREQ 8000

// --- WIFI CREDENTIALS ---
// Must match your home router for Chart.js to load!
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// --- OBJECTS ---
CRGB leds[NUM_LEDS];
double vReal[SAMPLES];
double vImag[SAMPLES];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);

WebServer server(80);
WebSocketsServer ws(81);

// --- STATE VARIABLES ---
int sensitivity = 50;
int brightness = 40;
int mode = 0; // 0=Rainbow, 1=Fire, 2=Matrix
unsigned long lastSocketSend = 0;

int XY(int x, int y) {
  return (x % 2 == 0) ? x * NUM_ROWS + y : x * NUM_ROWS + (NUM_ROWS - 1 - y);
}

void setup() {
  Serial.begin(115200);
  
  // LED Setup
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);

  // WiFi Setup (Station Mode)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nCONNECTED!");
  Serial.print("DASHBOARD URL: http://"); Serial.println(WiFi.localIP());

  // Server Setup
  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.begin();
  
  // WebSocket Setup
  ws.begin();
  ws.onEvent(webSocketEvent);
}

void loop() {
  ws.loop();
  server.handleClient();

  // 1. Sampling
  unsigned long t = micros();
  unsigned long sampling_period_us = round(1000000.0 / SAMPLING_FREQ);
  
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(MIC_PIN) - 2048;
    vImag[i] = 0;
    while (micros() - t < sampling_period_us);
    t += sampling_period_us;
  }

  // 2. FFT
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // 3. Process & Display
  int bandValues[8];
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (int i = 0; i < NUM_COLS; i++) {
    // Average bins for stability
    int val = (vReal[i*2 + 2] + vReal[i*2 + 3]) / 2; 
    
    // Apply Sensitivity Slider
    // Lower sensitivity value = Higher threshold (less sensitive)
    int threshold = map(sensitivity, 1, 100, 2000, 100); 
    
    int height = map(val, 100, threshold, 0, NUM_ROWS);
    height = constrain(height, 0, NUM_ROWS);
    bandValues[i] = height * 32; // Scale for graph (0-255)

    for (int y = 0; y < height; y++) {
      int idx = XY(i, y);
      if(mode == 0) leds[idx] = CHSV(i * 32, 255, 200);       // Rainbow
      if(mode == 1) leds[idx] = CHSV(0 + (y*28), 255, 200);   // Fire
      if(mode == 2) leds[idx] = CHSV(96, 255, 200);           // Matrix Green
    }
  }
  FastLED.show();

  // 4. Send Data to Dashboard (Limit to 20 FPS)
  if (millis() - lastSocketSend > 50) {
    String json = "{\"fft\":[";
    for(int i=0; i<8; i++) { 
      json += String(bandValues[i]) + (i<7 ? "," : ""); 
    }
    json += "], \"fps\":" + String(FastLED.getFPS()) + "}";
    ws.broadcastTXT(json);
    lastSocketSend = millis();
  }
}

// 5. Handle Commands from Dashboard
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String text = (char*)payload;
    char command = text.charAt(0);
    int value = text.substring(2).toInt();

    if (command == 'B') { 
      FastLED.setBrightness(value); 
      brightness = value; 
    }
    if (command == 'S') { 
      sensitivity = value; 
    }
    if (command == 'M') { 
      mode = value; 
    }
  }
}