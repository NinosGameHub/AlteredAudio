// App.jsx — the full Aurora Filter AF-74 MK III plugin.
(function () {
  const D = window.AF_DATA;
  const { AF_Header, AF_DisplayRow, AF_FilterRow, AF_BottomRow, AF_Footer } = window;

  function App() {
    const [filter, setFilter] = React.useState({
      type: "LP", freq: 824, q: 3.2, drive: 4.1, mix: 100, output: 0, slope: "12", mode: "ANALOG",
    });
    const [tab, setTab] = React.useState("SPECTRUM");
    const [presetIndex, setPresetIndex] = React.useState(0);
    const [ab, setAb] = React.useState("A");
    const [power, setPower] = React.useState(true);
    const [path, setPath] = React.useState("STEREO");
    const [mod, setMod] = React.useState({ source: "LFO A", dest: "FREQUENCY", amount: 37 });
    const [lfo, setLfo] = React.useState({ waveform: "sine", rate: 0.57, depth: 37, phase: 0 });
    const [env, setEnv] = React.useState({ attack: 10, release: 200, sens: 50 });
    const [util, setUtil] = React.useState({ drywet: 100, solo: false, bypass: false });
    const [meters, setMeters] = React.useState({ outL: 0.74, outR: 0.7, inL: 0.82, inR: 0.79 });

    const setF = (p) => setFilter((s) => ({ ...s, ...p }));

    // gentle meter animation
    React.useEffect(() => {
      const id = setInterval(() => {
        const j = () => (Math.random() - 0.5) * 0.12;
        setMeters((m) => ({
          outL: Math.max(0.2, Math.min(0.98, m.outL + j())),
          outR: Math.max(0.2, Math.min(0.98, m.outR + j())),
          inL: Math.max(0.2, Math.min(0.98, m.inL + j())),
          inR: Math.max(0.2, Math.min(0.98, m.inR + j())),
        }));
      }, 140);
      return () => clearInterval(id);
    }, []);

    const stepPreset = (d) => setPresetIndex((i) => (i + d + D.presets.length) % D.presets.length);

    return (
      <div id="plugin">
        <AF_Header
          presetIndex={presetIndex} presetName={D.presets[presetIndex]}
          onPrev={() => stepPreset(-1)} onNext={() => stepPreset(1)}
          ab={ab} onAb={setAb} oversampling="4x" mix={Math.round(util.drywet) + "%"}
          power={power} onPower={setPower} />

        <AF_DisplayRow
          filter={filter} tab={tab} onTab={setTab}
          onNodeChange={(n) => setF({ freq: n.freq, q: n.q })}
          outL={meters.outL} outR={meters.outR} />

        <AF_FilterRow filter={filter} set={setF} />

        <AF_BottomRow
          mod={mod} setMod={(p) => setMod((s) => ({ ...s, ...p }))}
          lfo={lfo} setLfo={(p) => setLfo((s) => ({ ...s, ...p }))}
          env={env} setEnv={(p) => setEnv((s) => ({ ...s, ...p }))}
          util={util} setUtil={(p) => setUtil((s) => ({ ...s, ...p }))}
          inL={meters.inL} inR={meters.inR} />

        <AF_Footer path={path} onPath={setPath} />
      </div>
    );
  }

  ReactDOM.createRoot(document.getElementById("root")).render(<App />);
})();
