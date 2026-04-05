<script setup>
import { computed } from "vue";
import { useAppStore } from "@/stores/appStore.js";
import { chronological, toNumber } from "@/utils/weatherData.js";

const app = useAppStore();

const sensors = [
  { key: "temperature", label: "Temperature", unit: "°C", emoji: "🌡️", color: "#7ecef4", gradient: "linear-gradient(90deg,#7ecef4,#b6d9f7)" },
  { key: "soil_moisture", label: "Soil Moisture", unit: "%", emoji: "🌱", color: "#69e0a0", gradient: "linear-gradient(90deg,#b6f0e4,#69e0a0)" },
  { key: "humidity", label: "Humidity", unit: "%", emoji: "💧", color: "#a5b4fc", gradient: "linear-gradient(90deg,#a5b4fc,#c3b6f7)" },
  { key: "pressure", label: "Pressure", unit: "hPa", emoji: "🌬️", color: "#f5d478", gradient: "linear-gradient(90deg,#f5d478,#ffd4a3)" },
];

const filteredReadings = computed(() => chronological(app.readings));

// 🔥 UPDATED: time-only formatting
const formatTime = (timestamp) => {
  const d = new Date(timestamp);
  return d.toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
  });
};

const getChartOptions = (sensor) => {
  const categories = filteredReadings.value.map((r) =>
    formatTime(r.timestamp)
  );

  return {
    chart: {
      type: "line",
      height: 300,
      backgroundColor: "transparent",
    },
    title: { text: "" },
    xAxis: {
      categories,
      labels: {
        rotation: -30,
        style: { fontSize: "0.65rem" },
      },
    },
    yAxis: {
      title: { text: sensor.unit },
    },
    legend: { enabled: false },
    tooltip: {
      shared: true,
    },
    series: [
      {
        name: sensor.label,
        data: filteredReadings.value.map((r) =>
          toNumber(r[sensor.key])
        ),
        color: sensor.color,
      },
    ],
  };
};
</script>

<template>
  <div class="charts-page">
    <div class="page-header">
      <h1 class="page-title">📊 Interactive Charts</h1>
    </div>

    <div class="charts-grid">
      <div class="station-card" v-for="s in sensors" :key="s.key">
        <div class="card-accent-bar" :style="{ background: s.gradient }"></div>

        <div class="card-header">
          <div class="card-title" :style="{ color: s.color }">
            <span class="title-dot" :style="{ background: s.color }"></span>
            {{ s.emoji }} {{ s.label }}
            <span class="unit-label">{{ s.unit }}</span>
          </div>

          <div class="live-badge">
            <span class="live-dot"></span> live
          </div>
        </div>

        <div class="card-body">
          <highcharts
            v-if="filteredReadings.length"
            :options="getChartOptions(s)"
          />
          <div v-else class="empty-state">
            <p>No live data yet...</p>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.charts-page {
  padding: 24px;
}

.page-title {
  color: var(--rose);
}

.charts-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 20px;
}

@media (max-width: 900px) {
  .charts-grid {
    grid-template-columns: 1fr;
  }
}

.station-card {
  background: var(--navy-mid);
  border-radius: 16px;
  overflow: hidden;
}

.card-accent-bar {
  height: 4px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 16px;
}

.card-title {
  font-weight: 600;
  display: flex;
  align-items: center;
  gap: 6px;
}

.title-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}

.unit-label {
  font-size: 0.7rem;
  opacity: 0.7;
  margin-left: 4px;
}

.live-badge {
  font-size: 0.75rem;
  color: #9ca3af;
  display: flex;
  align-items: center;
  gap: 5px;
}

.live-dot {
  width: 8px;
  height: 8px;
  background: #22c55e;
  border-radius: 50%;
  animation: pulse 1.2s infinite;
}

.card-body {
  padding: 10px 12px 16px;
}

.empty-state {
  height: 250px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--slate);
}

@keyframes pulse {
  0% {
    transform: scale(0.9);
    opacity: 0.7;
  }
  70% {
    transform: scale(1.3);
    opacity: 0;
  }
  100% {
    transform: scale(0.9);
    opacity: 0;
  }
}
</style>