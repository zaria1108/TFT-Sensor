import { defineStore } from "pinia";
import { computed, ref } from "vue";

export const useAppStore = defineStore("app", () => {
  const latestReading = ref(null);
  const readings = ref([]);
  const count = ref(0);
  const loading = ref(false);
  const error = ref(null);
  const toast = ref({
    show: false,
    text: "",
    color: "success",
  });
  const systemStatus = ref({
    mqtt: {
      connected: false,
      last_message_at: null,
      messages_total: 0,
      last_error: null,
    },
    mongodb: {
      ok: false,
      error: null,
      total_readings: 0,
    },
  });
  const streamConnected = ref(false);

  let pollInterval = null;
  let eventSource = null;

  const totalReadings = computed(() => count.value);
  const mqttConnected = computed(() => systemStatus.value?.mqtt?.connected ?? false);
  const mqttMessageCount = computed(() => systemStatus.value?.mqtt?.messages_total ?? 0);

  async function fetchJson(url, options) {
    const response = await fetch(url, options);
    if (!response.ok) {
      if (response.status === 404) return null;
      throw new Error(`HTTP ${response.status}`);
    }
    return response.json();
  }

  function setError(err) {
    error.value = err?.message ?? String(err);
    console.warn("[App Store]", error.value);
  }

  function upsertReading(reading) {
    if (!reading?.id) return;
    const next = readings.value.filter((item) => item.id !== reading.id);
    next.unshift(reading);
    readings.value = next.slice(0, 100);
    latestReading.value = reading;
  }

  function removeReading(id) {
    readings.value = readings.value.filter((item) => item.id !== id);
    if (latestReading.value?.id === id) {
      latestReading.value = readings.value[0] ?? null;
    }
  }

  function applySnapshot(snapshot = {}) {
    if (snapshot.latestReading !== undefined) latestReading.value = snapshot.latestReading;
    if (Array.isArray(snapshot.readings)) readings.value = snapshot.readings;
    if (typeof snapshot.count === "number") count.value = snapshot.count;
    if (snapshot.mqtt) {
      systemStatus.value = {
        ...systemStatus.value,
        mqtt: snapshot.mqtt,
      };
    }
  }

  function applyMqttStatus(status = {}) {
    systemStatus.value = {
      ...systemStatus.value,
      mqtt: {
        ...systemStatus.value.mqtt,
        ...status,
      },
    };
  }

  const fetchLatest = async () => {
    try {
      loading.value = true;
      const data = await fetchJson("/api/latest");
      if (data) latestReading.value = data;
    } catch (err) {
      setError(err);
    } finally {
      loading.value = false;
    }
  };

  const fetchReadings = async () => {
    try {
      loading.value = true;
      const data = await fetchJson("/api/data");
      readings.value = Array.isArray(data) ? data : [];
    } catch (err) {
      setError(err);
    } finally {
      loading.value = false;
    }
  };

  const fetchCount = async () => {
    try {
      const data = await fetchJson("/api/count");
      count.value = data?.count || data?.total_readings || 0;
    } catch (err) {
      setError(err);
    }
  };

  const fetchStatus = async () => {
    try {
      const data = await fetchJson("/api/status");
      if (data) systemStatus.value = data;
      return data;
    } catch (err) {
      setError(err);
      return null;
    }
  };

  const refreshAll = async () => {
    await Promise.all([fetchLatest(), fetchReadings(), fetchCount(), fetchStatus()]);
  };

  function connectStream() {
    if (eventSource) return;

    eventSource = new EventSource("/api/stream");

    eventSource.onopen = () => {
      streamConnected.value = true;
      error.value = null;
    };

    eventSource.onerror = () => {
      streamConnected.value = false;
    };

    eventSource.addEventListener("snapshot", (event) => {
      try {
        applySnapshot(JSON.parse(event.data));
      } catch (err) {
        setError(err);
      }
    });

    eventSource.addEventListener("reading_created", (event) => {
      try {
        const payload = JSON.parse(event.data);
        upsertReading(payload.reading);
        count.value = payload.count ?? count.value + 1;
      } catch (err) {
        setError(err);
      }
    });

    eventSource.addEventListener("reading_updated", (event) => {
      try {
        const payload = JSON.parse(event.data);
        upsertReading(payload.reading);
      } catch (err) {
        setError(err);
      }
    });

    eventSource.addEventListener("reading_deleted", (event) => {
      try {
        const payload = JSON.parse(event.data);
        removeReading(payload.id);
        count.value = payload.count ?? Math.max(0, count.value - 1);
      } catch (err) {
        setError(err);
      }
    });

    eventSource.addEventListener("mqtt_status", (event) => {
      try {
        applyMqttStatus(JSON.parse(event.data));
      } catch (err) {
        setError(err);
      }
    });
  }

  function disconnectStream() {
    if (!eventSource) return;
    eventSource.close();
    eventSource = null;
    streamConnected.value = false;
  }

  const startPolling = async (interval = 10000) => {
    if (pollInterval) clearInterval(pollInterval);
    await refreshAll();
    connectStream();

    pollInterval = setInterval(async () => {
      await fetchStatus();
      if (!streamConnected.value) {
        await Promise.all([fetchLatest(), fetchReadings(), fetchCount()]);
      }
    }, interval);
  };

  const stopPolling = () => {
    if (pollInterval) {
      clearInterval(pollInterval);
      pollInterval = null;
    }
    disconnectStream();
  };

  return {
    latestReading,
    readings,
    count,
    loading,
    error,
    toast,
    systemStatus,
    streamConnected,
    totalReadings,
    mqttConnected,
    mqttMessageCount,
    fetchLatest,
    fetchReadings,
    fetchCount,
    fetchStatus,
    refreshAll,
    startPolling,
    stopPolling,
    applySnapshot,
  };
});
