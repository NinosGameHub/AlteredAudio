// TweaksPanel.jsx — collapsible surface-finish / advanced options drawer.
// useTweaks() hook + TweaksPanel shell + TweakSection / TweakSlider / TweakRadio primitives.
(function () {
  const { useState, useCallback } = React;

  // ── hook ──────────────────────────────────────────────────────────────────
  function useTweaks(defaults) {
    const [tweaks, setTweaks] = useState(defaults);
    const set = useCallback((key, value) => setTweaks((t) => ({ ...t, [key]: value })), []);
    const reset = useCallback(() => setTweaks(defaults), [defaults]);
    return { tweaks, set, reset };
  }

  // ── label ─────────────────────────────────────────────────────────────────
  const TweakLabel = ({ children }) => (
    <span className="af-label" style={{ fontSize: "9px", whiteSpace: "nowrap" }}>{children}</span>
  );

  // ── section ───────────────────────────────────────────────────────────────
  function TweakSection({ title, children }) {
    return (
      <section style={{ display: "flex", flexDirection: "column", gap: "10px" }}>
        <div className="af-section-label" style={{ fontSize: "10px", borderBottom: "1px solid var(--border-subtle)", paddingBottom: "4px" }}>{title}</div>
        {children}
      </section>
    );
  }

  // ── slider ─────────────────────────────────────────────────────────────────
  function TweakSlider({ label, value, min, max, step = 1, display, onChange }) {
    const pct = ((value - min) / (max - min)) * 100;
    return (
      <div style={{ display: "flex", alignItems: "center", gap: "10px" }}>
        <TweakLabel>{label}</TweakLabel>
        <div style={{ flex: 1, position: "relative", height: "20px", display: "flex", alignItems: "center" }}>
          <div style={{
            position: "absolute", left: 0, top: "50%", transform: "translateY(-50%)",
            width: "100%", height: "3px", background: "var(--surface-panel)",
            borderRadius: "2px", boxShadow: "var(--inset-readout)",
          }} />
          <div style={{
            position: "absolute", left: 0, top: "50%", transform: "translateY(-50%)",
            width: pct + "%", height: "3px", background: "var(--accent-amber-active)",
            borderRadius: "2px",
          }} />
          <input type="range" min={min} max={max} step={step} value={value}
            onChange={(e) => onChange(Number(e.target.value))}
            style={{
              position: "absolute", inset: 0, width: "100%", opacity: 0, cursor: "pointer",
              height: "100%", margin: 0, padding: 0,
            }} />
        </div>
        <span style={{
          fontFamily: "var(--font-mono)", fontSize: "11px", color: "var(--text-primary)",
          minWidth: "36px", textAlign: "right",
        }}>{display !== undefined ? display : value}</span>
      </div>
    );
  }

  // ── radio group ────────────────────────────────────────────────────────────
  function TweakRadio({ label, value, options, onChange }) {
    return (
      <div style={{ display: "flex", alignItems: "center", gap: "10px" }}>
        <TweakLabel>{label}</TweakLabel>
        <div style={{ display: "flex", gap: "4px", flex: 1, flexWrap: "wrap" }}>
          {options.map((opt) => {
            const id = typeof opt === "string" ? opt : opt.value;
            const lbl = typeof opt === "string" ? opt : opt.label;
            const active = value === id;
            return (
              <button key={id} onClick={() => onChange(id)}
                style={{
                  fontFamily: "var(--font-mono)", fontSize: "10px", fontWeight: 500,
                  letterSpacing: "var(--tracking-wide)", textTransform: "uppercase",
                  padding: "4px 9px", borderRadius: "var(--radius-xs)",
                  background: active ? "var(--accent-amber-active)" : "var(--surface-panel)",
                  color: active ? "var(--surface-base)" : "var(--text-muted)",
                  border: active ? "none" : "var(--border-subtle)",
                  cursor: "pointer", transition: "all 0.12s",
                }}>{lbl}</button>
            );
          })}
        </div>
      </div>
    );
  }

  // ── panel shell ────────────────────────────────────────────────────────────
  function TweaksPanel({ open, onClose, children }) {
    if (!open) return null;
    return (
      <div style={{
        position: "absolute", right: 0, top: 0, bottom: 0, width: "320px",
        background: "var(--surface-panel)", borderLeft: "var(--border-default)",
        boxShadow: "-4px 0 24px rgba(0,0,0,0.35)",
        display: "flex", flexDirection: "column", gap: "0", zIndex: 100,
        borderTopRightRadius: "var(--radius-xl)", borderBottomRightRadius: "var(--radius-xl)",
        overflow: "hidden",
      }}>
        <header style={{
          display: "flex", alignItems: "center", justifyContent: "space-between",
          padding: "14px 16px", borderBottom: "var(--border-default)",
          flex: "0 0 auto",
        }}>
          <span className="af-section-label" style={{ fontSize: "11px" }}>TWEAKS</span>
          <button onClick={onClose}
            style={{
              background: "none", border: "none", cursor: "pointer",
              color: "var(--text-muted)", fontSize: "18px", lineHeight: 1, padding: "0 2px",
            }}>×</button>
        </header>
        <div style={{ flex: 1, overflowY: "auto", padding: "16px", display: "flex", flexDirection: "column", gap: "20px" }}>
          {children}
        </div>
      </div>
    );
  }

  // ── export ─────────────────────────────────────────────────────────────────
  window.AF_useTweaks = useTweaks;
  window.AF_TweaksPanel = TweaksPanel;
  window.AF_TweakSection = TweakSection;
  window.AF_TweakSlider = TweakSlider;
  window.AF_TweakRadio = TweakRadio;
})();
