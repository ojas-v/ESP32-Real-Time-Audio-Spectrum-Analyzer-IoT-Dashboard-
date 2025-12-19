const ws = new WebSocket("ws://192.168.4.1:81/");
ws.onmessage = e => {
  console.log(e.data);
};
