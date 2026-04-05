from datetime import timezone, timedelta

# ── MQTT ──────────────────────────────────────────────────────────────────────
MQTT_BROKER = "www.yanacreations.com"
MQTT_PORT   = 1883
MQTT_TOPIC  = "weather/sensors"

# ── MongoDB ───────────────────────────────────────────────────────────────────
MONGO_URI   = "mongodb://localhost:27017/"
DB_NAME     = "weather_station"
COLLECTION  = "readings"

# ── Flask ─────────────────────────────────────────────────────────────────────
FLASK_PORT  = 5000

# ── Timezone ──────────────────────────────────────────────────────────────────
JAMAICA_TZ  = timezone(timedelta(hours=-5))