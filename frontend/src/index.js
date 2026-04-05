import { createRouter, createWebHistory } from "vue-router";

const routes = [
  {
    path: "/",
    name: "home",
    component: () => import("@/views/Home.vue"),
    meta: { title: "Home" },
  },
  {
    path: "/dashboard",
    name: "dashboard",
    component: () => import("@/views/Dashboard.vue"),
    meta: { title: "Live Dashboard" },
  },
  {
    path: "/analysis",
    name: "analysis",
    component: () => import("@/views/Analysis.vue"),
    meta: { title: "Analysis" },
  },
  {
    path: "/control",
    name: "control",
    component: () => import("@/views/Control.vue"),
    meta: { title: "Data Control" },
  },
];

const router = createRouter({
  history: createWebHistory(),
  routes,
});

// Update document title on navigation
router.afterEach((to) => {
  document.title = to.meta.title
    ? `${to.meta.title} · Azaria's Weather Station`
    : "Azaria's Weather Station";
});

export default router;