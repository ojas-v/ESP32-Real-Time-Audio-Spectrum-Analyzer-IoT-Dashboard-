#  ESP32 Real-Time Audio Spectrum Analyzer (IoT Dashboard)

A high-performance, low-latency audio visualization system built on the ESP32. This project combines Digital Signal Processing (DSP) for frequency analysis with a WebSocket-based React-style Dashboard for real-time remote control and monitoring.

##  Key Features

### Hardware & DSP
- Real-Time FFT: Performs 256-point Fast Fourier Transform on audio signals at 10kHz sampling rate.
- Adaptive Gain Control: Custom dynamic averaging algorithm removes DC Bias (1.25V offset) from the MAX9814 microphone, preventing "ghost" signals in silence.
- Matrix Mapping: Configurable `XY()` mapping supports both standard and 90° rotated LED layouts.

### Web Dashboard (Hosted on ESP32)
- Zero-Cloud Latency: WebSockets provide <30ms latency for instant feedback.
- Dual Visualization Modes:
  - FFT Spectrum: Frequency bars (Bass, Mids, Treble).
  - Oscilloscope View: Real-time time-domain waveform streaming.
- Virtual Hardware Mirror: HTML5 Canvas element renders a digital twin of the 8x8 matrix in the browser.
- Remote Control: Adjust Sensitivity, Brightness, and Color Modes via UI.

## 🛠️ Tech Stack
- **Embedded:** C++ (Arduino), FastLED, arduinoFFT v2.0
- **Web Frontend:** HTML5, CSS3 (Dark Mode), Chart.js, Vanilla JavaScript
- **Communication:** WebSockets (JSON payloads)
- **Hardware:** ESP32 WROOM, MAX9814 Microphone, WS2812B 8x8 Matrix

## 🔌 Pinout & Wiring

| Component | Pin | ESP32 GPIO | Note |
|:---|:---|:---|:---|
| **Microphone** | OUT | GPIO 34 | ADC1 Input |
| **Microphone** | VCC | 3.3V | Low noise rail |
| **Microphone** | GND | GND | Common Ground |
| **LED Matrix** | DIN | GPIO 18 | Data Line |
| **LED Matrix** | VCC | 5V (Ext) | Do not power via USB |
