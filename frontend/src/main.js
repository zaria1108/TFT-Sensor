import { createApp }      from "vue";
import App                 from "./App.vue";
import router              from "./router/index.js";
import pinia               from "./stores/index.js";
import vuetify             from "./plugins/vuetify.js";
import HighchartsVue       from "highcharts-vue";
import Highcharts          from "highcharts";
import "@/styles/main.scss";

// ── Highcharts global theme ───────────────────────────────────────────────────
Highcharts.setOptions({
  chart: {
    backgroundColor: "transparent",
    style: { fontFamily: "'Roboto', sans-serif" },
  },
  title:    { style: { color: "#f0eae8" } },
  subtitle: { style: { color: "#8fa3b8" } },
  xAxis: {
    labels: { style: { color: "#8fa3b8" } },
    lineColor:   "#203550",
    tickColor:   "#203550",
    gridLineColor: "#1a2d45",
  },
  yAxis: {
    labels: { style: { color: "#8fa3b8" } },
    gridLineColor: "#203550",
    title: { style: { color: "#8fa3b8" } },
  },
  legend: {
    itemStyle:       { color: "#f0eae8" },
    itemHoverStyle:  { color: "#e8a0b4" },
  },
  tooltip: {
    backgroundColor: "#1a2d45",
    borderColor:     "#e8a0b4",
    style: { color: "#f0eae8" },
  },
  credits: { enabled: false },
});

createApp(App)
  .use(router)
  .use(pinia)
  .use(vuetify)
  .use(HighchartsVue)
  .mount("#app");