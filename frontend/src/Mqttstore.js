import { defineStore }    from "pinia";
import { ref, reactive }  from "vue";
import Paho               from "paho-mqtt";

const MQTT_HOST  = "www.yanacreations.com";
const MQTT_PORT  = 9001;           // Mosquitto WebSocket port
const MQTT_TOPIC = "weather/sensors";

export const useMqttStore = defineStore("mqtt", () => {
  // ── Connection state ──────────────────────────────────────────────────────
  const connected    = ref(false);
  const connecting   = ref(false);
  const lastError    = ref(null);
  const messageCount = ref(0);

  // ── Live sensor data (updated on every MQTT message) ─────────────────────
  const live = reactive({
    temperature     : null,
    humidity        : null,
    bmp_temperature : null,
    pressure        : null,
    soil_moisture   : null,
    receivedAt      : null,
  });

  // ── Ring buffer: last 60 readings for live chart ─────────────────────────
  const history = ref([]);   // [{ ts, temperature, humidity, pressure, soil }]
  const MAX_HISTORY = 60;

  // ── Internal client ref ───────────────────────────────────────────────────
  let _client = null;

  // ── Connect ───────────────────────────────────────────────────────────────
  function connect() {
    if (connected.value || connecting.value) return;
    connecting.value = true;
    lastError.value  = null;

    const clientId = `vue_ws_${Math.random().toString(16).slice(2, 8)}`;
    _client = new Paho.Client(MQTT_HOST, MQTT_PORT, clientId);

    _client.onConnectionLost = (resp) => {
      connected.value  = false;
      connecting.value = false;
      if (resp.errorCode !== 0) {
        lastError.value = resp.errorMessage;
        console.warn("[MQTT] Connection lost:", resp.errorMessage);
        // Auto-reconnect after 5 s
        setTimeout(connect, 5000);
      }
    };

    _client.onMessageArrived = (msg) => {
      try {
        const data = JSON.parse(msg.payloadString);
        live.temperature     = data.temperature     ?? live.temperature;
        live.humidity        = data.humidity        ?? live.humidity;
        live.bmp_temperature = data.bmp_temperature ?? live.bmp_temperature;
        live.pressure        = data.pressure        ?? live.pressure;
        live.soil_moisture   = data.soil_moisture   ?? live.soil_moisture;
        live.receivedAt      = new Date().toLocaleString();
        messageCount.value  += 1;

        // Push to ring buffer
        const entry = {
          ts          : Date.now(),
          temperature : data.temperature,
          humidity    : data.humidity,
          pressure    : data.pressure,
          soil        : data.soil_moisture,
        };
        history.value = [...history.value.slice(-(MAX_HISTORY - 1)), entry];
      } catch (e) {
        console.warn("[MQTT] Bad payload:", e);
      }
    };

    _client.connect({
      useSSL:    false,
      onSuccess: () => {
        connected.value  = true;
        connecting.value = false;
        lastError.value  = null;
        console.log("[MQTT] Connected via WebSocket");
        _client.subscribe(MQTT_TOPIC);
      },
      onFailure: (err) => {
        connected.value  = false;
        connecting.value = false;
        lastError.value  = err.errorMessage;
        console.warn("[MQTT] Connection failed:", err.errorMessage);
        setTimeout(connect, 5000);
      },
    });
  }

  function disconnect() {
    if (_client && connected.value) {
      _client.disconnect();
      connected.value = false;
    }
  }

  return {
    connected, connecting, lastError, messageCount,
    live, history,
    connect, disconnect,
  };
});