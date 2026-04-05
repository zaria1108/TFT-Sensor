<script setup>
import { computed, ref } from "vue";
import { useAppStore } from "@/stores/appStore.js";
import { useMqttStore } from "@/stores/mqttStore.js";
import { formatTimestamp } from "@/utils/weatherData.js";

const app = useAppStore();
const mqtt = useMqttStore();

const useFahrenheit = ref(false);
const tempUnit = computed(() => useFahrenheit.value ? "°F" : "°C");

function toF(c) {
  if (c == null) return null;
  return Number(c) * 9 / 5 + 32;
}

function displayTemp(raw) {
  if (raw == null) return null;
  return useFahrenheit.value ? toF(raw) : Number(raw);
}

const liveData = computed(() => app.latestReading ?? {});

function fmt(value, decimals = 1) {
  return value == null ? "--" : Number(value).toFixed(decimals);
}

function soilMeta(percent) {
  if (percent == null) return { label: "--", color: "#94a3b8" };
  if (percent > 60) return { label: "Hydrated ✨", color: "#ccfbf1" };
  if (percent > 30) return { label: "Happy 🌿", color: "#fef08a" };
  return { label: "Thirsty ☁️", color: "#fbcfe8" };
}

function tempStatus(celsius) {
  if (celsius == null) return { label: "No signal", color: "#94a3b8" };
  const v = Number(celsius);
  if (v > 32) return { label: "Toasty ☀️", color: "#fbcfe8" };
  if (v > 25) return { label: "Warm", color: "#fef08a" };
  if (v >= 18) return { label: "Lovely ✨", color: "#ccfbf1" };
  if (v >= 10) return { label: "Chilly ❄️", color: "#7ecef4" };
  return { label: "Brrr 🧊", color: "#e9d5ff" };
}

function pressureStatus(hpa) {
  if (hpa == null) return { label: "No signal", color: "#94a3b8" };
  const v = Number(hpa);
  if (v > 1020) return { label: "Clear Skies ⛅", color: "#fef08a" };
  if (v >= 1000) return { label: "Stable ☁️", color: "#ccfbf1" };
  return { label: "Rainy Vibes 💧", color: "#fbcfe8" };
}

function humidStatus(h) {
  if (h == null) return { label: "No signal", color: "#94a3b8" };
  const v = Number(h);
  if (v > 80) return { label: "Dewy 🫧", color: "#e9d5ff" };
  if (v > 60) return { label: "Humid ☁️", color: "#e9d5ff" };
  if (v >= 40) return { label: "Perfect ✨", color: "#ccfbf1" };
  return { label: "Dry Air 🌬️", color: "#fef08a" };
}

const SOIL_R = 46;
const SOIL_C = 2 * Math.PI * SOIL_R;
const SOIL_ARC = 0.75;
const SOIL_ARC_LEN = SOIL_C * SOIL_ARC;
const SOIL_OFFSET = (SOIL_C * (1 - SOIL_ARC)) / 2 + SOIL_C * 0.25;

function soilDash(pct) {
  const filled = Math.min(100, Math.max(0, pct ?? 0)) / 100 * SOIL_ARC_LEN;
  return `${filled.toFixed(1)} ${(SOIL_C - filled).toFixed(1)}`;
}
</script>

<template>
  <div class="dashboard-root">
    <div class="orb orb-1" />
    <div class="orb orb-2" />
    <div class="orb orb-3" />

    <div class="dash-container">

      <!-- Header -->
      <div class="header-row">
        <div class="header-left">
          <div class="eyebrow">✨ Environmental Monitor ✨</div>
          <h2 class="page-title">☁️ Live Dashboard</h2>
          <div class="timestamp mono">
            <span class="pulse-dot" />
            {{ formatTimestamp(liveData.timestamp) || "Awaiting data... ⏳" }}
          </div>
        </div>
        <div class="header-right">
          <button class="unit-toggle mono" @click="useFahrenheit = !useFahrenheit">
            <span :class="{ 'unit-active': !useFahrenheit }">°C</span>
            <span class="unit-sep"> | </span>
            <span :class="{ 'unit-active': useFahrenheit }">°F</span>
          </button>
          <div class="status-pill" :class="mqtt.connected ? 'pill-mint' : 'pill-peach'">
            <v-icon size="12" class="mr-1">{{ mqtt.connected ? 'mdi-wifi' : 'mdi-wifi-off' }}</v-icon>
            {{ mqtt.connected ? `MQTT · ${mqtt.messageCount} msgs` : "Offline" }}
          </div>
        </div>
      </div>

      <!-- Main Cards -->
      <div class="main-grid">

        <!-- Temp & Humidity -->
        <div class="main-card">
          <div class="card-accent-bar pastel-bar"></div>
          <div class="card-inner">
            <div class="card-section-label">🌡️ Temp &amp; Humidity ☁️</div>
            <div class="temp-hero">
              <span class="temp-value mono" :style="{ color: tempStatus(liveData.temperature).color }">
                {{ fmt(displayTemp(liveData.temperature), 1) }}
              </span>
              <span class="temp-unit">{{ tempUnit }}</span>
            </div>
            <div class="status-badge" :style="{ color: tempStatus(liveData.temperature).color, borderColor: tempStatus(liveData.temperature).color }">
              {{ tempStatus(liveData.temperature).label }}
            </div>
            <div class="humidity-row">
              <span class="sub-label">💧 Humidity</span>
              <span class="sub-value mono" style="color:#a5b4fc">{{ fmt(liveData.humidity, 1) }}%</span>
            </div>
            <div class="hum-bar-track">
              <div class="hum-bar-fill" :style="{ width: (liveData.humidity != null ? Math.min(100, liveData.humidity) : 0) + '%' }" />
            </div>
          </div>
        </div>

        <!-- Soil Moisture -->
        <div class="main-card">
          <div class="card-accent-bar soft-green"></div>
          <div class="card-inner card-inner--center">
            <div class="card-section-label">🌿 Soil Moisture 💧</div>
            <svg viewBox="0 0 110 110" class="soil-svg">
              <circle cx="55" cy="55" :r="SOIL_R" fill="none"
                stroke="rgba(255,255,255,0.07)" stroke-width="8" stroke-linecap="round"
                :stroke-dasharray="`${SOIL_ARC_LEN.toFixed(1)} ${(SOIL_C - SOIL_ARC_LEN).toFixed(1)}`"
                :stroke-dashoffset="-SOIL_OFFSET" />
              <circle cx="55" cy="55" :r="SOIL_R" fill="none"
                :stroke="soilMeta(liveData.soil_moisture).color"
                stroke-width="8" stroke-linecap="round"
                :stroke-dasharray="soilDash(liveData.soil_moisture)"
                :stroke-dashoffset="-SOIL_OFFSET" />
              <text x="55" y="65" text-anchor="middle" font-size="22" font-weight="800"
                font-family="monospace" :fill="soilMeta(liveData.soil_moisture).color">
                {{ liveData.soil_moisture != null ? Math.round(liveData.soil_moisture) : '--' }}
              </text>
            </svg>
            <div class="status-badge" :style="{ color: soilMeta(liveData.soil_moisture).color, borderColor: soilMeta(liveData.soil_moisture).color }">
              {{ soilMeta(liveData.soil_moisture).label }}
            </div>
          </div>
        </div>

        <!-- Air Pressure -->
        <div class="main-card">
          <div class="card-accent-bar soft-blue"></div>
          <div class="card-inner card-inner--center">
            <div class="card-section-label">🌬️ Air Pressure 📍</div>
            <div class="pressure-hero">
              <span class="pressure-value mono" style="color:#fef08a">{{ fmt(liveData.pressure, 0) }}</span>
              <span class="pressure-unit">hPa</span>
            </div>
            <div class="status-badge" :style="{ color: pressureStatus(liveData.pressure).color, borderColor: pressureStatus(liveData.pressure).color }">
              {{ pressureStatus(liveData.pressure).label }}
            </div>
            <div class="rain-info">
              <span class="sub-label">Est. Rain Chance</span>
              <span class="sub-value mono" style="color:#7ecef4">
                {{ liveData.pressure == null ? '--' : Number(liveData.pressure) <= 1000 ? '80%' : '15%' }}
              </span>
            </div>
          </div>
        </div>

      </div>

      <!-- Session Summary -->
      <div class="section-label">✨ Session Summary ✨</div>
      <div class="summary-grid">
        <div class="summary-card">
          <div class="summary-icon">📡</div>
          <div class="summary-body">
            <div class="summary-label">Readings</div>
            <div class="summary-value mono">{{ mqtt.history.length || '--' }}</div>
          </div>
        </div>
        <div class="summary-card summary-card--wide">
          <div class="summary-icon">🔎</div>
          <div class="summary-body">
            <div class="summary-label">Overall Status</div>
            <div class="summary-conditions">
              <span class="cond-chip">{{ tempStatus(liveData.temperature).label }}</span>
              <span class="cond-chip">{{ humidStatus(liveData.humidity).label }}</span>
            </div>
          </div>
        </div>
      </div>

    </div>
  </div>
</template>

<style scoped>
/* ── Root ── */
.dashboard-root {
  --bg: #0a192f;
  --card-bg: var(--navy-mid);
  --border: rgba(165, 180, 252, 0.15);
  --text: #f8fafc;
  --muted: #94a3b8;
  position: relative;
  min-height: 100vh;
  background: var(--bg);
  color: var(--text);
  overflow-x: hidden;
  box-sizing: border-box;
}

*, *::before, *::after { box-sizing: border-box; }

/* ── Orbs ── */
.orb { position: absolute; border-radius: 50%; filter: blur(100px); pointer-events: none; opacity: 0.3; }
.orb-1 { width: 400px; height: 400px; background: #fbcfe8; top: -100px; left: -100px; }
.orb-2 { width: 300px; height: 300px; background: #e9d5ff; bottom: 10%; right: -50px; }
.orb-3 { width: 250px; height: 250px; background: #ccfbf1; bottom: -50px; left: 20%; }

/* ── Layout container ── */
.dash-container {
  position: relative;
  width: 100%;
  padding: 24px;
  display: flex;
  flex-direction: column;
  gap: 24px;
}

/* ── Header ── */
.header-row {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  flex-wrap: wrap;
  gap: 16px;
  width: 100%;
}

.header-left { display: flex; flex-direction: column; gap: 4px; }
.header-right { display: flex; flex-direction: column; align-items: flex-end; gap: 8px; flex-shrink: 0; }

.eyebrow { font-size: 0.65rem; font-weight: 700; letter-spacing: 3px; color: var(--rose); text-transform: uppercase; }
.page-title { font-size: 2.2rem; font-weight: 800; letter-spacing: -1px; margin: 0; color: var(--rose); }
.timestamp { font-size: 0.75rem; color: var(--muted); display: flex; align-items: center; gap: 8px; }
.pulse-dot { width: 8px; height: 8px; min-width: 8px; background: #ccfbf1; border-radius: 50%; box-shadow: 0 0 8px #ccfbf1; }

.unit-toggle {
  background: rgba(255,255,255,0.05);
  border: 1px solid var(--border);
  border-radius: 999px;
  padding: 5px 14px;
  font-size: 0.7rem;
  color: var(--muted);
  cursor: pointer;
}
.unit-active { color: var(--rose); font-weight: 700; }

.status-pill {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 5px 14px;
  border-radius: 999px;
  font-size: 0.65rem;
  font-weight: 700;
  border: 1px solid var(--border);
  background: rgba(255,255,255,0.03);
}
.pill-mint { color: #ccfbf1; }
.pill-peach { color: #f8a5a5; }

/* ── Main grid ── */
.main-grid {
  display: grid;
  grid-template-columns: minmax(0, 1.4fr) minmax(0, 1fr) minmax(0, 1fr);
  grid-auto-rows: 1fr;
  align-items: stretch;
  gap: 20px;
  width: 100%;
}

@media (max-width: 900px) {
  .main-grid { grid-template-columns: 1fr; grid-auto-rows: auto; }
}

/* ── Cards ── */
.main-card {
  background: var(--navy-mid);
  border: 1px solid var(--border);
  border-radius: 28px;
  backdrop-filter: blur(20px);
  transition: transform 0.3s ease;
  min-width: 0;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}
.main-card:hover { transform: translateY(-5px); box-shadow: 0 12px 30px rgba(0,0,0,0.4); }

/* Accent bars (matching Analysis page style) */
.card-accent-bar { height: 4px; }
.pastel-bar { background: linear-gradient(90deg, #e8a0b4, #b6d9f7); }
.soft-blue { background: #7ecef4; }
.soft-green { background: #69e0a0; }

.card-inner {
  padding: 28px;
  flex: 1;
  display: flex;
  flex-direction: column;
}
.card-inner--center {
  align-items: center;
  text-align: center;
  justify-content: center;
}

.card-section-label {
  font-size: 0.6rem;
  font-weight: 800;
  letter-spacing: 2px;
  color: var(--muted);
  text-transform: uppercase;
  margin-bottom: 16px;
}

/* Temp card */
.temp-hero { display: flex; align-items: baseline; gap: 4px; }
.temp-value { font-size: 5rem; font-weight: 700; line-height: 1; letter-spacing: -2px; }
.temp-unit { font-size: 1.2rem; opacity: 0.5; }

.status-badge {
  display: inline-block;
  padding: 4px 14px;
  border-radius: 999px;
  border: 1px solid;
  font-size: 0.65rem;
  font-weight: 700;
  text-transform: uppercase;
  margin-top: 10px;
  background: rgba(255,255,255,0.05);
  align-self: flex-start;
}
.card-inner--center .status-badge { align-self: center; }

.humidity-row { display: flex; justify-content: space-between; align-items: center; margin-top: auto; padding-top: 16px; }
.sub-label { font-size: 0.75rem; color: var(--muted); }
.sub-value { font-size: 1rem; font-weight: 600; }
.hum-bar-track { width: 100%; height: 6px; background: rgba(255,255,255,0.1); border-radius: 999px; overflow: hidden; margin-top: 8px; }
.hum-bar-fill { height: 100%; background: #a5b4fc; transition: width 0.6s ease; }

/* Soil card */
.soil-svg { width: 150px; height: 150px; margin: 8px 0; }

/* Pressure card */
.pressure-hero { display: flex; align-items: baseline; gap: 6px; }
.pressure-value { font-size: 3.8rem; font-weight: 700; line-height: 1; letter-spacing: -2px; }
.pressure-unit { font-size: 1rem; opacity: 0.5; }
.rain-info { display: flex; flex-direction: column; align-items: center; gap: 4px; margin-top: 16px; }

/* ── Summary ── */
.section-label { font-size: 0.7rem; font-weight: 800; letter-spacing: 3px; color: var(--rose); text-align: center; }

.summary-grid {
  display: grid;
  grid-template-columns: minmax(0, 1fr) minmax(0, 1fr);
  gap: 16px;
  width: 100%;
}

.summary-card {
  display: flex;
  align-items: center;
  gap: 16px;
  background: var(--navy-mid);
  border: 1px solid var(--border);
  border-left: 4px solid var(--rose);
  border-radius: 20px;
  padding: 20px;
  min-width: 0;
}
.summary-card--wide { border-left-color: #7ecef4; }
.summary-icon { font-size: 1.8rem; flex-shrink: 0; }
.summary-body { min-width: 0; }
.summary-label { font-size: 0.55rem; font-weight: 800; color: var(--muted); text-transform: uppercase; }
.summary-value { font-size: 1.2rem; font-weight: 700; }
.summary-conditions { display: flex; flex-wrap: wrap; gap: 6px; margin-top: 6px; }
.cond-chip {
  padding: 4px 12px;
  border-radius: 999px;
  border: 1px solid var(--border);
  font-size: 0.6rem;
  font-weight: 700;
  background: rgba(255,255,255,0.05);
  white-space: nowrap;
}

.mono { font-family: monospace; }
</style>