import { createRouter, createWebHistory } from "vue-router";

const routes = [
  {
    path: "/",
    name: "Dashboard",
    component: () => import("../views/DashboardView.vue"),
    meta: { title: "Live Dashboard" },
  },
  {
    path: "/analysis",
    name: "Analysis",
    component: () => import("../views/AnalysisView.vue"),
    meta: { title: "Trend Analysis" },
  },
  {
    path: "/graphs",
    name: "Graphs",
    component: () => import("../views/GraphsView.vue"),
    meta: { title: "Interactive Charts" },
  },
  {
    path: "/database",
    name: "Database",
    component: () => import("../views/ControlView.vue"),
    meta: { title: "Database Management" },
  },
  {
    path: "/simulation",
    name: "Simulation",
    component: () => import("../views/SimulationView.vue"),
    meta: { title: "Simulation Mode" },
  },
  {
    path: "/recommendations",
    name: "Recommendations",
    component: () => import("../views/RecommendationsView.vue"),
    meta: { title: "Smart Recommendations" },
  },
  {
    path: "/control",
    name: "Control",
    component: () => import("../views/ControlView.vue"),
    meta: { title: "Data Control" },
  },
];

const router = createRouter({
  history: createWebHistory(),
  routes,
});

export default router;
