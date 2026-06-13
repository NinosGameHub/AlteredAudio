// Gain 76 – full plugin UI (820×820 square window).
// Hero face knob (340px) · INPUT / OUTPUT meters · L/R peak strip · footer.
(function () {
  const { useState, useEffect, useRef } = React;
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { Knob, Meter, LED, PowerButton, SegmentedControl, Readout } = DS;

  // ── tick format helpers ────────────────────────────────────────────────────
  const fmtDb = (v) => (v >= 0 ? "+" : "") + v.toFixed(1) + " dB";
  const fmtPct = (v) => Math.round(v) + " %";

  // ── simulated meter drift ──────────────────────────────────────────────────
  function useFakeMeter(base) {
    const [val, setVal] = useState(base);
    const ref = useRef(base);
    useEffect(() => {
      const id = setInterval(() => {
        ref.current = Math.max(-60, Math.min(0, ref.current + (Math.random() - 0.48) * 4));
        setVal(ref.current);
      }, 80);
      return () => clearInterval(id);
    }, []);
    return val;
  }

  // ── sub-components ─────────────────────────────────────────────────────────
  function PeakStrip({ label, value }) {
    const over = value > -0.3;
    return (
      <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
        <span className="af-label" style={{ fontSize: "9px", width: "8px" }}>{label}</span>
        <span style={{
          fontFamily: "var(--font-mono)", fontSize: "13px", fontWeight: 600,
          color: over ? "var(--warn-amber)" : "var(--text-primary)",
          minWidth: "56px", textAlign: "right",
        }}>{value > -60 ? fmtDb(value) : "—"}</span>
        <div style={{
          width: "8px", height: "8px", borderRadius: "50%",
          background: over ? "var(--warn-amber)" : "var(--surface-panel)",
          boxShadow: over ? "0 0 6px 2px var(--warn-amber)" : "none",
          transition: "all .1s",
        }} />
      </div>
    );
  }

  function StatRow({ label, value }) {
    return (
      <div style={{ display: "inline-flex", alignItems: "baseline", gap: "8px" }}>
        <span className="af-label" style={{ fontSize: "9px" }}>{label}</span>
        <span style={{ fontFamily: "var(--font-mono)", fontSize: "12px", fontWeight: 500, color: "var(--text-primary)" }}>{value}</span>
      </div>
    );
  }

  // ── root app ───────────────────────────────────────────────────────────────
  function App() {
    const [gain, setGain] = useState(0);
    const [mix, setMix] = useState(100);
    const [mode, setMode] = useState("STEREO");
    const [oversampling, setOversampling] = useState("1×");
    const [power, setPower] = useState(true);
    const [peakL, setPeakL] = useState(-6.2);
    const [peakR, setPeakR] = useState(-6.2);
    const inL = useFakeMeter(-18 + gain * 0.4);
    const inR = useFakeMeter(-18 + gain * 0.4);
    const outL = useFakeMeter(-18 + gain * 0.6);
    const outR = useFakeMeter(-18 + gain * 0.6);

    useEffect(() => {
      if (outL > peakL) setPeakL(outL);
      if (outR > peakR) setPeakR(outR);
    }, [outL, outR]);

    const resetPeaks = () => { setPeakL(-60); setPeakR(-60); };

    return (
      <div id="plugin" style={{
        width: "820px", height: "820px",
        background: "var(--surface-base)",
        backgroundImage: "linear-gradient(to bottom, rgba(255,253,247,0.45), rgba(255,253,247,0) 110px)",
        borderRadius: "var(--radius-xl)", border: "1px solid #000",
        boxShadow: "var(--shadow-bezel), 0 1px 0 rgba(255,255,255,0.38) inset",
        display: "flex", flexDirection: "column", overflow: "hidden", boxSizing: "border-box",
      }}>
        {/* ── header ── */}
        <header style={{
          display: "flex", alignItems: "center", padding: "18px 22px 0",
          gap: "16px",
        }}>
          <div style={{ display: "flex", flexDirection: "column", gap: "4px" }}>
            <div className="af-wordmark" style={{ fontSize: "24px", lineHeight: 1 }}>ALTERED AUDIO</div>
            <div style={{ fontFamily: "var(--font-display)", fontSize: "11px", letterSpacing: "var(--tracking-wide)",
              textTransform: "uppercase", color: "var(--accent-amber-active)" }}>GAIN 76</div>
          </div>
          <div style={{ marginLeft: "auto", display: "flex", alignItems: "center", gap: "20px" }}>
            <Readout label="MODE" labelPlacement="top" value={mode} size="sm" />
            <Readout label="OVRSMPL" labelPlacement="top" value={oversampling} size="sm" />
            <PowerButton on={power} onChange={setPower} />
          </div>
        </header>

        {/* ── centre: meters · hero knob · meters ── */}
        <div style={{
          flex: 1, display: "flex", alignItems: "center", justifyContent: "center",
          gap: "32px", padding: "20px 36px",
        }}>
          {/* INPUT meter */}
          <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "12px" }}>
            <span className="af-section-label" style={{ fontSize: "10px" }}>INPUT</span>
            <Meter height={380} channels={[{ value: inL, label: "L" }, { value: inR, label: "R" }]} showReadout={false} />
          </div>

          {/* hero knob */}
          <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "24px" }}>
            <Knob
              face label="GAIN · DB"
              value={gain} min={-24} max={24} defaultValue={0} bipolar
              size="340px" ticks={36}
              display={fmtDb(gain)}
              onChange={setGain}
            />
            <div style={{ display: "flex", gap: "20px" }}>
              <div style={{ display: "flex", flexDirection: "column", gap: "4px" }}>
                <PeakStrip label="L" value={peakL} />
                <PeakStrip label="R" value={peakR} />
              </div>
              <button onClick={resetPeaks}
                style={{
                  fontFamily: "var(--font-mono)", fontSize: "9px", letterSpacing: "var(--tracking-wide)",
                  textTransform: "uppercase", background: "none", border: "var(--border-subtle)",
                  borderRadius: "var(--radius-xs)", color: "var(--text-muted)",
                  padding: "4px 10px", cursor: "pointer",
                }}>RESET</button>
            </div>
          </div>

          {/* OUTPUT meter */}
          <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "12px" }}>
            <span className="af-section-label" style={{ fontSize: "10px" }}>OUTPUT</span>
            <Meter height={380} channels={[{ value: outL, label: "L" }, { value: outR, label: "R" }]} showReadout={false} />
          </div>
        </div>

        {/* ── controls strip ── */}
        <div style={{
          display: "flex", alignItems: "center", justifyContent: "center", gap: "28px",
          padding: "0 36px 16px",
        }}>
          <SegmentedControl label="MODE" value={mode} onChange={setMode}
            options={[{ value: "STEREO", label: "STEREO" }, { value: "MONO", label: "MONO" }, { value: "M/S", label: "M / S" }]} />
          <SegmentedControl label="OVERSAMPLING" value={oversampling} onChange={setOversampling}
            options={[{ value: "1×", label: "1×" }, { value: "2×", label: "2×" }, { value: "4×", label: "4×" }]} />
          <Knob label="MIX" value={mix} min={0} max={100} defaultValue={100}
            size="var(--knob-sm)" ticks={20}
            display={fmtPct(mix)} onChange={setMix} />
        </div>

        {/* ── footer ── */}
        <footer style={{
          display: "flex", alignItems: "center", gap: "24px", height: "42px",
          padding: "0 18px", margin: "0 0 0 0",
          background: "var(--surface-panel)", borderTop: "var(--border-default)",
        }}>
          <LED on label="SYSTEM" />
          <StatRow label="SR" value="44.1 kHz" />
          <StatRow label="LATENCY" value="0.00 ms" />
          <StatRow label="CPU" value="0.4 %" />
          <span style={{ marginLeft: "auto", fontFamily: "var(--font-mono)", fontSize: "11px", color: "var(--text-muted)" }}>v1.0.1</span>
        </footer>
      </div>
    );
  }

  window.AF_GainApp = App;

  // auto-mount if #root is present and DS is ready
  function tryMount() {
    const root = document.getElementById("root");
    if (!root || !window.AuroraFilterDesignSystem_83b750) return setTimeout(tryMount, 60);
    ReactDOM.createRoot(root).render(<App />);
    function fit() {
      const stage = document.getElementById("stage");
      if (!stage) return;
      const sc = Math.min(window.innerWidth / 820, window.innerHeight / 820, 1);
      stage.style.transform = "scale(" + sc + ")";
    }
    window.addEventListener("resize", fit);
    fit(); setTimeout(fit, 300);
  }
  tryMount();
})();
