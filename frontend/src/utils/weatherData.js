const SENSOR_KEYS = [
  "temperature",
  "humidity",
  "pressure",
  "bmp_temperature",
  "soil_moisture",
];

export function toNumber(value) {
  if (value == null || value === "") return null;
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : null;
}

export function parseTimestamp(value) {
  if (!value) return null;
  const normalized = typeof value === "string" ? value.replace(" ", "T") : value;
  const date = new Date(normalized);
  return Number.isNaN(date.getTime()) ? null : date;
}

export function formatTimestamp(value) {
  const date = parseTimestamp(value);
  return date ? date.toLocaleString() : "--";
}

export function chronological(readings = []) {
  return [...readings]
    .filter((reading) => reading?.timestamp)
    .sort((a, b) => {
      const aTime = parseTimestamp(a.timestamp)?.getTime() ?? 0;
      const bTime = parseTimestamp(b.timestamp)?.getTime() ?? 0;
      return aTime - bTime;
    });
}

export function calculateStats(readings = [], key) {
  const values = chronological(readings)
    .map((reading) => toNumber(reading[key]))
    .filter((value) => value != null);

  if (!values.length) {
    return {
      min: null,
      max: null,
      avg: null,
      change: null,
      current: null,
    };
  }

  const min = Math.min(...values);
  const max = Math.max(...values);
  const avg = values.reduce((sum, value) => sum + value, 0) / values.length;
  const current = values[values.length - 1];
  const change = values.length > 1 ? current - values[0] : 0;

  return { min, max, avg, change, current };
}

export function formatDelta(value, unit = "", decimals = 1) {
  if (value == null) return "--";
  const sign = value > 0 ? "+" : "";
  return `${sign}${value.toFixed(decimals)}${unit}`;
}

export function trendLabel(change) {
  if (change == null) return "No data";
  if (change > 0.2) return "Rising";
  if (change < -0.2) return "Falling";
  return "Stable";
}

export function buildRisks(latest = {}) {
  const risks = [];
  const humidity = toNumber(latest.humidity);
  const temperature = toNumber(latest.temperature);
  const soil = toNumber(latest.soil_moisture);
  const pressure = toNumber(latest.pressure);

  if (humidity != null && humidity >= 75) {
    risks.push({
      icon: "mdi-water-opacity",
      title: "High Humidity Risk",
      subtitle: `${humidity.toFixed(1)}% - mold growth possible`,
    });
  }

  if (temperature != null && temperature >= 30) {
    risks.push({
      icon: "mdi-thermometer-high",
      title: "Heat Stress Alert",
      subtitle: `${temperature.toFixed(1)} C - consider cooling or shade`,
    });
  }

  if (soil != null && soil <= 30) {
    risks.push({
      icon: "mdi-water-alert",
      title: "Dry Soil",
      subtitle: `${soil.toFixed(0)}% - watering recommended`,
    });
  }

  if (pressure != null && pressure <= 1005) {
    risks.push({
      icon: "mdi-weather-pouring",
      title: "Rain Shift Detected",
      subtitle: `${pressure.toFixed(0)} hPa - pressure is low`,
    });
  }

  return risks;
}

export function buildRecommendations(latest = {}, readings = []) {
  const temperature = toNumber(latest.temperature);
  const humidity = toNumber(latest.humidity);
  const soil = toNumber(latest.soil_moisture);
  const pressureStats = calculateStats(readings, "pressure");
  const tempStats = calculateStats(readings, "temperature");

  const recommendations = {
    critical: [],
    warning: [],
    info: [],
  };

  if (soil != null && soil <= 20) {
    recommendations.critical.push(`Soil moisture is critically low at ${soil.toFixed(0)}%. Water immediately.`);
  }

  if (temperature != null && temperature >= 30) {
    recommendations.warning.push(`Temperature is elevated at ${temperature.toFixed(1)} C. Improve airflow or add shade.`);
  }

  if (humidity != null && humidity >= 75) {
    recommendations.warning.push(`Humidity is high at ${humidity.toFixed(1)}%. Increase ventilation to reduce mold risk.`);
  }

  if ((pressureStats.change ?? 0) <= -3) {
    recommendations.warning.push("Pressure is dropping quickly. Prepare for rain or higher ambient moisture.");
  }

  if (soil != null && soil > 20 && soil <= 35) {
    recommendations.warning.push(`Soil moisture is ${soil.toFixed(0)}%. Plan a watering cycle soon.`);
  }

  if (Math.abs(tempStats.change ?? 0) < 1) {
    recommendations.info.push("Temperature has stayed fairly stable across the recent readings.");
  }

  if (humidity != null && humidity >= 40 && humidity <= 70) {
    recommendations.info.push("Humidity is in a healthy operating range.");
  }

  if (soil != null && soil >= 40 && soil <= 70) {
    recommendations.info.push("Soil moisture looks balanced right now.");
  }

  if (!recommendations.critical.length && !recommendations.warning.length && !recommendations.info.length) {
    recommendations.info.push("No action is needed yet. Keep monitoring incoming readings.");
  }

  return recommendations;
}

export function chartSeries(readings = [], key) {
  return chronological(readings).map((reading) => ({
    x: formatTimestamp(reading.timestamp),
    y: toNumber(reading[key]),
  }));
}

export function latestSensorSnapshot(latest = {}) {
  return SENSOR_KEYS.reduce((snapshot, key) => {
    snapshot[key] = toNumber(latest[key]);
    return snapshot;
  }, {});
}
