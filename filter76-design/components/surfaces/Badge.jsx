import React from "react";

/**
 * Badge — a small status / count pill for faceplate metadata (preset
 * number, "4x", "ON", "MOD"). Tones: "amber" (filled accent), "outline"
 * (hairline), "display" (dark amber-text well).
 */
export function Badge({
  children,
  tone = "outline",   // "amber" | "outline" | "display" | "warning"
  className = "",
  style = {},
  ...rest
}) {
  const tones = {
    amber:   { bg: "var(--accent-amber)", fg: "var(--text-on-accent)", bd: "1px solid var(--accent-amber-active)" },
    outline: { bg: "transparent", fg: "var(--text-secondary)", bd: "var(--border-default)" },
    display: { bg: "var(--surface-display)", fg: "var(--text-on-display)", bd: "1px solid rgba(0,0,0,0.3)" },
    warning: { bg: "var(--warning)", fg: "#fff", bd: "1px solid rgba(0,0,0,0.18)" },
  };
  const t = tones[tone] || tones.outline;
  return (
    <span
      className={`af-badge af-badge--${tone} ${className}`}
      style={{
        display: "inline-flex",
        alignItems: "center",
        gap: "5px",
        height: "20px",
        padding: "0 8px",
        fontFamily: "var(--font-mono)",
        fontSize: "var(--fs-10)",
        fontWeight: "var(--fw-medium)",
        letterSpacing: "var(--tracking-label)",
        textTransform: "uppercase",
        color: t.fg,
        background: t.bg,
        border: t.bd,
        borderRadius: "var(--radius-pill)",
        whiteSpace: "nowrap",
        ...style,
      }}
      {...rest}
    >
      {children}
    </span>
  );
}
