// gain-app.jsx — Altered Audio GAIN 76: hero gain dial, I/O meters, L/R peak strip.
// Kit-local module: defines window.AF_GainApp; the kit page mounts it.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  if (!DS) return;
  const { Knob, Readout, Select, Button, PowerButton, Panel, PeakMeter } = DS;

  const fmtDb = (v, signed = true) => (signed && v >= 0 ? "+" : "") + v.toFixed(2);

  function App() {
    const [gain, setGain] = React.useState(0);
    const [mode, setMode] = React.useState("STEREO");
    const [link, setLink] = React.useState(true);
    const [os, setOs] = React.useState("1x");
    const [power, setPower] = React.useState(true);

    // ---- cosmetic signal simulation ----
    const [sig, setSig] = React.useState({ inL: -9, inR: -10, peak: -6, lufs: -14.5 });
    React.useEffect(() => {
      const id = setInterval(() => {
        setSig((s) => {
          const j = () => (Math.random() - 0.46) * 2.4;
          const inL = Math.max(-22, Math.min(-2, s.inL + j()));
          const inR = Math.max(-22, Math.min(-2, s.inR + j()));
          const outL = inL + gain, outR = inR + gain;
          const pk = Math.max(outL, outR);
          return {
            inL, inR,
            peak: Math.max(pk, s.peak - 0.18),
            lufs: s.lufs + (((outL + outR) / 2 - 9) - s.lufs) * 0.04,
          };
        });
      }, 120);
      return () => clearInterval(id);
    }, [gain]);

    const outL = sig.inL + gain, outR = sig.inR + gain;
    const inAvg = (sig.inL + sig.inR) / 2, outAvg = (outL + outR) / 2;

    const statLbl = { fontFamily: "var(--font-mono)", fontSize: "10px", fontWeight: 500,
      letterSpacing: "var(--tracking-label)", textTransform: "uppercase", color: "var(--text-secondary)" };
    const cell = { display: "flex", flexDirection: "column", alignItems: "center", justifyContent: "center",
      gap: "10px", flex: 1, padding: "12px 0", borderLeft: "1px solid var(--c-border-soft)" };

    // I/O meter column: INPUT/dB header, embossed meter, dark readout well below
    const MeterCol = ({ label, value, readout }) => (
      <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "8px" }}>
        <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "2px" }}>
          <span className="af-label" style={{ fontSize: "11px", fontWeight: 600 }}>{label}</span>
          <span style={{ fontFamily: "var(--font-mono)", fontSize: "10px", color: "var(--text-muted)" }}>dB</span>
        </div>
        <PeakMeter value={power ? value : -60} min={-24} max={24} warnAt={0}
          length={470} thickness={22} segments={52} scale={[24, 12, 0, -12, -24]} />
        <Readout value={power ? readout : "—"} size="sm" align="center" />
      </div>
    );

    return (
      <div id="plugin">
        {/* ---- host title bar ---- */}
        <div style={{
          display: "flex", alignItems: "center", gap: "12px", height: "44px", padding: "0 18px",
          background: "var(--surface-raised)", borderBottom: "1px solid var(--c-border-soft)",
          borderRadius: "var(--radius-xl) var(--radius-xl) 0 0", flex: "0 0 auto",
        }}>
          <span style={{ fontFamily: "var(--font-mono)", fontSize: "13px", color: "var(--text-primary)" }}>
            Down In The Dark / AlteredAudio Gain 76
          </span>
          <span aria-hidden="true" style={{ marginLeft: "auto", fontFamily: "var(--font-mono)", fontSize: "15px",
            color: "var(--text-secondary)", lineHeight: 1 }}>×</span>
        </div>

        {/* ---- faceplate ---- */}
        <div style={{ flex: 1, display: "flex", flexDirection: "column", gap: "12px", padding: "14px 16px 12px", minHeight: 0 }}>
          {/* header: wordmark · power */}
          <header style={{ display: "flex", alignItems: "flex-start", padding: "4px 6px 0", flex: "0 0 auto" }}>
            <div style={{ display: "flex", flexDirection: "column", gap: "5px" }}>
              <div className="af-wordmark" style={{ fontSize: "24px", lineHeight: 1 }}>ALTERED AUDIO</div>
              <div style={{ fontFamily: "var(--font-display)", fontSize: "11px", letterSpacing: "var(--tracking-wide)",
                textTransform: "uppercase", color: "var(--accent-amber-active)" }}>GAIN 76</div>
            </div>
            <div style={{ marginLeft: "auto" }}>
              <PowerButton on={power} onChange={setPower} size={44} />
            </div>
          </header>

          {/* main: input meter · hero dial · output meter */}
          <Panel style={{ flex: 1, display: "flex", alignItems: "center", justifyContent: "space-between", padding: "22px 44px" }}>
            <MeterCol label="INPUT" value={inAvg} readout={inAvg.toFixed(1)} />

            <div style={{ position: "relative", display: "flex", flexDirection: "column", alignItems: "center" }}>
              <Knob face label="GAIN · DB" value={gain} min={-24} max={24} defaultValue={0} bipolar
                size={430} ticks={48} display={gain.toFixed(1)} onChange={setGain} />
              <div style={{ position: "absolute", left: "8px", bottom: "40px", fontFamily: "var(--font-mono)",
                fontSize: "13px", color: "var(--text-secondary)" }}>−24</div>
              <div style={{ position: "absolute", right: "8px", bottom: "40px", fontFamily: "var(--font-mono)",
                fontSize: "13px", color: "var(--text-secondary)" }}>+24</div>
              <div style={{ position: "absolute", top: "-20px", fontFamily: "var(--font-mono)",
                fontSize: "13px", color: "var(--text-secondary)" }}>0</div>
            </div>

            <MeterCol label="OUTPUT" value={outAvg} readout={outAvg.toFixed(1)} />
          </Panel>

          {/* L/R peak strip — center-split butterfly */}
          <Panel padding="14px 22px" style={{ flex: "0 0 auto" }}>
            <div style={{ display: "flex", alignItems: "center", gap: "14px" }}>
              <span style={{ ...statLbl, fontSize: "11px" }}>L</span>
              <div style={{ flex: 1, display: "flex", flexDirection: "column", gap: "5px" }}>
                <div style={{ display: "flex", alignItems: "center" }}>
                  <PeakMeter orientation="horizontal" reverse value={power ? outL : -60}
                    min={-24} max={6} warnAt={0} length={398} thickness={16} segments={44} style={{ flex: 1 }} />
                  <span style={{ width: "2px", height: "16px", background: "var(--c-border)", margin: "0 3px", flex: "0 0 auto" }}></span>
                  <PeakMeter orientation="horizontal" value={power ? outR : -60}
                    min={-24} max={6} warnAt={0} length={398} thickness={16} segments={44} style={{ flex: 1 }} />
                </div>
                <div style={{ display: "flex", fontFamily: "var(--font-mono)", fontSize: "9px", color: "var(--text-muted)" }}>
                  <div style={{ flex: 1, display: "flex", justifyContent: "space-between", textAlign: "center" }}>
                    {[6, 3, 0, -3, -6, -12, -18, -24].map((s, i) => <span key={i}>{s > 0 ? `+${s}` : s}</span>)}
                  </div>
                  <span style={{ width: "8px" }}></span>
                  <div style={{ flex: 1, display: "flex", justifyContent: "space-between", textAlign: "center" }}>
                    {[-24, -18, -12, -6, -3, 0, 3, 6].map((s, i) => <span key={i}>{s > 0 ? `+${s}` : s}</span>)}
                  </div>
                </div>
              </div>
              <span style={{ ...statLbl, fontSize: "11px" }}>R</span>
            </div>
          </Panel>

          {/* footer: mode · link · peak · lufs · oversampling */}
          <Panel padding="0" style={{ display: "flex", alignItems: "stretch", flex: "0 0 auto" }}>
            <div style={{ ...cell, borderLeft: "none" }}>
              <span style={statLbl}>MODE</span>
              <Select value={mode} onChange={setMode} options={["STEREO", "MONO", "SIDE"]} />
            </div>
            <div style={cell}>
              <span style={statLbl}>LINK</span>
              <Button variant="toggle" active={link} onClick={() => setLink(!link)}>{link ? "ON" : "OFF"}</Button>
            </div>
            <div style={cell}>
              <span style={statLbl}>PEAK</span>
              <Readout value={power ? fmtDb(sig.peak, false) + " dB" : "—"} size="sm" />
            </div>
            <div style={cell}>
              <span style={statLbl}>LUFS</span>
              <Readout value={power ? sig.lufs.toFixed(1) : "– –"} size="sm" />
            </div>
            <div style={cell}>
              <span style={statLbl}>OVERSAMPLING</span>
              <Select value={os} onChange={setOs} options={["1x", "4x", "8x"]} />
            </div>
          </Panel>

          {/* version */}
          <div style={{ display: "flex", justifyContent: "flex-end", padding: "0 6px", flex: "0 0 auto" }}>
            <span style={{ fontFamily: "var(--font-mono)", fontSize: "11px", color: "var(--text-muted)" }}>v1.0.1</span>
          </div>
        </div>
      </div>
    );
  }

  window.AF_GainApp = App;
})();
