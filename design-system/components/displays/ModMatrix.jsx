import React from "react";

/**
 * ModMatrix — the modulation routing grid (backlog item #5).
 * Rows are mod sources (LFOs, envelopes, macros), columns are destination
 * parameters. Each cell shows a bipolar depth (-1..+1) as a center-anchored
 * bar — positive in the source's color, negative in warm-grey. Click-drag a
 * cell vertically to set depth; click the center to clear. Cosmetic: drives
 * the `values` map you pass in (keyed "<sourceId>:<destId>").
 */
export function ModMatrix({
  sources = [],
  destinations = [],
  values = {},
  onChange,
  cell = 38,
  className = "",
  ...rest
}) {
  const key = (s, d) => `${s}:${d}`;
  const drag = React.useRef(null);

  const onDown = (s, d) => (e) => {
    drag.current = { s, d, startY: e.clientY, start: values[key(s, d)] || 0 };
    e.preventDefault();
  };
  React.useEffect(() => {
    const move = (e) => {
      const g = drag.current;
      if (!g || !onChange) return;
      const dv = (g.startY - e.clientY) / 80; // 80px = full range
      const v = Math.max(-1, Math.min(1, g.start + dv));
      onChange({ ...values, [key(g.s, g.d)]: +v.toFixed(2) });
    };
    const up = () => (drag.current = null);
    window.addEventListener("mousemove", move);
    window.addEventListener("mouseup", up);
    return () => { window.removeEventListener("mousemove", move); window.removeEventListener("mouseup", up); };
  }, [values, onChange]);

  const clearCell = (s, d) => (e) => {
    if (!onChange) return;
    e.stopPropagation();
    const next = { ...values }; delete next[key(s, d)]; onChange(next);
  };

  const labelW = 96;

  return (
    <div className={`aa-modmatrix ${className}`} style={{ display: "inline-block", background: "var(--surface-card)", border: "var(--border-hairline)", borderRadius: "var(--radius-md)", overflow: "hidden" }} {...rest}>
      {/* header row */}
      <div style={{ display: "flex", borderBottom: "var(--border-hairline)" }}>
        <div style={{ width: labelW, flex: "0 0 auto", borderRight: "var(--border-hairline)", background: "var(--surface-sunken)" }} />
        {destinations.map((d) => (
          <div key={d.id} style={{ width: cell, flex: "0 0 auto", height: 64, display: "flex", alignItems: "flex-end", justifyContent: "center", paddingBottom: 6, borderRight: "var(--border-hairline)" }}>
            <span style={{ writingMode: "vertical-rl", transform: "rotate(180deg)", fontSize: "var(--fs-9)", fontWeight: "var(--fw-semibold)", letterSpacing: "0.02em", textTransform: "uppercase", color: "var(--text-muted)", whiteSpace: "nowrap" }}>{d.label}</span>
          </div>
        ))}
      </div>

      {/* source rows */}
      {sources.map((s) => (
        <div key={s.id} style={{ display: "flex", borderBottom: "var(--border-hairline)" }}>
          <div style={{ width: labelW, flex: "0 0 auto", height: cell, display: "flex", alignItems: "center", gap: 7, padding: "0 10px", borderRight: "var(--border-hairline)", background: "var(--surface-sunken)" }}>
            <span style={{ width: 8, height: 8, borderRadius: 2, background: s.color || "var(--accent-primary)", flex: "0 0 auto" }} />
            <span style={{ fontSize: "var(--fs-10)", fontWeight: "var(--fw-semibold)", color: "var(--text-body)", whiteSpace: "nowrap" }}>{s.label}</span>
          </div>
          {destinations.map((d) => {
            const v = values[key(s.id, d.id)] || 0;
            const active = Math.abs(v) > 0.001;
            const col = v >= 0 ? (s.color || "var(--accent-primary)") : "var(--text-muted)";
            return (
              <div key={d.id} onMouseDown={onDown(s.id, d.id)} onDoubleClick={clearCell(s.id, d.id)} title={active ? (v > 0 ? "+" : "") + Math.round(v * 100) + "%" : "drag to assign"}
                style={{ width: cell, height: cell, flex: "0 0 auto", position: "relative", borderRight: "var(--border-hairline)", cursor: "ns-resize", background: active ? "var(--surface-page)" : "transparent" }}>
                {/* center line */}
                <div style={{ position: "absolute", left: 4, right: 4, top: "50%", height: 1, background: "var(--border-default)", opacity: 0.7 }} />
                {/* bipolar bar */}
                {active ? (
                  <div style={{ position: "absolute", left: "50%", transform: "translateX(-50%)",
                    width: 6, borderRadius: 2, background: col,
                    height: Math.abs(v) * (cell / 2 - 3),
                    top: v >= 0 ? `calc(50% - ${Math.abs(v) * (cell / 2 - 3)}px)` : "50%" }} />
                ) : (
                  <div style={{ position: "absolute", inset: 0, display: "flex", alignItems: "center", justifyContent: "center", color: "var(--border-default)", fontSize: 11, opacity: 0.5 }}>·</div>
                )}
              </div>
            );
          })}
        </div>
      ))}
    </div>
  );
}
