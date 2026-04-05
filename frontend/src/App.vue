<script setup>
import { computed, ref, onMounted, onUnmounted } from "vue";
import { useRoute } from "vue-router";

import { useAppStore } from "@/stores/appStore.js";

const route = useRoute();
const app = useAppStore();
const drawer = ref(false);

const navItems = [
  { title: "Dashboard", icon: "mdi-dashboard", to: "/" },
  { title: "Analysis", icon: "mdi-chart-line", to: "/analysis" },
  { title: "Graphs", icon: "mdi-chart-box-outline", to: "/graphs" },
  { title: "Database", icon: "mdi-database", to: "/database" },
  { title: "Simulation", icon: "mdi-play-circle", to: "/simulation" },
  { title: "Recommendations", icon: "mdi-lightbulb", to: "/recommendations" },
];

const apiHealthy = computed(() => app.systemStatus?.mongodb?.ok ?? false);
const mqttHealthy = computed(() => app.mqttConnected);

const isActive = (to) => route.path === to;

onMounted(() => {
  app.startPolling();
});

onUnmounted(() => {
  app.stopPolling();
});
</script>

<template>
  <v-app>
    <v-app-bar flat class="navbar-bar">
      <v-app-bar-nav-icon
        @click="drawer = !drawer"
        icon="mdi-menu"
        class="d-sm-none"
      />

      <!-- TITLE -->
      <div class="navbar-title">
        <router-link to="/" class="brand">
        <span class="logo">🌦️</span>
        Clima-Core
        </router-link>
      </div>

      <!-- NAV -->
      <div class="nav-container d-none d-sm-flex">
        <v-btn
          v-for="item in navItems"
          :key="item.to"
          :to="item.to"
          variant="text"
          size="small"
          class="navbar-link"
          :class="{ active: isActive(item.to) }"
          :prepend-icon="item.icon"
          :ripple="false"
        >
          {{ item.title }}
        </v-btn>
      </div>

      <!-- STATUS (FIXED) -->
      <div class="status-container d-none d-sm-flex">
        <v-chip size="small" variant="tonal" :color="apiHealthy ? 'success' : 'error'">
          API {{ apiHealthy ? "online" : "offline" }}
        </v-chip>

        <v-chip size="small" variant="tonal" :color="mqttHealthy ? 'success' : 'warning'">
          MQTT {{ mqttHealthy ? "live" : "waiting" }}
        </v-chip>
      </div>
    </v-app-bar>

    <v-navigation-drawer v-model="drawer" temporary location="left">
      <v-list-item
        prepend-icon="mdi-weather-cloudy"
        title="Azaria's Weather Station"
        subtitle="Navigation"
        class="mb-3"
      />

      <v-divider />

      <v-list>
        <v-list-item
          v-for="item in navItems"
          :key="item.to"
          :to="item.to"
          :prepend-icon="item.icon"
          :title="item.title"
          @click="drawer = false"
          :active="isActive(item.to)"
        />
      </v-list>
    </v-navigation-drawer>

    <v-main>
      <v-container>
        <router-view />
      </v-container>
    </v-main>
  </v-app>
</template>

<style scoped>
@import url('https://fonts.googleapis.com/css2?family=Outfit:wght@400;500;700&display=swap');

.navbar-bar {
  display: flex;
  align-items: center;
  padding: 0 16px;
  background: linear-gradient(90deg, #0d1b2e 0%, #1a2d45 100%);
  border-bottom: 1px solid rgba(232,160,180,.12);
  font-family: 'Outfit', sans-serif;
}

/* TITLE */
.navbar-title {
  flex: 0 0 auto;
  margin-right: 24px;
}

.brand {
  color: #e8a0b4;
  text-decoration: none;
  white-space: nowrap;
  font-weight: 700;
  font-size: 1.1rem;
}

/* NAV */
.nav-container {
  display: flex;
  gap: 1rem;
  flex: 1;
}

/* STATUS FIX */
.status-container {
  display: flex;
  gap: 0.5rem;
  flex: 0 0 auto; /* 🔥 prevents shrinking */
  margin-left: 16px;
}

/* LINKS */
.navbar-link {
  color: #8fa3b8 !important;
  font-weight: 500;
  text-transform: none;
  font-size: 0.9rem;
  background: transparent !important;
}

/* REMOVE VUETIFY ACTIVE BG */
:deep(.navbar-link.v-btn--active) {
  background-color: transparent !important;
  box-shadow: none !important;
}

/* REMOVE OVERLAY */
:deep(.navbar-link .v-btn__overlay),
:deep(.navbar-link .v-btn__underlay),
:deep(.navbar-link .v-ripple__container) {
  display: none !important;
}

/* ACTIVE */
.navbar-link.active {
  color: #e8a0b4 !important;
  border-bottom: 2px solid #e8a0b4;
}

/* HOVER */
.navbar-link:hover {
  color: #e8a0b4 !important;
}

.gap-2 {
  gap: 1rem;
}
</style>
