import { defineStore } from "pinia";
import { computed } from "vue";

import { useAppStore } from "./appStore.js";

export const useMqttStore = defineStore("mqtt", () => {
  const app = useAppStore();

  const connected = computed(() => app.mqttConnected);
  const status = computed(() => app.systemStatus.mqtt);
  const error = computed(() => app.error);
  const messageCount = computed(() => app.mqttMessageCount);
  const live = computed(() => app.latestReading ?? {});
  const history = computed(() => [...app.readings].reverse().map((reading) => ({
    ...reading,
    ts: reading.timestamp,
  })));

  const connect = async () => {
    await app.startPolling();
  };

  const disconnect = () => {
    app.stopPolling();
  };

  const updateLiveData = () => {};

  return {
    connected,
    status,
    error,
    messageCount,
    live,
    history,
    connect,
    disconnect,
    updateLiveData,
  };
});
