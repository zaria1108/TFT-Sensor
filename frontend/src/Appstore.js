import { defineStore } from "pinia";
import { ref, computed } from "vue";

export const useAppStore = defineStore(
  "app",
  () => {
    // ── State ──────────────────────────────────────────────────────────────
    const readings      = ref([]);       // last 100 readings from /api/data
    const latestReading = ref(null);     // most recent reading
    const totalReadings = ref(0);
    const loading       = ref(false);
    const toast         = ref({ show: false, text: "", color: "success" });

    // ── Getters ────────────────────────────────────────────────────────────
    const hasData = computed(() => readings.value.length > 0);

    // ── Toast helper ───────────────────────────────────────────────────────
    function notify(text, color = "success") {
      toast.value = { show: true, text, color };
    }

    // ── API helpers ────────────────────────────────────────────────────────
    async function _fetch(url, options = {}) {
      const res = await fetch(url, options);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return res.json();
    }

    // ── CRUD actions ───────────────────────────────────────────────────────
    async function fetchReadings() {
      loading.value = true;
      try {
        readings.value = await _fetch("/api/data");
      } catch (e) {
        notify(`Failed to load readings: ${e.message}`, "error");
      } finally {
        loading.value = false;
      }
    }

    async function fetchLatest() {
      try {
        const data = await _fetch("/api/latest");
        if (!data.error) latestReading.value = data;
      } catch {}
    }

    async function fetchCount() {
      try {
        const d = await _fetch("/api/count");
        totalReadings.value = d.total_readings ?? 0;
      } catch {}
    }

    async function createReading(payload) {
      try {
        const data = await _fetch("/api/create", {
          method:  "POST",
          headers: { "Content-Type": "application/json" },
          body:    JSON.stringify(payload),
        });
        if (data.success) {
          notify("✅ Reading added");
          await Promise.all([fetchReadings(), fetchLatest(), fetchCount()]);
          return true;
        }
        notify(`❌ ${data.error}`, "error");
      } catch (e) {
        notify(`❌ ${e.message}`, "error");
      }
      return false;
    }

    async function updateReading(id, payload) {
      try {
        const data = await _fetch(`/api/update/${id}`, {
          method:  "PUT",
          headers: { "Content-Type": "application/json" },
          body:    JSON.stringify(payload),
        });
        if (data.success) {
          notify("✅ Reading updated");
          await fetchReadings();
          return true;
        }
        notify(`❌ ${data.error}`, "error");
      } catch (e) {
        notify(`❌ ${e.message}`, "error");
      }
      return false;
    }

    async function deleteReading(id) {
      try {
        const data = await _fetch(`/api/delete/${id}`, { method: "DELETE" });
        if (data.success) {
          notify("🗑️ Reading deleted", "warning");
          await Promise.all([fetchReadings(), fetchCount()]);
          return true;
        }
        notify(`❌ ${data.error}`, "error");
      } catch (e) {
        notify(`❌ ${e.message}`, "error");
      }
      return false;
    }

    async function fetchStatus() {
      try {
        return await _fetch("/api/status");
      } catch {
        return null;
      }
    }

    return {
      readings, latestReading, totalReadings, loading, toast, hasData,
      notify, fetchReadings, fetchLatest, fetchCount,
      createReading, updateReading, deleteReading, fetchStatus,
    };
  },
  {
    persist: {
      // Only persist non-volatile state
      pick: ["totalReadings"],
    },
  }
);