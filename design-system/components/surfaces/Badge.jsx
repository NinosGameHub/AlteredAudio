import React from "react";

/**
 * Badge — small status / count pill.
 * tone: "neutral" (grey), "accent" (blue tint), "active" (green tint),
 * "muted" (sunken). Uppercase, tracked, tiny — matches the UI's label voice.
 */
export function Badge({
  children,
  tone = "neutral",
  dot = false,
  className = "",
  ...rest
}) {
  const tones = {
    neutral: { bg: "var(--surface-sunken)", fg: "var(--text-muted)", dot: "var(--text-muted)" },
    accent: { bg: "var(--accent-tint)", fg: "var(--accent-press)", dot: "var(--accent-primary)" },
    active: { bg: "rgba(48,209,88,0.14)", fg: "#1B7F38", dot: "var(--state-active)" },
    muted: { bg: "var(--surface-sunken)", fg: "var(--text-disabled)", dot: "var(--state-inactive)" },
  };
  const t = tones[tone] || tones.neutral;

  return (
    <span
      className={`aa-badge aa-badge--${tone} ${className}`}
      style={{
        display: "inline-flex",
        alignItems: "center",
        gap: "5px",
        height: "18px",
        padding: "0 8px",
        borderRadius: "var(--radius-pill)",
        background: t.bg,
        color: t.fg,
        fontSize: "var(--fs-10)",
        fontWeight: "var(--fw-semibold)",
        letterSpacing: "var(--tracking-label)",
        textTransform: "uppercase",
        whiteSpace: "nowrap",
      }}
      {...rest}
    >
      {dot ? (
        <span
          style={{
            width: "6px",
            height: "6px",
            borderRadius: "var(--radius-pill)",
            background: t.dot,
          }}
        />
      ) : null}
      {children}
    </span>
  );
}
