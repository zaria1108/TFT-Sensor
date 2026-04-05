import "vuetify/styles";
import "@mdi/font/css/materialdesignicons.css";
import { createVuetify }   from "vuetify";
import * as components     from "vuetify/components";
import * as directives     from "vuetify/directives";

const weatherDark = {
  dark: true,
  colors: {
    background : "#0d1b2e",
    surface    : "#1a2d45",
    "surface-2": "#203550",
    primary    : "#e8a0b4",   // rose
    secondary  : "#7ecef4",   // sky
    accent     : "#a5b4fc",   // violet
    success    : "#69e0a0",   // mint
    warning    : "#f5d478",   // amber
    error      : "#f87171",   // red
    info       : "#f4a46a",   // peach
  },
};

export default createVuetify({
  components,
  directives,
  theme: {
    defaultTheme: "weatherDark",
    themes: { weatherDark },
  },
  defaults: {
    VCard: {
      rounded: "xl",
      elevation: 0,
    },
    VBtn: {
      rounded: "pill",
      elevation: 0,
    },
    VTextField: {
      variant:  "outlined",
      density:  "compact",
      color:    "primary",
      hideDetails: "auto",
    },
    VChip: {
      rounded: "pill",
    },
  },
});