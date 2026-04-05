<script setup>
import { ref, computed } from "vue";

const simParams = ref({
  temperature: 25,
  humidity: 60,
  pressure: 1013,
  soil_moisture: 50,
});

const simulationResults = ref(null);

const rainProb = computed(() => {
  const p = simParams.value.pressure;
  return p < 1000 ? 80 : p < 1010 ? 60 : p < 1020 ? 30 : 10;
});

const plantHealth = computed(() => {
  const s = simParams.value.soil_moisture;
  const t = simParams.value.temperature;
  if (t > 36 || s < 10) return 'dead';
  if (s < 28 || t > 32) return 'stressed';
  if (s > 78) return 'overwatered';
  if (s > 35 && t >= 15 && t <= 30) return 'optimal';
  return 'ok';
});

const plantColor   = computed(() => ({ dead:'#a08040', stressed:'#b8c840', ok:'#66bb6a', optimal:'#4caf50', overwatered:'#2e7d32' }[plantHealth.value]));
const leafRotLeft  = computed(() => ({ dead:52, stressed:22, ok:-26, optimal:-42, overwatered:16 }[plantHealth.value]));
const leafRotRight = computed(() => ({ dead:-52, stressed:-22, ok:26, optimal:42, overwatered:-16 }[plantHealth.value]));
const showFlowers  = computed(() => ['optimal','ok'].includes(plantHealth.value));
const flowerColor  = computed(() => { const t = simParams.value.temperature; return t>28?'#ff9eb5':t<18?'#c3b6f7':'#ffc6e0'; });

const sceneBg = computed(() => {
  const r = rainProb.value; const t = simParams.value.temperature;
  if (r > 65) return 'linear-gradient(180deg, #5c6f7e 0%, #8fa3b1 100%)';
  if (t > 32) return 'linear-gradient(180deg, #e8a46a 0%, #fde8d0 100%)';
  if (t < 14) return 'linear-gradient(180deg, #8fb0d4 0%, #cce0f0 100%)';
  return 'linear-gradient(180deg, #7ecef5 0%, #c8eeff 100%)';
});

const groundBg = computed(() => {
  const s = simParams.value.soil_moisture;
  if (s > 75) return 'linear-gradient(180deg, #2e7d32 0%, #1b5e20 100%)';
  if (s < 20) return 'linear-gradient(180deg, #c4a96a 0%, #8d6e3a 100%)';
  if (s > 55) return 'linear-gradient(180deg, #388e3c 0%, #2e7d32 100%)';
  return 'linear-gradient(180deg, #4caf50 0%, #388e3c 100%)';
});

const sunSize    = computed(() => Math.max(40, Math.min(90, 16 + simParams.value.temperature * 1.9)));
const sunColor   = computed(() => { const t = simParams.value.temperature; return t>33?'#ff7043':t>27?'#ffd166':'#ffe082'; });
const sunOpacity = computed(() => Math.max(0.08, (100 - rainProb.value) / 100));

const cloudColor = computed(() => {
  const r = rainProb.value;
  return r > 60 ? 'rgba(155,170,182,0.93)' : r > 30 ? 'rgba(212,222,230,0.93)' : 'rgba(255,255,255,0.95)';
});

const dropStyle  = (i) => ({ left: (i * 6.8 - 4) + '%', animationDelay: (((i * 137) % 100) / 100) + 's', height: (7 + (i % 4) * 2) + 'px' });
const flakeStyle = (i) => ({ left: (i * 14) + '%', animationDelay: (i * 0.4) + 's', animationDuration: (2 + i * 0.28) + 's' });

const conditions = computed(() => [
  { emoji:'🌡️', label:'Temp',     value: simParams.value.temperature + '°C',  color: simParams.value.temperature>30?'#ffb6c1':simParams.value.temperature<15?'#b6d9f7':'#b6f0e4' },
  { emoji:'💧',  label:'Humidity', value: simParams.value.humidity + '%',      color: '#b6d9f7' },
  { emoji:'🌬️', label:'Pressure', value: simParams.value.pressure + ' hPa',  color: '#c3b6f7' },
  { emoji:'🌱',  label:'Soil',     value: simParams.value.soil_moisture + '%', color: '#b6f0e4' },
  { emoji:'🌧️', label:'Rain',     value: rainProb.value + '%',               color: rainProb.value > 50 ? '#b6d9f7' : '#ffd4a3' },
]);

const runSimulation = () => {
  const temp = simParams.value.temperature;
  const humid = simParams.value.humidity;
  const soil = simParams.value.soil_moisture;
  const pressure = simParams.value.pressure;

  simulationResults.value = {
    tempStatus: {
      label: temp > 30 ? "⚠️ HOT" : temp < 15 ? "❄️ COLD" : "✅ NORMAL",
      color: temp > 30 ? "red" : temp < 15 ? "blue" : "green",
    },
    humidityStatus: {
      label: humid > 75 ? "💧 VERY WET" : humid > 60 ? "💦 WET" : humid < 30 ? "🏜️ DRY" : "✅ NORMAL",
      color: humid > 75 ? "cyan" : humid > 60 ? "blue" : humid < 30 ? "orange" : "green",
    },
    soilStatus: {
      label: soil > 70 ? "💦 WET" : soil > 40 ? "✅ OPTIMAL" : "🏜️ DRY",
      color: soil > 70 ? "blue" : soil > 40 ? "green" : "red",
    },
    rainProbability: pressure < 1000 ? 80 : pressure < 1010 ? 60 : pressure < 1020 ? 30 : 10,
    recommendations: [
      temp > 28 ? "💨 Consider improving ventilation" : "✅ Temperature is good",
      humid > 65 ? "🌬️ Increase air circulation to prevent mold" : humid < 40 ? "💧 Air too dry, consider humidifying" : "✅ Humidity is optimal",
      soil < 30 ? "💧 Water your plants soon" : soil > 80 ? "🌧️ Reduce watering, soil is too wet" : "✅ Soil moisture is good",
    ],
  };
};
</script>

<template>
  <div class="sim-page">

    <div class="page-header">
      <h1 class="page-title">🎮 Simulation Mode</h1>
    </div>

    <!-- ── MAIN ROW: sliders + garden always side by side ── -->
    <div class="main-row">

      <!-- LEFT: Controls -->
      <div class="col-left">
        <div class="station-card">
          <div class="card-accent-bar controls-accent"></div>
          <div class="card-title" style="color:#ffd4a3">🎛️ Adjust Parameters</div>
          <div class="card-body">

            <div class="slider-wrap" style="border-color:rgba(255,212,163,0.2)">
              <div class="slider-header">
                <span>🌡️</span>
                <span class="slider-label" style="color:#ffd4a3">Temperature</span>
                <span class="slider-val" style="color:#ffd4a3">{{ simParams.temperature }}°C</span>
              </div>
              <v-slider v-model="simParams.temperature" min="10" max="40" step="0.5" thumb-label color="#ffd4a3" track-color="rgba(255,212,163,0.15)" hide-details />
            </div>

            <div class="slider-wrap" style="border-color:rgba(182,217,247,0.2)">
              <div class="slider-header">
                <span>💧</span>
                <span class="slider-label" style="color:#b6d9f7">Humidity</span>
                <span class="slider-val" style="color:#b6d9f7">{{ simParams.humidity }}%</span>
              </div>
              <v-slider v-model="simParams.humidity" min="0" max="100" step="1" thumb-label color="#b6d9f7" track-color="rgba(182,217,247,0.15)" hide-details />
            </div>

            <div class="slider-wrap" style="border-color:rgba(195,182,247,0.2)">
              <div class="slider-header">
                <span>🌬️</span>
                <span class="slider-label" style="color:#c3b6f7">Pressure</span>
                <span class="slider-val" style="color:#c3b6f7">{{ simParams.pressure }} hPa</span>
              </div>
              <v-slider v-model="simParams.pressure" min="950" max="1050" step="1" thumb-label color="#c3b6f7" track-color="rgba(195,182,247,0.15)" hide-details />
            </div>

            <div class="slider-wrap" style="border-color:rgba(182,240,228,0.2)">
              <div class="slider-header">
                <span>🌱</span>
                <span class="slider-label" style="color:#b6f0e4">Soil Moisture</span>
                <span class="slider-val" style="color:#b6f0e4">{{ simParams.soil_moisture }}%</span>
              </div>
              <v-slider v-model="simParams.soil_moisture" min="0" max="100" step="1" thumb-label color="#b6f0e4" track-color="rgba(182,240,228,0.15)" hide-details />
            </div>

            <button class="run-btn" @click="runSimulation">▶️ Run Simulation</button>
          </div>
        </div>
      </div>

      <!-- RIGHT: Garden -->
      <div class="col-right">
        <div class="station-card">
          <div class="card-accent-bar scene-accent"></div>
          <div class="card-title" style="color:#c3b6f7">🌿 Live Garden Preview</div>
          <div class="card-body">

            <div class="garden" :style="{ background: sceneBg }">
              <div class="garden-badges">
                <span class="gbadge" style="background:rgba(255,212,163,0.22);color:#ffd4a3;border-color:rgba(255,212,163,0.4)">🌡️ {{ simParams.temperature }}°C</span>
                <span class="gbadge" style="background:rgba(182,217,247,0.22);color:#b6d9f7;border-color:rgba(182,217,247,0.4)">💧 {{ simParams.humidity }}%</span>
                <span class="gbadge" style="background:rgba(195,182,247,0.22);color:#c3b6f7;border-color:rgba(195,182,247,0.4)">🌧️ {{ rainProb }}%</span>
              </div>

              <div class="heat-label" v-show="simParams.temperature > 33">🥵 Heat Wave!</div>

              <div class="sun" :style="{
                width: sunSize + 'px', height: sunSize + 'px',
                background: sunColor, opacity: sunOpacity,
                boxShadow: `0 0 ${sunSize/2}px ${sunColor}99, 0 0 ${sunSize*1.3}px ${sunColor}44`
              }" />

              <div class="cloud cloud-1" :style="{ opacity: Math.min(1, 0.22 + rainProb/100) }">
                <div class="cb" :style="{ background: cloudColor }"></div>
                <div class="cp cp1" :style="{ background: cloudColor }"></div>
                <div class="cp cp2" :style="{ background: cloudColor }"></div>
              </div>
              <div class="cloud cloud-2" v-show="rainProb > 22" :style="{ opacity: Math.min(1, rainProb/75) }">
                <div class="cb" :style="{ background: cloudColor }"></div>
                <div class="cp cp1" :style="{ background: cloudColor }"></div>
              </div>
              <div class="cloud cloud-3" v-show="rainProb > 55" :style="{ opacity: Math.min(1, (rainProb-55)/45) }">
                <div class="cb" :style="{ background: cloudColor }"></div>
                <div class="cp cp1" :style="{ background: cloudColor }"></div>
                <div class="cp cp2" :style="{ background: cloudColor }"></div>
              </div>

              <div class="rain-zone" v-show="rainProb > 50">
                <div class="raindrop" v-for="i in 15" :key="i" :style="dropStyle(i)"></div>
              </div>
              <div class="snow-zone" v-show="simParams.temperature < 12">
                <span class="flake" v-for="i in 7" :key="i" :style="flakeStyle(i)">❄️</span>
              </div>

              <div class="ground-layer" :style="{ background: groundBg }">
                <div class="plant" style="left:7%">
                  <div class="pflower" v-show="showFlowers" :style="{ background:flowerColor, width:'10px', height:'10px' }"></div>
                  <div class="pstem" :style="{ background:plantColor, height:'36px', width:'4px' }">
                    <div class="pleaf pleaf-l" :style="{ background:plantColor, transform:`rotate(${leafRotLeft}deg)`, width:'13px', height:'8px' }"></div>
                    <div class="pleaf pleaf-r" :style="{ background:plantColor, transform:`rotate(${leafRotRight}deg)`, width:'13px', height:'8px' }"></div>
                  </div>
                </div>
                <div class="plant" style="left:26%">
                  <div class="pflower" v-show="showFlowers" :style="{ background:flowerColor }"></div>
                  <div class="pstem" :style="{ background:plantColor, height:'66px' }">
                    <div class="pleaf pleaf-l" :style="{ background:plantColor, transform:`rotate(${leafRotLeft}deg)` }"></div>
                    <div class="pleaf pleaf-r" :style="{ background:plantColor, transform:`rotate(${leafRotRight}deg)` }"></div>
                  </div>
                </div>
                <div class="plant" style="left:50%;transform:translateX(-50%)">
                  <div class="pflower pflower-lg" v-show="showFlowers" :style="{ background:flowerColor }"></div>
                  <div class="pstem" :style="{ background:plantColor, height:'102px' }">
                    <div class="pleaf pleaf-l" :style="{ background:plantColor, transform:`rotate(${leafRotLeft}deg)`, top:'17%' }"></div>
                    <div class="pleaf pleaf-r" :style="{ background:plantColor, transform:`rotate(${leafRotRight}deg)`, top:'17%' }"></div>
                    <div class="pleaf pleaf-l" :style="{ background:plantColor, transform:`rotate(${leafRotLeft}deg)`, top:'52%' }"></div>
                    <div class="pleaf pleaf-r" :style="{ background:plantColor, transform:`rotate(${leafRotRight}deg)`, top:'52%' }"></div>
                  </div>
                </div>
                <div class="plant" style="right:26%">
                  <div class="pflower" v-show="showFlowers" :style="{ background:flowerColor }"></div>
                  <div class="pstem" :style="{ background:plantColor, height:'76px' }">
                    <div class="pleaf pleaf-l" :style="{ background:plantColor, transform:`rotate(${leafRotLeft}deg)` }"></div>
                    <div class="pleaf pleaf-r" :style="{ background:plantColor, transform:`rotate(${leafRotRight}deg)` }"></div>
                  </div>
                </div>
                <div class="plant" style="right:7%">
                  <div class="pflower" v-show="showFlowers" :style="{ background:flowerColor, width:'10px', height:'10px' }"></div>
                  <div class="pstem" :style="{ background:plantColor, height:'44px', width:'4px' }">
                    <div class="pleaf pleaf-l" :style="{ background:plantColor, transform:`rotate(${leafRotLeft}deg)`, width:'13px', height:'8px' }"></div>
                    <div class="pleaf pleaf-r" :style="{ background:plantColor, transform:`rotate(${leafRotRight}deg)`, width:'13px', height:'8px' }"></div>
                  </div>
                </div>
                <div class="puddle" v-show="simParams.soil_moisture > 76"></div>
                <div class="health-badge" :class="`hb-${plantHealth}`">
                  {{ { optimal:'🌸 Thriving!', ok:'🌿 Healthy', stressed:'😰 Stressed', overwatered:'💦 Overwatered', dead:'💀 Critical' }[plantHealth] }}
                </div>
              </div>
            </div>

            <div class="condition-row">
              <div class="cond-item" v-for="c in conditions" :key="c.label" :style="{ borderLeftColor: c.color }">
                <span style="font-size:1rem;flex-shrink:0">{{ c.emoji }}</span>
                <div>
                  <p class="cond-label">{{ c.label }}</p>
                  <p class="cond-val" :style="{ color: c.color }">{{ c.value }}</p>
                </div>
              </div>
            </div>

          </div>
        </div>
      </div>
    </div>

    <!-- ── RESULTS ROW ── -->
    <div class="results-row">
      <div class="station-card" v-if="simulationResults">
        <div class="card-accent-bar results-accent"></div>
        <div class="card-title" style="color:#b6f0e4">📋 Results</div>
        <div class="card-body">
          <div class="status-grid">
            <div class="status-item"><span class="s-label">🌡️ Temp</span><span class="s-chip">{{ simulationResults.tempStatus.label }}</span></div>
            <div class="status-item"><span class="s-label">💧 Humidity</span><span class="s-chip">{{ simulationResults.humidityStatus.label }}</span></div>
            <div class="status-item"><span class="s-label">🌱 Soil</span><span class="s-chip">{{ simulationResults.soilStatus.label }}</span></div>
            <div class="status-item"><span class="s-label">🌧️ Rain</span><span class="s-chip">{{ simulationResults.rainProbability }}%</span></div>
          </div>
          <p class="rec-title">💡 Recommendations</p>
          <ul class="rec-list">
            <li v-for="(r, i) in simulationResults.recommendations" :key="i">{{ r }}</li>
          </ul>
        </div>
      </div>

      <div class="station-card" v-else>
        <div class="card-body hint-box">
          <div style="font-size:2rem">🌤️</div>
          <p>Adjust the sliders and hit <strong style="color:#f9c6d0">Run Simulation</strong> to see results!</p>
        </div>
      </div>
    </div>

  </div>
</template>

<style scoped>
/* ── Page ── */
.sim-page {
  min-height: 100vh;
  background: transparent;
  padding: 24px;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.page-header { border-left: 4px solid #f9c6d0; padding-left: 1rem; }
.page-title  { font-size: 1.75rem; font-weight: 700; color: #f9c6d0; letter-spacing: -0.3px; margin: 0; }

/* ── THE KEY LAYOUT ── */
.main-row {
  display: flex !important;
  flex-direction: row !important;
  gap: 16px;
  align-items: stretch;
  width: 100%;
}

.col-left {
  flex: 0 0 38%;
  min-width: 0;
}

.col-right {
  flex: 1 1 0;
  min-width: 0;
}

.results-row { width: 100%; }

/* ── Cards ── */
.station-card {
  background: rgba(10,28,54,0.88);
  border-radius: 16px;
  border: 1px solid rgba(255,255,255,0.07);
  overflow: hidden;
  transition: border-color 0.2s;
  height: 100%;
}
.station-card:hover { border-color: rgba(255,255,255,0.13); }

.card-accent-bar { height: 4px; width: 100%; }
.controls-accent { background: linear-gradient(90deg, #ffd4a3, #f9c6d0, #c3b6f7); }
.results-accent  { background: linear-gradient(90deg, #b6f0e4, #b6d9f7); }
.scene-accent    { background: linear-gradient(90deg, #b6d9f7, #c3b6f7, #f9c6d0, #ffd4a3); }

.card-title {
  font-size: 1rem;
  font-weight: 600;
  display: flex;
  align-items: center;
  gap: 0.4rem;
  padding: 14px 16px 4px;
}

.card-body { padding: 12px 16px 16px; }

/* ── Sliders ── */
.slider-wrap {
  background: rgba(255,255,255,0.04);
  border-radius: 12px;
  padding: 10px 14px 2px;
  border: 1px solid;
  margin-bottom: 10px;
}
.slider-header { display: flex; align-items: center; gap: 8px; }
.slider-label  { font-size: 0.82rem; font-weight: 600; flex: 1; }
.slider-val    { font-size: 0.85rem; font-weight: 700; }

/* ── Run button ── */
.run-btn {
  width: 100%; height: 48px; border: none; border-radius: 12px;
  background: linear-gradient(135deg, #f9c6d0, #c3b6f7);
  color: #0a1c36; font-size: 0.95rem; font-weight: 700;
  cursor: pointer; letter-spacing: 0.3px; margin-top: 4px;
  box-shadow: 0 4px 18px rgba(195,182,247,0.28);
  transition: opacity 0.2s, transform 0.15s;
}
.run-btn:hover  { opacity: 0.9; transform: translateY(-1px); }
.run-btn:active { transform: scale(0.98); }

/* ── Results ── */
.status-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 8px;
  margin-bottom: 12px;
}
.status-item {
  background: rgba(255,255,255,0.04);
  border-radius: 10px; padding: 8px 10px;
  border: 1px solid rgba(255,255,255,0.06);
}
.s-label { font-size: 0.7rem; color: #8faecf; display: block; margin-bottom: 3px; }
.s-chip  { font-size: 0.82rem; font-weight: 600; color: #c8ddef; }

.rec-title {
  font-size: 0.72rem; font-weight: 600;
  text-transform: uppercase; letter-spacing: 0.5px;
  color: #8faecf; margin: 0 0 6px;
}
.rec-list { list-style: none; padding: 0; margin: 0; display: flex; gap: 8px; flex-wrap: wrap; }
.rec-list li {
  color: #c8ddef; font-size: 0.84rem;
  padding: 5px 8px;
  background: rgba(255,255,255,0.03);
  border-radius: 7px; flex: 1; min-width: 180px;
}

.hint-box { text-align: center; padding: 24px 16px; color: #8faecf; }
.hint-box p { font-size: 0.88rem; margin: 8px 0 0; }

/* ════════════════════════════
   GARDEN
════════════════════════════ */
.garden {
  position: relative; width: 100%; height: 300px;
  border-radius: 14px; overflow: hidden;
  transition: background 1s ease;
  margin-bottom: 10px;
}

.sun {
  position: absolute; top: 16px; right: 22px;
  border-radius: 50%; transition: all 0.7s ease; z-index: 2;
}

.heat-label {
  position: absolute; top: 10px; left: 50%; transform: translateX(-50%);
  font-size: 0.72rem; font-weight: 700; color: #ff7043;
  background: rgba(255,112,67,0.18); padding: 3px 12px; border-radius: 20px;
  animation: pulse 1.4s ease-in-out infinite; z-index: 6;
}
@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.45} }

.cloud { position: absolute; transition: opacity 0.9s ease; z-index: 3; }
.cb { position: absolute; border-radius: 14px; transition: background 0.8s ease; }
.cp { position: absolute; border-radius: 50%; transition: background 0.8s ease; }

.cloud-1 { top: 22px; left: 28px; width: 92px; height: 52px; }
.cloud-1 .cb  { width: 90px; height: 28px; bottom: 0; left: 0; }
.cloud-1 .cp1 { width: 46px; height: 46px; bottom: 15px; left: 8px; }
.cloud-1 .cp2 { width: 35px; height: 35px; bottom: 15px; left: 42px; }

.cloud-2 { top: 44px; left: 145px; width: 72px; height: 44px; }
.cloud-2 .cb  { width: 70px; height: 23px; bottom: 0; left: 0; }
.cloud-2 .cp1 { width: 38px; height: 38px; bottom: 13px; left: 14px; }

.cloud-3 { top: 12px; left: 225px; width: 115px; height: 62px; }
.cloud-3 .cb  { width: 113px; height: 35px; bottom: 0; left: 0; }
.cloud-3 .cp1 { width: 55px; height: 55px; bottom: 19px; left: 10px; }
.cloud-3 .cp2 { width: 42px; height: 42px; bottom: 19px; left: 54px; }

.rain-zone { position: absolute; inset: 0; z-index: 4; pointer-events: none; }
.raindrop {
  position: absolute; width: 1.5px; top: -10px;
  background: linear-gradient(180deg, transparent, rgba(182,217,247,0.78));
  border-radius: 1px; animation: rainfall 0.88s linear infinite;
}
@keyframes rainfall {
  0%   { transform: translateY(0); opacity: 0; }
  10%  { opacity: 1; }
  85%  { opacity: 0.8; }
  100% { transform: translateY(320px); opacity: 0; }
}

.snow-zone { position: absolute; inset: 0; z-index: 4; pointer-events: none; }
.flake { position: absolute; top: -20px; animation: snowfall linear infinite; }
@keyframes snowfall {
  0%   { transform: translateY(0) rotate(0deg); opacity: 1; }
  100% { transform: translateY(320px) rotate(360deg); opacity: 0; }
}

.ground-layer {
  position: absolute; bottom: 0; left: 0; right: 0;
  height: 38%; transition: background 0.8s ease;
  overflow: visible; z-index: 1;
}

.puddle {
  position: absolute; bottom: 10px; left: 50%; transform: translateX(-50%);
  width: 58px; height: 12px;
  background: rgba(100,160,220,0.42); border-radius: 50%;
  animation: shimmer 2s ease-in-out infinite;
}
@keyframes shimmer { 0%,100%{opacity:0.38} 50%{opacity:0.72} }

.plant { position: absolute; bottom: 14px; display: flex; flex-direction: column; align-items: center; }
.pstem { width: 5px; border-radius: 3px; transition: background 0.55s ease; position: relative; flex-shrink: 0; }
.pleaf { position: absolute; width: 18px; height: 10px; border-radius: 50%; top: 35%; transition: transform 0.65s ease, background 0.55s ease; }
.pleaf-l { right: 100%; transform-origin: right center; }
.pleaf-r { left:  100%; transform-origin: left  center; }
.pflower { width: 14px; height: 14px; border-radius: 50%; flex-shrink: 0; margin-bottom: 2px; transition: background 0.55s ease; box-shadow: 0 0 5px rgba(255,198,224,0.6); }
.pflower-lg { width: 18px; height: 18px; }

.health-badge {
  position: absolute; bottom: 6px; right: 8px;
  font-size: 0.68rem; font-weight: 700;
  padding: 3px 9px; border-radius: 20px;
  transition: all 0.5s ease;
}
.hb-optimal     { background: rgba(182,240,228,0.2); color: #b6f0e4; border: 1px solid rgba(182,240,228,0.35); }
.hb-ok          { background: rgba(182,240,228,0.14); color: #b6f0e4; border: 1px solid rgba(182,240,228,0.22); }
.hb-stressed    { background: rgba(255,212,163,0.2);  color: #ffd4a3; border: 1px solid rgba(255,212,163,0.35); }
.hb-overwatered { background: rgba(182,217,247,0.2);  color: #b6d9f7; border: 1px solid rgba(182,217,247,0.35); }
.hb-dead        { background: rgba(255,182,193,0.2);  color: #ffb6c1; border: 1px solid rgba(255,182,193,0.35); }

.garden-badges { position: absolute; top: 8px; left: 10px; display: flex; gap: 6px; z-index: 10; }
.gbadge { font-size: 0.7rem; font-weight: 700; padding: 3px 9px; border-radius: 20px; border: 1px solid; backdrop-filter: blur(3px); }

.condition-row { display: flex; flex-wrap: wrap; gap: 7px; }
.cond-item {
  flex: 1; min-width: 70px;
  background: rgba(255,255,255,0.04); border-radius: 9px;
  border-left: 3px solid; padding: 6px 8px;
  display: flex; align-items: center; gap: 7px;
}
.cond-label { font-size: 0.62rem; color: #8faecf; margin: 0; line-height: 1; }
.cond-val   { font-size: 0.78rem; font-weight: 700; margin: 0; line-height: 1.3; }
</style>