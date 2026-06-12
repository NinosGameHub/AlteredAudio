import React from "react";

/**
 * Meter — segmented level meter for the I/O readouts. One or more vertical
 * channels of amber segments (top band warns orange) on near-black, an
 * optional dB scale on the left and a dark readout box under each channel.
 * Used for OUTPUT (L/R) and UTILITY INPUT (L/R).
 */
export function Meter({
  channels = [{ value: 0.5, label: "L" }],
  scale,                  // e.g. [24, 12, 0, -12, -24] — rendered at left
  segments = 22,
  height = 150,
  showReadout = true,
  title,
  className = "",
  ...rest
}) {
  const chs = channels.map((c) => (typeof c === "number" ? { value: c } : c));

  const seg = (v) => {
    const cells = [];
    for (let i = segments - 1; i >= 0; i--) {
      const frac = (i + 0.5) / segments;
      const lit = frac <= v;
      const color = frac > 0.9 ? "var(--warning)" : "var(--crt-line)";
      cells.push(
        <span key={i} style={{
          flex: 1,
          background: lit ? color : "rgba(240,181,71,0.10)",
          opacity: lit ? (0.55 + frac * 0.45) : 1,
          borderRadius: "1px",
        }} />
      );
    }
    return cells;
  };

  return (
    <div className={`af-meter ${className}`} style={{ display: "inline-flex", flexDirection: "column", gap: "8px" }} {...rest}>
      {title ? <span className="af-label" style={{ textAlign: "center" }}>{title}</span> : null}
      <div style={{ display: "flex", alignItems: "stretch", gap: "8px" }}>
        {scale ? (
          <div style={{
            display: "flex", flexDirection: "column", justifyContent: "space-between",
            height: `${height}px`, paddingTop: "10px",
            fontFamily: "var(--font-mono)", fontSize: "var(--fs-9)", color: "var(--text-muted)", textAlign: "right",
          }}>
            <span style={{ marginBottom: "-6px", color: "var(--text-secondary)" }}>dB</span>
            {scale.map((s) => <span key={s}>{s > 0 ? `+${s}` : s}</span>)}
          </div>
        ) : null}
        <div style={{ display: "flex", gap: "10px" }}>
          {chs.map((c, i) => (
            <div key={i} style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "6px" }}>
              {c.label ? <span style={{ fontFamily: "var(--font-mono)", fontSize: "var(--fs-9)", color: "var(--text-muted)" }}>{c.label}</span> : null}
              <div style={{
                display: "flex", flexDirection: "column", gap: "2px",
                width: "12px", height: `${height}px`, padding: "2px",
                background: "var(--surface-display)", borderRadius: "var(--radius-xs)",
                boxShadow: "var(--inset-readout)",
              }}>
                {seg(Math.max(0, Math.min(1, c.value)))}
              </div>
              {showReadout ? (
                <span style={{
                  fontFamily: "var(--font-mono)", fontSize: "var(--fs-10)", color: "var(--text-on-display)",
                  background: "var(--surface-display)", padding: "2px 6px", borderRadius: "var(--radius-xs)",
                  boxShadow: "var(--inset-readout)", minWidth: "32px", textAlign: "center",
                }}>{c.readout != null ? c.readout : "0.0"}</span>
              ) : null}
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
