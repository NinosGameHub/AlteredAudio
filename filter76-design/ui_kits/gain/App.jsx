// App.jsx — Gain 76: utility gain with the Altered Audio faceplate.
// Mirrors the JUCE implementation (GainEditor.cpp, v0.8.7):
//   header wordmark + power key · main panel with INPUT / OUTPUT vertical
//   meters flanking one huge hero face knob (value printed on the disc) ·
//   L/R peak strip with hold markers · footer cells MODE | PEAK | LUFS |
//   OVERSAMPLING.
(function () {
  const { AF_Knob, AF_Meter, AF_Select, AF_PowerButton, AF_Panel } = window;

  function GainApp() {
    const [gain, setGain] = React.useState(4.22);
    const [mode, setMode] = React.useState("STEREO");
    const [os, setOs] = React.useState("1x");
    const [power, setPower] = React.useState(true);
    const [meters, setMeters] = React.useState({ in: 0.7, outL: 0.74, outR: 0.7 });
    const [heldPeak, setHeldPeak] = React.useState(-120);
    const [lufs, setLufs] = React.useState(-12.4);

    React.useEffect(() => {
      const id = setInterval(() => {
        const j = () => (Math.random() - 0.5) * 0.12;
        setMeters((m) => {
          const outL = Math.max(0.2, Math.min(0.98, m.outL + j()));
          const outR = Math.max(0.2, Math.min(0.98, m.outR + j()));
          setHeldPeak((p) => Math.max(p, 20 * Math.log10(Math.max(outL, outR))));
          setLufs((l) => Math.max(-30, Math.min(-6, l + (Math.random() - 0.5) * 0.6)));
          return { in: Math.max(0.2, Math.min(0.98, m.in + j())), outL, outR };
        });
      }, 140);
      return () => clearInterval(id);
    }, []);

    const fmtPeak = heldPeak <= -119 ? "-∞ dB"
      : (heldPeak >= 0 ? "+" : "") + heldPeak.toFixed(2) + " dB";

    return (
      <div id="plugin" style={{ width: 820, height: 820, position: "relative",
        background: "var(--surface-base)", fontFamily: "var(--font-mono)" }}>

        {/* header */}
        <div style={{ display: "flex", alignItems: "flex-start", padding: "10px 24px 0" }}>
          <div style={{ flex: 1 }}>
            <div style={{ fontFamily: "var(--font-display)", fontSize: 24, fontWeight: 700,
              letterSpacing: "0.22em", color: "var(--text-primary)" }}>ALTERED AUDIO</div>
            <div style={{ fontFamily: "var(--font-display)", fontSize: 11, fontWeight: 700,
              letterSpacing: "0.14em", color: "var(--accent-press)", marginTop: 4 }}>GAIN 76</div>
          </div>
          <AF_PowerButton on={power} onChange={setPower} />
        </div>

        {/* main panel: meters + hero knob */}
        <AF_Panel style={{ position: "absolute", left: 16, top: 80, width: 788, height: 540 }}>
          <div style={{ position: "absolute", left: 28, top: 22, fontSize: 9,
            letterSpacing: "0.08em", color: "var(--text-secondary)" }}>INPUT</div>
          <div style={{ position: "absolute", left: 28, top: 40, height: 460 }}>
            <AF_Meter level={meters.in} vertical height={460} width={38} />
          </div>

          <div style={{ position: "absolute", right: 28, top: 22, fontSize: 9,
            letterSpacing: "0.08em", color: "var(--text-secondary)" }}>OUTPUT</div>
          <div style={{ position: "absolute", right: 28, top: 40, height: 460 }}>
            <AF_Meter level={Math.max(meters.outL, meters.outR)} vertical height={460} width={38} />
          </div>

          {/* hero face knob — value on the disc, GAIN · DB label folded in */}
          <div style={{ position: "absolute", left: "50%", top: "50%",
            transform: "translate(-50%,-50%)" }}>
            <AF_Knob value={gain} min={-24} max={24} defaultValue={0}
              size={340} face label="GAIN · DB"
              format={(v) => (v >= 0 ? "+" : "") + v.toFixed(2)}
              onChange={setGain} />
          </div>
          {/* scale labels */}
          <div style={{ position: "absolute", left: "calc(50% - 190px)", bottom: 124,
            fontSize: 12, color: "var(--text-secondary)" }}>-24</div>
          <div style={{ position: "absolute", left: "50%", top: 56, transform: "translateX(-50%)",
            fontSize: 12, color: "var(--text-secondary)" }}>0</div>
          <div style={{ position: "absolute", right: "calc(50% - 190px)", bottom: 124,
            fontSize: 12, color: "var(--text-secondary)" }}>+24</div>
        </AF_Panel>

        {/* L/R peak strip with hold markers (R bar mirrored, grows right→left) */}
        <AF_Panel style={{ position: "absolute", left: 16, top: 636, width: 788, height: 74 }}>
          <div style={{ position: "absolute", left: 12, top: "50%", transform: "translateY(-50%)",
            fontSize: 9, color: "var(--text-secondary)" }}>L</div>
          <div style={{ position: "absolute", left: 30, top: 18, width: 354, height: 38 }}>
            <AF_Meter level={meters.outL} peakHold height={38} />
          </div>
          <div style={{ position: "absolute", right: 30, top: 18, width: 354, height: 38 }}>
            <AF_Meter level={meters.outR} peakHold mirrored height={38} />
          </div>
          <div style={{ position: "absolute", right: 12, top: "50%", transform: "translateY(-50%)",
            fontSize: 9, color: "var(--text-secondary)" }}>R</div>
        </AF_Panel>

        {/* footer: MODE | PEAK | LUFS | OVERSAMPLING */}
        <div style={{ position: "absolute", left: 16, top: 726, width: 788, height: 52,
          background: "var(--surface-recessed)", border: "1px solid var(--border)",
          borderRadius: "var(--radius-md)", display: "grid",
          gridTemplateColumns: "1fr 1fr 1fr 1fr", alignItems: "center" }}>
          <div style={{ display: "flex", alignItems: "center", gap: 10, padding: "0 16px",
            borderRight: "1px solid var(--border)" }}>
            <span style={{ fontSize: 9, color: "var(--text-secondary)" }}>MODE</span>
            <AF_Select value={mode} options={["STEREO", "MONO", "SIDE"]} onChange={setMode} width={96} />
          </div>
          <div style={{ display: "flex", alignItems: "center", gap: 10, padding: "0 16px",
            borderRight: "1px solid var(--border)", cursor: "pointer" }}
            onClick={() => setHeldPeak(-120)} title="click to reset">
            <span style={{ fontSize: 9, color: "var(--text-secondary)" }}>PEAK</span>
            <span style={{ fontSize: 12, color: "var(--text-primary)" }}>{fmtPeak}</span>
          </div>
          <div style={{ display: "flex", alignItems: "center", gap: 10, padding: "0 16px",
            borderRight: "1px solid var(--border)" }}>
            <span style={{ fontSize: 9, color: "var(--text-secondary)" }}>LUFS</span>
            <span style={{ fontSize: 12, color: "var(--text-primary)" }}>{lufs.toFixed(1)}</span>
          </div>
          <div style={{ display: "flex", alignItems: "center", gap: 10, padding: "0 16px" }}>
            <span style={{ fontSize: 9, color: "var(--text-secondary)" }}>OVERSAMPLING</span>
            <AF_Select value={os} options={["1x", "4x", "8x"]} onChange={setOs} width={64} />
          </div>
        </div>
      </div>
    );
  }

  window.GainApp = GainApp;
  if (document.getElementById("root"))
    ReactDOM.createRoot(document.getElementById("root")).render(<GainApp />);
})();
