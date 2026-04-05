<script setup>
import { computed, reactive, ref } from "vue";
import { useAppStore } from "@/stores/appStore.js";

const app = useAppStore();

const search = ref("");
const sortBy = ref("timestamp");
const itemsPerPage = ref(10);
const saving = ref(false);

const sortOptions = [
  { title: "Timestamp", value: "timestamp" },
  { title: "Temperature", value: "temperature" },
  { title: "Humidity", value: "humidity" },
  { title: "Pressure", value: "pressure" },
];

const headers = [
  { title: "Timestamp", key: "timestamp", sortable: true },
  { title: "Temperature (C)", key: "temperature", sortable: true },
  { title: "Humidity (%)", key: "humidity", sortable: true },
  { title: "Pressure (hPa)", key: "pressure", sortable: true },
  { title: "Soil Moisture (%)", key: "soil_moisture", sortable: true },
];

const newReading = reactive({
  temperature: 20,
  humidity: 50,
  pressure: 1013,
  bmp_temperature: 20,
  soil_moisture: 50,
});

const filteredReadings = computed(() => {
  const query = search.value.toLowerCase();
  return app.readings.filter((reading) => JSON.stringify(reading).toLowerCase().includes(query));
});

const formatDate = (timestamp) => {
  if (!timestamp) return "N/A";
  const date = new Date(timestamp.replace(" ", "T"));
  return Number.isNaN(date.getTime()) ? timestamp : date.toLocaleString();
};

const submitReading = async () => {
  saving.value = true;
  try {
    const response = await fetch("/api/create", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(newReading),
    });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    Object.assign(newReading, {
      temperature: 20,
      humidity: 50,
      pressure: 1013,
      bmp_temperature: 20,
      soil_moisture: 50,
    });
  } catch (err) {
    console.error(err);
    alert(`Error: ${err.message}`);
  } finally {
    saving.value = false;
  }
};
</script>

<template>
  <v-container fluid class="pa-6 db-page">

    <div class="page-header mb-8">
      <h1 class="page-title">🗄️ Historical Data</h1>
    </div>

    <!-- ── CREATE READING ── -->
    <v-card class="station-card create-card mb-7">
      <div class="card-accent-bar create-accent"></div>
      <v-card-title class="card-title create-title">✨ Log a New Reading</v-card-title>
      <v-card-text class="pt-3">
        <v-form @submit.prevent="submitReading">
          <v-row dense>

            <v-col cols="12" md="4">
              <div class="input-group">
                <span class="input-emoji">🌡️</span>
                <div class="input-inner temp-inner">
                  <label class="input-label">Temperature (°C)</label>
                  <input v-model.number="newReading.temperature" type="number" step="0.1" class="pastel-input" />
                </div>
              </div>
            </v-col>

            <v-col cols="12" md="4">
              <div class="input-group">
                <span class="input-emoji">💧</span>
                <div class="input-inner humidity-inner">
                  <label class="input-label">Humidity (%)</label>
                  <input v-model.number="newReading.humidity" type="number" step="0.1" class="pastel-input" />
                </div>
              </div>
            </v-col>

            <v-col cols="12" md="4">
              <div class="input-group">
                <span class="input-emoji">🌬️</span>
                <div class="input-inner pressure-inner">
                  <label class="input-label">Pressure (hPa)</label>
                  <input v-model.number="newReading.pressure" type="number" step="0.1" class="pastel-input" />
                </div>
              </div>
            </v-col>

            <v-col cols="12" md="4">
              <div class="input-group">
                <span class="input-emoji">🔬</span>
                <div class="input-inner bmp-inner">
                  <label class="input-label">BMP Temperature (°C)</label>
                  <input v-model.number="newReading.bmp_temperature" type="number" step="0.1" class="pastel-input" />
                </div>
              </div>
            </v-col>

            <v-col cols="12" md="4">
              <div class="input-group">
                <span class="input-emoji">🌱</span>
                <div class="input-inner soil-inner">
                  <label class="input-label">Soil Moisture (%)</label>
                  <input v-model.number="newReading.soil_moisture" type="number" step="0.1" class="pastel-input" />
                </div>
              </div>
            </v-col>

            <v-col cols="12" md="4" class="d-flex align-center">
              <button type="submit" class="submit-btn" :disabled="saving">
                <span v-if="!saving">✨ Save Reading</span>
                <span v-else>Saving…</span>
              </button>
            </v-col>

          </v-row>
        </v-form>
      </v-card-text>
    </v-card>

    <!-- ── FILTERS ── -->
    <v-card class="station-card filter-card mb-5">
      <v-card-text class="py-4">
        <v-row dense align="center">
          <v-col cols="12" md="6">
            <div class="input-group">
              <span class="input-emoji">🔍</span>
              <div class="input-inner search-inner" style="flex:1">
                <label class="input-label">Search readings…</label>
                <input v-model="search" type="text" class="pastel-input" />
              </div>
            </div>
          </v-col>
          <v-col cols="12" md="3">
            <div class="input-group">
              <span class="input-emoji">↕️</span>
              <div class="input-inner sort-inner" style="flex:1">
                <label class="input-label">Sort by</label>
                <select v-model="sortBy" class="pastel-input pastel-select">
                  <option v-for="o in sortOptions" :key="o.value" :value="o.value">{{ o.title }}</option>
                </select>
              </div>
            </div>
          </v-col>
          <v-col cols="12" md="3">
            <div class="input-group">
              <span class="input-emoji">📄</span>
              <div class="input-inner rows-inner" style="flex:1">
                <label class="input-label">Rows per page</label>
                <select v-model="itemsPerPage" class="pastel-input pastel-select">
                  <option v-for="n in [10,25,50,100]" :key="n" :value="n">{{ n }}</option>
                </select>
              </div>
            </div>
          </v-col>
        </v-row>
      </v-card-text>
    </v-card>

    <!-- ── TABLE ── -->
    <v-card class="station-card table-card">
      <div class="card-accent-bar table-accent"></div>
      <v-card-title class="card-title table-title">
        📊 Readings
        <v-chip class="ml-3 count-chip" size="small">{{ filteredReadings.length }} records</v-chip>
      </v-card-title>
      <v-card-text class="pa-0">
        <v-data-table
          :headers="headers"
          :items="filteredReadings"
          :items-per-page="itemsPerPage"
          :sort-by="[{ key: sortBy, order: 'desc' }]"
          class="db-table"
        >
          <template #item.timestamp="{ item }">
            <span class="timestamp-text">🕐 {{ formatDate(item.timestamp) }}</span>
          </template>
          <template #item.temperature="{ item }">
            <span :class="['data-pill', item.temperature > 28 ? 'pill-hot' : item.temperature < 15 ? 'pill-cold' : 'pill-ok']">
              {{ item.temperature > 28 ? '🔥' : item.temperature < 15 ? '🧊' : '✅' }} {{ item.temperature }} °C
            </span>
          </template>
          <template #item.humidity="{ item }">
            <span :class="['data-pill', item.humidity > 75 ? 'pill-humid' : item.humidity > 60 ? 'pill-mid' : 'pill-ok']">
              {{ item.humidity > 75 ? '🌊' : '💧' }} {{ item.humidity }}%
            </span>
          </template>
        </v-data-table>
      </v-card-text>
    </v-card>

  </v-container>
</template>

<style scoped>
.db-page { min-height: 100vh; background: transparent; }

/* ── Header ── */
.page-header { border-left: 4px solid #f9c6d0; padding-left: 1rem; }
.page-title {
  font-size: 1.75rem;
  font-weight: 700;
  color: #f9c6d0;
  letter-spacing: -0.3px;
  margin: 0;
}

/* ── Base Card ── */
.station-card {
  background: rgba(10, 28, 54, 0.88) !important;
  border-radius: 16px !important;
  border: 1px solid rgba(255,255,255,0.07) !important;
  overflow: hidden;
  transition: border-color 0.2s;
}
.station-card:hover { border-color: rgba(255,255,255,0.13) !important; }

/* ── Accent Bars ── */
.card-accent-bar { height: 4px; width: 100%; }
.create-accent { background: linear-gradient(90deg, #f9c6d0, #c3b6f7, #b6f0e4); }
.table-accent  { background: linear-gradient(90deg, #b6d9f7, #c3b6f7); }

/* ── Card Titles ── */
.card-title {
  font-size: 1rem !important;
  font-weight: 600 !important;
  display: flex;
  align-items: center;
  gap: 0.4rem;
  padding-top: 1rem !important;
}
.create-title { color: #f0d6ff !important; }
.table-title  { color: #b6d9f7 !important; }

/* ── Input Groups ── */
.input-group {
  display: flex;
  align-items: stretch;
  gap: 0.6rem;
  margin-bottom: 10px;
}
.input-emoji {
  font-size: 1.3rem;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  padding-top: 2px;
}
.input-inner {
  display: flex;
  flex-direction: column;
  justify-content: center;
  flex: 1;
  min-height: 52px;
  border-radius: 12px;
  padding: 8px 12px;
  border: 1.5px solid rgba(255,255,255,0.1);
  transition: border-color 0.2s, background 0.2s;
  box-sizing: border-box;
}
.input-inner:focus-within { border-color: rgba(249,198,208,0.5); }

.input-label {
  font-size: 0.68rem;
  font-weight: 600;
  letter-spacing: 0.4px;
  text-transform: uppercase;
  margin-bottom: 4px;
  line-height: 1;
  display: block;
}
.pastel-input {
  background: transparent;
  border: none;
  outline: none;
  width: 100%;
  font-size: 0.95rem;
  font-weight: 500;
  line-height: 1.2;
  display: block;
}
.pastel-select { cursor: pointer; }
.pastel-select option { background: #0d2240; color: #c8ddef; }

/* ── Per-field pastel colors ── */
.temp-inner     { background: rgba(255,218,185,0.12); border-color: rgba(255,218,185,0.3); }
.temp-inner .input-label, .temp-inner .pastel-input { color: #ffd4a3; }

.humidity-inner { background: rgba(182,217,247,0.1);  border-color: rgba(182,217,247,0.3); }
.humidity-inner .input-label, .humidity-inner .pastel-input { color: #b6d9f7; }

.pressure-inner { background: rgba(195,182,247,0.1);  border-color: rgba(195,182,247,0.3); }
.pressure-inner .input-label, .pressure-inner .pastel-input { color: #c3b6f7; }

.bmp-inner      { background: rgba(182,240,228,0.1);  border-color: rgba(182,240,228,0.3); }
.bmp-inner .input-label, .bmp-inner .pastel-input { color: #b6f0e4; }

.soil-inner     { background: rgba(198,240,182,0.1);  border-color: rgba(198,240,182,0.3); }
.soil-inner .input-label, .soil-inner .pastel-input { color: #c6f0b6; }

.search-inner   { background: rgba(249,198,208,0.08); border-color: rgba(249,198,208,0.25); }
.search-inner .input-label, .search-inner .pastel-input { color: #f9c6d0; }

.sort-inner     { background: rgba(195,182,247,0.08); border-color: rgba(195,182,247,0.25); }
.sort-inner .input-label, .sort-inner .pastel-input { color: #c3b6f7; }

.rows-inner     { background: rgba(182,217,247,0.08); border-color: rgba(182,217,247,0.25); }
.rows-inner .input-label, .rows-inner .pastel-input { color: #b6d9f7; }

/* ── Submit Button ── */
.submit-btn {
  width: 100%;
  min-height: 52px;
  border: none;
  border-radius: 12px;
  background: linear-gradient(135deg, #f9c6d0, #c3b6f7);
  color: #0a1c36;
  font-size: 0.95rem;
  font-weight: 700;
  cursor: pointer;
  letter-spacing: 0.3px;
  box-shadow: 0 4px 18px rgba(195,182,247,0.3);
  transition: opacity 0.2s, transform 0.15s;
}
.submit-btn:hover  { opacity: 0.9; transform: translateY(-1px); }
.submit-btn:active { transform: scale(0.98); }
.submit-btn:disabled { opacity: 0.5; cursor: not-allowed; }

/* ── Filter Card ── */
.filter-card { border-color: rgba(249,198,208,0.12) !important; }

/* ── Table ── */
.db-table { background: transparent !important; }
.db-table :deep(th) {
  background: rgba(255,255,255,0.04) !important;
  color: #8faecf !important;
  font-size: 0.75rem !important;
  font-weight: 600 !important;
  letter-spacing: 0.5px;
  text-transform: uppercase;
  border-bottom: 1px solid rgba(255,255,255,0.07) !important;
}
.db-table :deep(td) {
  color: #c8ddef !important;
  font-size: 0.875rem !important;
  border-bottom: 1px solid rgba(255,255,255,0.04) !important;
}
.db-table :deep(tr:hover td) { background: rgba(255,255,255,0.03) !important; }

.timestamp-text { color: #8faecf; font-size: 0.82rem; }

/* ── Data Pills ── */
.data-pill {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  padding: 3px 10px;
  border-radius: 999px;
  font-size: 0.78rem;
  font-weight: 600;
}
.pill-hot   { background: rgba(255,182,193,0.18); color: #ffb6c1; border: 1px solid rgba(255,182,193,0.35); }
.pill-cold  { background: rgba(182,217,247,0.18); color: #b6d9f7; border: 1px solid rgba(182,217,247,0.35); }
.pill-ok    { background: rgba(182,240,228,0.15); color: #b6f0e4; border: 1px solid rgba(182,240,228,0.3);  }
.pill-humid { background: rgba(195,182,247,0.18); color: #c3b6f7; border: 1px solid rgba(195,182,247,0.35); }
.pill-mid   { background: rgba(182,217,247,0.15); color: #b6d9f7; border: 1px solid rgba(182,217,247,0.3);  }

/* ── Count Chip ── */
.count-chip {
  background: rgba(195,182,247,0.15) !important;
  color: #c3b6f7 !important;
  border: 1px solid rgba(195,182,247,0.3) !important;
  font-size: 0.72rem !important;
  font-weight: 600 !important;
}
</style>