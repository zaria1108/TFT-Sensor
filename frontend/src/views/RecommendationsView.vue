<script setup>
import { computed } from "vue";

import { useAppStore } from "@/stores/appStore.js";
import { buildRecommendations } from "@/utils/weatherData.js";

const app = useAppStore();
const recommendations = computed(() => buildRecommendations(app.latestReading ?? {}, app.readings));
</script>

<template>
  <v-container fluid class="pa-6 recommendations-page">

    <div class="page-header mb-8">
      <h1 class="page-title">🌤️ Smart Recommendations</h1>
    </div>

    <!-- Critical Alerts -->
    <v-row class="mb-5" v-if="recommendations.critical.length > 0">
      <v-col cols="12">
        <v-card class="station-card critical-card">
          <v-card-title class="card-title critical-title">
            <span class="title-icon">🚨</span>
            Critical Actions Required
          </v-card-title>
          <v-card-text class="pt-2">
            <ul class="styled-list">
              <li v-for="(item, index) in recommendations.critical" :key="`critical-${index}`">
                <span class="bullet critical-bullet">●</span>{{ item }}
              </li>
            </ul>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- Recommended Actions -->
    <v-row class="mb-5" v-if="recommendations.warning.length > 0">
      <v-col cols="12">
        <v-card class="station-card warning-card">
          <v-card-title class="card-title warning-title">
            <span class="title-icon">⚠️</span>
            Recommended Actions
          </v-card-title>
          <v-card-text class="pt-2">
            <v-timeline side="start" density="compact">
              <v-timeline-item
                v-for="(item, index) in recommendations.warning"
                :key="`warning-${index}`"
                dot-color="#f9c6d0"
                size="small"
              >
                <span class="timeline-text">{{ item }}</span>
              </v-timeline-item>
            </v-timeline>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- Tips -->
    <v-row class="mb-6" v-if="recommendations.info.length > 0">
      <v-col cols="12">
        <v-card class="station-card info-card">
          <v-card-title class="card-title info-title">
            <span class="title-icon">💡</span>
            Additional Tips
          </v-card-title>
          <v-card-text class="pt-2">
            <ul class="styled-list">
              <li v-for="(item, index) in recommendations.info" :key="`info-${index}`">
                <span class="bullet info-bullet">●</span>{{ item }}
              </li>
            </ul>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- Reference Cards -->
    <v-row>
      <v-col cols="12" md="6">
        <v-card class="station-card ref-card temp-ref-card">
          <div class="ref-card-accent temp-accent"></div>
          <v-card-title class="card-title ref-title temp-ref-title">
            🌡️ Temperature Management
          </v-card-title>
          <v-card-text class="pt-3">
            <div class="range-chip temp-chip mb-4">
              <span class="chip-label">Ideal Range</span>
              <span class="chip-value">18 – 25 °C</span>
            </div>
            <p class="actions-label mb-2">Actions</p>
            <ul class="styled-list">
              <li><span class="bullet temp-bullet">●</span>Open vents if temperature climbs above 28 °C.</li>
              <li><span class="bullet temp-bullet">●</span>Close openings if temperature drops below 15 °C.</li>
              <li><span class="bullet temp-bullet">●</span>Use shade during high midday heat.</li>
            </ul>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6">
        <v-card class="station-card ref-card humidity-ref-card">
          <div class="ref-card-accent humidity-accent"></div>
          <v-card-title class="card-title ref-title humidity-ref-title">
            💧 Humidity Control
          </v-card-title>
          <v-card-text class="pt-3">
            <div class="range-chip humidity-chip mb-4">
              <span class="chip-label">Ideal Range</span>
              <span class="chip-value">40 – 70%</span>
            </div>
            <p class="actions-label mb-2">Actions</p>
            <ul class="styled-list">
              <li><span class="bullet humidity-bullet">●</span>Increase ventilation if humidity moves above 75%.</li>
              <li><span class="bullet humidity-bullet">●</span>Retain moisture if humidity falls below 30%.</li>
              <li><span class="bullet humidity-bullet">●</span>Watch for condensation after sharp pressure drops.</li>
            </ul>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

  </v-container>
</template>

<style scoped>
.recommendations-page {
  background: transparent;
  min-height: 100vh;
}

/* ── Page Header ── */
.page-header {
  border-left: 4px solid #f9c6d0;
  padding-left: 1rem;
}

.page-title {
  font-size: 1.75rem;
  font-weight: 700;
  color: #f9c6d0;
  letter-spacing: -0.3px;
  margin: 0;
}

/* ── Base Card ── */
.station-card {
  background: rgba(10, 28, 54, 0.85) !important;
  border-radius: 16px !important;
  border: 1px solid rgba(255,255,255,0.07) !important;
  backdrop-filter: blur(4px);
  overflow: hidden;
  transition: border-color 0.2s ease;
}
.station-card:hover {
  border-color: rgba(255,255,255,0.15) !important;
}

/* ── Card Titles ── */
.card-title {
  font-size: 0.95rem !important;
  font-weight: 600 !important;
  letter-spacing: 0.2px;
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding-bottom: 0.25rem;
}
.title-icon { font-size: 1rem; }

/* ── Critical ── */
.critical-card {
  border-color: rgba(255, 182, 193, 0.4) !important;
  background: rgba(35, 10, 22, 0.92) !important;
  box-shadow: 0 0 0 1px rgba(255, 182, 193, 0.15) inset;
}
.critical-title { color: #ffb6c1; }
.critical-bullet { color: #ffb6c1; }

/* ── Warning ── */
.warning-card {
  border-color: rgba(255, 218, 185, 0.35) !important;
  background: rgba(28, 18, 10, 0.88) !important;
}
.warning-title { color: #ffdab9; }
.timeline-text {
  color: #cce0f0;
  font-size: 0.9rem;
}

/* ── Info ── */
.info-card {
  border-color: rgba(182, 240, 228, 0.3) !important;
  background: rgba(8, 28, 30, 0.88) !important;
}
.info-title { color: #b6f0e4; }
.info-bullet { color: #b6f0e4; }

/* ── Reference Cards — top accent bar ── */
.ref-card-accent {
  height: 4px;
  width: 100%;
}
.temp-accent {
  background: linear-gradient(90deg, #ffd4a3 0%, #ffb6c1 100%);
}
.humidity-accent {
  background: linear-gradient(90deg, #b6d9f7 0%, #c3b6f7 100%);
}

.temp-ref-card {
  border-color: rgba(255, 212, 163, 0.3) !important;
}
.humidity-ref-card {
  border-color: rgba(182, 217, 247, 0.3) !important;
}

.temp-ref-title  { color: #ffd4a3 !important; font-size: 1rem !important; }
.humidity-ref-title { color: #b6d9f7 !important; font-size: 1rem !important; }

/* ── Range Chips ── */
.range-chip {
  display: inline-flex;
  align-items: center;
  gap: 0.625rem;
  padding: 0.4rem 0.9rem;
  border-radius: 999px;
  font-size: 0.82rem;
  font-weight: 500;
}
.chip-label { opacity: 0.65; }
.chip-value { font-weight: 700; }

.temp-chip {
  background: rgba(255, 212, 163, 0.12);
  border: 1px solid rgba(255, 212, 163, 0.35);
  color: #ffd4a3;
}
.temp-bullet { color: #ffd4a3; }

.humidity-chip {
  background: rgba(182, 217, 247, 0.1);
  border: 1px solid rgba(182, 217, 247, 0.3);
  color: #b6d9f7;
}
.humidity-bullet { color: #b6d9f7; }

/* ── Lists ── */
.styled-list {
  list-style: none;
  padding: 0;
  margin: 0;
}
.styled-list li {
  display: flex;
  align-items: baseline;
  gap: 0.5rem;
  margin-bottom: 0.6rem;
  color: #c8ddef;
  font-size: 0.9rem;
  line-height: 1.5;
}
.bullet {
  font-size: 0.4rem;
  flex-shrink: 0;
  position: relative;
  top: -1px;
}

/* ── Actions Label ── */
.actions-label {
  font-size: 0.75rem;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.8px;
  color: #8faecf;
  margin: 0;
}
</style>