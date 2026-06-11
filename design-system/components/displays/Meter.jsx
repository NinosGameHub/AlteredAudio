import React from "react";

/**
 * Meter — a thin segmented level meter for I/O and gain-reduction readouts
 * (the header I-O meters and the comp/limiter GR meters in the todo).
 * Vertical or horizontal. For levels, fill rises green→amber→orange toward
 * the top; for gain reduction (`mode="gr"`) it fills downward in the
 * dynamics burnt-orange.
 */
export function Meter({
  value = 0,            // 0..1
  mode = "level",       // "level" | "gr"
  orientation = "vertical",
  segments = 16,
  length = 80,
  thickness = 6,
  label,
  className = "",
  ...rest
}) {
  const v = Math.max(0, Math.min(1, value));
  const horizontal = orientation === "horizontal";

  const segColor = (i) => {
    const frac = (i + 1) / segments;
    if (mode === "gr") return "var(--cat-dynamics)";
    if (frac > 0.92) return "var(--cat-dynamics)";
    if (frac > 0.78) return "var(--crt-amber)";
    return "var(--state-active)";
  };

  // for GR the meter lights from the top downward
  const lit = (i) => {
    const frac = (i + 1) / segments;
    return mode === "gr" ? frac > 1 - v : frac <= v;
  };

  const segs = Array.from({ length: segments }, (_, i) => {
    // visual order: bottom→top for vertical level; render reversed
    const idx = horizontal ? i : segments - 1 - i;
    return (
      <span
        key={idx}
        style={{
          flex: 1,
          background: lit(idx) ? segColor(idx) : "var(--aa-surface-alt)",
          borderRadius: "1px",
          opacity: lit(idx) ? 1 : 0.6,
          transition: "background var(--dur-fast) linear",
        }}
      />
    );
  });

  return (
    <div
      className={`aa-meter aa-meter--${mode} ${className}`}
      style={{
        display: "inline-flex",
        flexDirection: "column",
        alignItems: "center",
        gap: "4px",
      }}
      {...rest}
    >
      <div
        style={{
          display: "flex",
          flexDirection: horizontal ? "row" : "column",
          gap: "2px",
          width: horizontal ? `${length}px` : `${thickness}px`,
          height: horizontal ? `${thickness}px` : `${length}px`,
          padding: "2px",
          background: "var(--crt-bg)",
          borderRadius: "var(--radius-xs)",
        }}
      >
        {segs}
      </div>
      {label ? (
        <span
          style={{
            fontSize: "var(--fs-9)",
            fontFamily: "var(--font-mono)",
            color: "var(--text-muted)",
            letterSpacing: "0.04em",
          }}
        >
          {label}
        </span>
      ) : null}
    </div>
  );
}
