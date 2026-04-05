import { createVuetify } from "vuetify";
import * as components from "vuetify/components";
import * as directives from "vuetify/directives";

const vuetify = createVuetify({
  components,
  directives,
  theme: {
    defaultTheme: "dark",
    themes: {
      dark: {
        colors: {
          primary: "#e8a0b4",
          secondary: "#8fa3b8",
          accent: "#7ecef4",
          background: "#0d1b2e",
          surface: "#1a2d45",
        },
      },
    },
  },
});

export default vuetify;
