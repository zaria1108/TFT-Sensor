import { defineConfig }  from "vite";
import vue               from "@vitejs/plugin-vue";
import vuetify           from "vite-plugin-vuetify";
import Fonts             from "unplugin-fonts/vite";
import { fileURLToPath } from "url";

export default defineConfig({
  plugins: [
    vue(),

    // Auto-import Vuetify components & directives (tree-shakeable)
    vuetify({ autoImport: true }),

    // Load Roboto from Google Fonts
    Fonts({
      google: {
        families: [
          {
            name: "Roboto",
            styles: "wght@300;400;500;700",
          },
          {
            name: "Space Mono",
            styles: "wght@400;700",
          },
        ],
      },
    }),
  ],

  resolve: {
    alias: {
      "@": fileURLToPath(new URL("./src", import.meta.url)),
    },
  },

  server: {
    port: 3000,
    proxy: {
      // Forward all /api/* requests to the Flask backend during development
      "/api": {
        target:       "http://localhost:5000",
        changeOrigin: true,
      },
    },
  },
});