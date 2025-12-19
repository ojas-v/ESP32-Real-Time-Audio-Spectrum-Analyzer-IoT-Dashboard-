import websocket
import json
import time

# UPDATE THIS IP
ESP_IP = "172.20.10.2"  
URL = f"ws://{ESP_IP}:81/"

def on_message(ws, message):
    try:
        data = json.loads(message)
        
        # Check if we are receiving FFT data
        if "fft" in data:
            fft_vals = data["fft"]
            fps = data.get("fps", 0)
            
            # Draw ASCII Graph
            graph = " | ".join(["#" * val for val in fft_vals])
            print(f"[FFT] FPS: {fps} | {graph}")
            
        # Check if we are receiving Waveform data
        elif "wave" in data:
            wave_vals = data["wave"]
            fps = data.get("fps", 0)
            # Just print the first few values to show it's working
            print(f"[WAVE] FPS: {fps} | Samples: {wave_vals[:5]}...")

    except Exception as e:
        print(f"Error: {e}")

def on_error(ws, error):
    print(f"Error: {error}")

def on_close(ws, close_status_code, close_msg):
    print("### Closed ###")

def on_open(ws):
    print(f"### Connected to {ESP_IP} ###")

if __name__ == "__main__":
    ws = websocket.WebSocketApp(URL,
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    try:
        ws.run_forever()
    except KeyboardInterrupt:
        print("\nExiting...")