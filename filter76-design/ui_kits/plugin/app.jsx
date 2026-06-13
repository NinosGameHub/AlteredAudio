// app.jsx — the full Filter 76 plugin (synced to v0.7.6).
// Mounts only on the kit page: requires the sibling modules + tweaks panel
// (loaded via <script type="text/babel">) and the page's #root container.
(function () {
  const D = window.AF_DATA;
  const { AF_Header, AF_DisplayRow, AF_FilterRow, AF_BottomRow, AF_Footer,
          useTweaks, TweaksPanel, TweakSection, TweakRadio, TweakSlider } = window;

  if (!D || !AF_Header || !AF_DisplayRow || !AF_FilterRow || !AF_BottomRow || !AF_Footer
      || typeof useTweaks !== "function" || !document.getElementById("root")) {
    return; // not the plugin kit page — do nothing
  }

  const TWEAK_DEFAULTS = /*EDITMODE-BEGIN*/{
    "surface": "worn",
    "grain": 25,
    "wear": 100,
    "gradient": 45
  }/*EDITMODE-END*/;

  function App() {
    const [filter, setFilter] = React.useState({
      type: "LP", freq: 800, q: 6.0, drive: 3.0, mix: 100, output: 0, gain: 0, slope: "24", mode: "ANALOG",
    });
    const [presetIndex, setPresetIndex] = React.useState(3);
    const [ab, setAb] = React.useState("A");
    const [power, setPower] = React.useState(true);
    const [mod, setMod] = React.useState({ source: "LFO A", dest: "FREQUENCY", amount: 60 });
    const [lfo, setLfo] = React.useState({ waveform: "sine", rate: 0.1, depth: 100, phase: 0 });
    const [env, setEnv] = React.useState({ attack: 2, release: 120, sens: 1.4 });
    const [meters, setMeters] = React.useState({ outL: 0.12, outR: 0.1 });
    const [t, setTweak] = useTweaks(TWEAK_DEFAULTS);

    const clean = t.surface === "clean";
    const grainO = clean ? 0 : (t.grain / 100) * 0.45;
    const wearO = clean ? 0 : (t.wear / 100) * 0.30;
    const sheenO = t.gradient / 100;

    const setF = (p) => setFilter((s) => ({ ...s, ...p }));

    // ghost curve: the modulated cutoff target when modulation routes to frequency
    const ghost = (mod.source !== "OFF" && mod.dest === "FREQUENCY" && mod.amount !== 0)
      ? { freq: Math.min(20000, Math.max(20, filter.freq * Math.pow(2, (mod.amount / 100) * 2.2))) }
      : null;

    // gentle meter idle animation
    React.useEffect(() => {
      const id = setInterval(() => {
        const j = () => (Math.random() - 0.5) * 0.06;
        setMeters((m) => ({
          outL: Math.max(0.04, Math.min(0.3, m.outL + j())),
          outR: Math.max(0.04, Math.min(0.3, m.outR + j())),
        }));
      }, 160);
      return () => clearInterval(id);
    }, []);

    const stepPreset = (d) => setPresetIndex((i) => (i + d + D.presets.length) % D.presets.length);

    return (
      <div id="plugin" style={{ "--grain-o": grainO, "--wear-o": wearO, "--sheen-o": sheenO }}>
        <div className="af-sheen"></div>
        <AF_Header
          presetIndex={presetIndex} presetName={D.presets[presetIndex]}
          onPrev={() => stepPreset(-1)} onNext={() => stepPreset(1)}
          ab={ab} onAb={setAb} oversampling="1x" mix="100 %"
          power={power} onPower={setPower} />

        <AF_DisplayRow
          filter={filter} ghost={ghost}
          onNodeChange={(n) => setF({ freq: n.freq, q: n.q })}
          outL={meters.outL} outR={meters.outR} />

        <AF_FilterRow filter={filter} set={setF} />

        <AF_BottomRow
          mod={mod} setMod={(p) => setMod((s) => ({ ...s, ...p }))}
          lfo={lfo} setLfo={(p) => setLfo((s) => ({ ...s, ...p }))}
          env={env} setEnv={(p) => setEnv((s) => ({ ...s, ...p }))} />

        <AF_Footer />

        {ReactDOM.createPortal(
          <TweaksPanel>
            <TweakSection label="Surface finish" />
            <TweakRadio label="Finish" value={t.surface} options={["clean", "worn"]}
              onChange={(v) => setTweak("surface", v)} />
            <TweakSlider label="Grain" value={t.grain} min={0} max={100} step={5}
              onChange={(v) => setTweak("grain", v)} />
            <TweakSlider label="Wear" value={t.wear} min={0} max={100} step={5}
              onChange={(v) => setTweak("wear", v)} />
            <TweakSection label="Lighting" />
            <TweakSlider label="Gradient" value={t.gradient} min={0} max={100} step={5}
              onChange={(v) => setTweak("gradient", v)} />
          </TweaksPanel>,
          document.body
        )}
      </div>
    );
  }

  ReactDOM.createRoot(document.getElementById("root")).render(<App />);
})();
