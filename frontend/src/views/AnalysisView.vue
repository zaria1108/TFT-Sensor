<script setup>
import { computed } from "vue";

import { useAppStore } from "@/stores/appStore.js";
import { buildRisks, calculateStats, formatDelta, trendLabel } from "@/utils/weatherData.js";

const app = useAppStore();

const latest = computed(() => app.latestReading ?? {});
const temperatureStats = computed(() => calculateStats(app.readings, "temperature"));
const humidityStats = computed(() => calculateStats(app.readings, "humidity"));
const pressureStats = computed(() => calculateStats(app.readings, "pressure"));
const soilStats = computed(() => calculateStats(app.readings, "soil_moisture"));

const trendCards = computed(() => [
  {
    title: "Temperature Trend",
    icon: temperatureStats.value.change >= 0 ? "mdi-trending-up" : "mdi-trending-down",
    color: "#7ecef4",
    value: formatDelta(temperatureStats.value.change, " °C"),
    subtitle: `${trendLabel(temperatureStats.value.change)} across recent readings`,
  },
  {
    title: "Humidity Trend",
    icon: humidityStats.value.change >= 0 ? "mdi-trending-up" : "mdi-trending-down",
    color: "#a5b4fc",
    value: formatDelta(humidityStats.value.change, "%"),
    subtitle: `${trendLabel(humidityStats.value.change)} across recent readings`,
  },
  {
    title: "Pressure Trend",
    icon: pressureStats.value.change >= 0 ? "mdi-trending-up" : "mdi-trending-down",
    color: "#c3b6f7",
    value: formatDelta(pressureStats.value.change, " hPa"),
    subtitle: `${trendLabel(pressureStats.value.change)} pressure pattern`,
  },
  {
    title: "Soil Moisture Trend",
    icon: soilStats.value.change >= 0 ? "mdi-trending-up" : "mdi-trending-down",
    color: "#69e0a0",
    value: formatDelta(soilStats.value.change, "%"),
    subtitle: `${trendLabel(soilStats.value.change)} soil condition`,
  },
]);

const risks = computed(() => buildRisks(latest.value));

const rainChance = computed(() => {
  const pressure = Number(latest.value.pressure ?? 0);
  if (!pressure) return "--";
  if (pressure <= 1000) return "80%";
  if (pressure <= 1008) return "60%";
  if (pressure <= 1015) return "35%";
  return "15%";
});
</script>

<template>
  <div class="analysis-page">
    <!-- Header -->
    <div class="page-header">
      <h1 class="page-title">✨ Trend Analysis & Insights</h1>
    </div>

    <!-- Trend Cards -->
    <div class="trend-grid">
      <div class="station-card trend-card" v-for="card in trendCards" :key="card.title">
        <div class="card-accent-bar pastel-bar"></div>

        <div class="card-body center">
          <v-icon size="28" :color="card.color" class="mb-2">
            {{ card.icon }}
          </v-icon>

          <div class="trend-title">{{ card.title }}</div>
          <div class="trend-value">{{ card.value }}</div>
          <div class="trend-sub">{{ card.subtitle }}</div>
        </div>
      </div>
    </div>

    <!-- Risk + Forecast -->
    <div class="insight-grid">
      <!-- Risks -->
      <div class="station-card">
        <div class="card-accent-bar soft-red"></div>

        <div class="card-header">
          <div class="card-title">⚠️ Active Risks</div>
        </div>

        <div class="card-body">
          <div v-if="risks.length">
            <div v-for="risk in risks" :key="risk.title" class="risk-pill">
              <v-icon size="16">{{ risk.icon }}</v-icon>
              <div>
                <div class="risk-title">{{ risk.title }}</div>
                <div class="risk-sub">{{ risk.subtitle }}</div>
              </div>
            </div>
          </div>

          <div v-else class="empty-state">
            No immediate risk detected ✨
          </div>
        </div>
      </div>

      <!-- Forecast -->
      <div class="station-card">
        <div class="card-accent-bar soft-purple"></div>

        <div class="card-header">
          <div class="card-title">🌤️ Forecast</div>
        </div>

        <div class="card-body">
          <div class="forecast-item">
            🌧️ Rain Chance: <strong>{{ rainChance }}</strong>
          </div>
          <div class="forecast-item">
            📉 Pressure Trend: {{ trendLabel(pressureStats.change) }}
          </div>
          <div class="forecast-item">
            🌡️ Temp: {{ latest.temperature ?? "--" }} °C
          </div>

          <div class="recommendation">
            💡 Check recommendations page for actions
          </div>
        </div>
      </div>
    </div>

    <!-- Stats -->
    <div class="stats-grid">
      <div class="station-card">
        <div class="card-accent-bar soft-blue"></div>

        <div class="card-header">
          <div class="card-title">🌡️ Temperature Stats</div>
        </div>

        <div class="card-body pills">
          <div class="stat-pill">Min: {{ temperatureStats.min?.toFixed(1) ?? "--" }} °C</div>
          <div class="stat-pill">Max: {{ temperatureStats.max?.toFixed(1) ?? "--" }} °C</div>
          <div class="stat-pill">Avg: {{ temperatureStats.avg?.toFixed(1) ?? "--" }} °C</div>
        </div>
      </div>

      <div class="station-card">
        <div class="card-accent-bar soft-indigo"></div>

        <div class="card-header">
          <div class="card-title">💧 Humidity Stats</div>
        </div>

        <div class="card-body pills">
          <div class="stat-pill">Min: {{ humidityStats.min?.toFixed(1) ?? "--" }}%</div>
          <div class="stat-pill">Max: {{ humidityStats.max?.toFixed(1) ?? "--" }}%</div>
          <div class="stat-pill">Avg: {{ humidityStats.avg?.toFixed(1) ?? "--" }}%</div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.analysis-page {
  padding: 24px;
}

.page-title {
  color: var(--rose);
}

/* Layouts */
.trend-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 16px;
  margin-bottom: 24px;
}

.insight-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 20px;
  margin-bottom: 24px;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 20px;
}

/* Cards */
.station-card {
  background: var(--navy-mid);
  border-radius: 16px;
  overflow: hidden;
}

/* Accent bars */
.card-accent-bar {
  height: 4px;
}

.soft-blue { background: #7ecef4; }
.soft-indigo { background: #a5b4fc; }
.soft-red { background: #f8a5a5; }
.soft-purple { background: #c3b6f7; }
.pastel-bar { background: linear-gradient(90deg,#e8a0b4,#b6d9f7); }

/* Card body */
.card-body {
  padding: 16px;
}

.center {
  text-align: center;
}

/* Trend */
.trend-title {
  font-size: 0.75rem;
  opacity: 0.7;
}

.trend-value {
  font-size: 1.4rem;
  font-weight: bold;
}

.trend-sub {
  font-size: 0.7rem;
  opacity: 0.6;
}

/* Risk */
.risk-pill {
  display: flex;
  gap: 10px;
  padding: 10px;
  border-radius: 12px;
  background: rgba(255,255,255,0.05);
  margin-bottom: 10px;
}

.risk-title {
  font-weight: 600;
}

.risk-sub {
  font-size: 0.7rem;
  opacity: 0.6;
}

/* Forecast */
.forecast-item {
  margin-bottom: 8px;
}

.recommendation {
  margin-top: 12px;
  font-size: 0.8rem;
  opacity: 0.7;
}

/* Stats pills */
.pills {
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
}

.stat-pill {
  background: rgba(255,255,255,0.06);
  padding: 8px 12px;
  border-radius: 999px;
  font-size: 0.8rem;
}

/* Responsive */
@media (max-width: 900px) {
  .trend-grid {
    grid-template-columns: repeat(2, 1fr);
  }
  .insight-grid,
  .stats-grid {
    grid-template-columns: 1fr;
  }
}
</style>