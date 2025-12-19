// web_interface.h
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Spectrum</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    /* Dark Mode Styling */
    body { background-color: #121212; color: #e0e0e0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; text-align: center; padding: 20px; }
    h2 { color: #bb86fc; letter-spacing: 2px; text-transform: uppercase; }
    
    /* Container for the graph */
    .chart-container { position: relative; height: 300px; width: 100%; max-width: 800px; margin: 0 auto; }
    
    /* Sliders and Inputs */
    .control-group { background: #1e1e1e; padding: 15px; margin: 15px auto; border-radius: 12px; max-width: 400px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    label { display: block; margin-bottom: 10px; font-weight: bold; color: #03dac6; }
    input[type=range] { width: 100%; accent-color: #bb86fc; cursor: pointer; }
    
    /* Buttons */
    .btn-group { margin-top: 10px; }
    button { background: #333; color: white; border: 1px solid #bb86fc; padding: 10px 20px; margin: 5px; cursor: pointer; border-radius: 20px; transition: 0.2s; }
    button:hover { background: #bb86fc; color: black; font-weight: bold; }
    
    /* Status Footer */
    #status { font-size: 0.8rem; color: #666; margin-top: 30px; }
  </style>
</head>
<body>

  <h2> Audio Visualizer</h2>
  
  <div class="chart-container">
    <canvas id="spectrumChart"></canvas>
  </div>

  <div class="control-group">
    <label>Brightness: <span id="valBright">40</span></label>
    <input type="range" min="0" max="255" value="40" oninput="send('B', this.value)">
    
    <label>Sensitivity: <span id="valSens">50</span></label>
    <input type="range" min="1" max="100" value="50" oninput="send('S', this.value)">
    
    <label>Mode Select</label>
    <div class="btn-group">
      <button onclick="send('M', 0)"> Rainbow</button>
      <button onclick="send('M', 1)"> Fire</button>
      <button onclick="send('M', 2)"> Matrix</button>
    </div>
  </div>

  <div id="status">Status: Connecting... | FPS: 0</div>

<script>
  // --- JAVASCRIPT LOGIC ---
  var gateway = `ws://${window.location.hostname}:81/`;
  var websocket;
  var ctx = document.getElementById('spectrumChart').getContext('2d');

  // Configure the Chart
  var chart = new Chart(ctx, {
      type: 'bar',
      data: {
          labels: ['Bass', 'Low-Mid', 'Mid', 'High-Mid', 'High', 'Pres', 'Brill', 'Air'],
          datasets: [{
              label: 'Amplitude',
              data: [0,0,0,0,0,0,0,0],
              backgroundColor: [
                '#FF0000', '#FF7F00', '#FFFF00', '#00FF00', 
                '#0000FF', '#4B0082', '#9400D3', '#FFFFFF'
              ],
              borderRadius: 5,
              borderSkipped: false
          }]
      },
      options: {
          responsive: true,
          maintainAspectRatio: false,
          animation: false, // CRITICAL: Disable animation for real-time performance
          scales: {
              y: { min: 0, max: 255, display: false }, // Hide Y axis for clean look
              x: { ticks: { color: '#888' }, grid: { display: false } }
          },
          plugins: { legend: { display: false } }
      }
  });

  // WebSocket Handling
  function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = () => { document.getElementById('status').innerText = "Status: Connected"; };
    websocket.onclose = () => { document.getElementById('status').innerText = "Status: Disconnected"; setTimeout(initWebSocket, 2000); };
    websocket.onmessage = (event) => {
      var data = JSON.parse(event.data);
      
      // Update Graph
      if (data.fft) { 
        chart.data.datasets[0].data = data.fft; 
        chart.update(); 
      }
      
      // Update FPS Counter
      if (data.fps) {
         var txt = document.getElementById('status').innerText.split('|')[0];
         document.getElementById('status').innerText = txt + "| FPS: " + data.fps;
      }
    };
  }

  // Send Commands to ESP32
  function send(key, value) {
    websocket.send(key + ":" + value);
    if(key === 'B') document.getElementById('valBright').innerText = value;
    if(key === 'S') document.getElementById('valSens').innerText = value;
  }

  window.onload = initWebSocket;
</script>
</body>
</html>
)rawliteral";
