// BottomRow.jsx — MODULATION · LFO ENGINE · ENVELOPE FOLLOWER (v0.7.x: no utility panel).
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { Panel, Select, Knob, Oscilloscope, Button } = DS;
  const D = window.AF_DATA;

  function Modulation({ mod, set }) {
    return (
      <Panel title="MODULATION" led style={{ flex: "1 1 0" }}>
        <div style={{ display: "flex", gap: "20px", alignItems: "flex-end" }}>
          <div style={{ display: "flex", flexDirection: "column", gap: "12px", flex: 1 }}>
            <Select label="SOURCE" value={mod.source} onChange={(v) => set({ source: v })} options={D.lfoSources} style={{ width: "100%" }} />
            <Select label="DESTINATION" value={mod.dest} onChange={(v) => set({ dest: v })} options={D.destinations} style={{ width: "100%" }} />
          </div>
          <Knob label="AMOUNT" value={mod.amount} min={-100} max={100} defaultValue={0} bipolar
            size="var(--knob-md)" ticks={28}
            display={(mod.amount >= 0 ? "+" : "") + mod.amount.toFixed(1) + " %"} onChange={(v) => set({ amount: v })} />
        </div>
      </Panel>
    );
  }

  function LFOEngine({ lfo, set }) {
    return (
      <Panel title="LFO ENGINE" style={{ flex: "1.25 1 0" }}>
        <div style={{ display: "flex", gap: "16px", alignItems: "flex-start" }}>
          <div style={{ flex: "1.1 1 0", display: "flex", flexDirection: "column", gap: "10px" }}>
            <div style={{ display: "flex", gap: "6px" }}>
              {D.waveforms.map((w) => (
                <Button key={w.id} size="sm" variant="toggle" active={lfo.waveform === w.id}
                  onClick={() => set({ waveform: w.id })}>{w.label}</Button>
              ))}
            </div>
            <Oscilloscope waveform={lfo.waveform} cycles={2} height={86} />
          </div>
          <div style={{ display: "flex", gap: "12px", flex: "1 0 auto" }}>
            <Knob label="RATE" value={lfo.rate} min={0.01} max={20} defaultValue={1} size="var(--knob-md)" ticks={28}
              display={lfo.rate.toFixed(1) + " Hz"} onChange={(v) => set({ rate: v })} />
            <Knob label="DEPTH" value={lfo.depth} min={0} max={100} defaultValue={100} size="var(--knob-md)" ticks={28}
              display={lfo.depth.toFixed(1) + " %"} onChange={(v) => set({ depth: v })} />
            <Knob label="PHASE" value={lfo.phase} min={-180} max={180} defaultValue={0} bipolar size="var(--knob-md)" ticks={28}
              display={lfo.phase.toFixed(1) + "°"} onChange={(v) => set({ phase: v })} />
          </div>
        </div>
      </Panel>
    );
  }

  function Envelope({ env, set }) {
    return (
      <Panel title="ENVELOPE FOLLOWER" led style={{ flex: "1.25 1 0" }}>
        <div style={{ display: "flex", gap: "16px", alignItems: "flex-start" }}>
          <div style={{ flex: "1.1 1 0" }}>
            <Oscilloscope waveform="envelope" height={118} />
          </div>
          <div style={{ display: "flex", gap: "12px", flex: "1 0 auto" }}>
            <Knob label="ATTACK" value={env.attack} min={0.1} max={500} defaultValue={2} size="var(--knob-md)" ticks={28}
              display={env.attack.toFixed(1) + " ms"} onChange={(v) => set({ attack: v })} />
            <Knob label="RELEASE" value={env.release} min={10} max={2000} defaultValue={120} size="var(--knob-md)" ticks={28}
              display={env.release.toFixed(1) + " ms"} onChange={(v) => set({ release: v })} />
            <Knob label="SENS" value={env.sens} min={0} max={10} defaultValue={1.4} size="var(--knob-md)" ticks={28}
              display={env.sens.toFixed(1)} onChange={(v) => set({ sens: v })} />
          </div>
        </div>
      </Panel>
    );
  }

  window.AF_BottomRow = function BottomRow({ mod, setMod, lfo, setLfo, env, setEnv }) {
    return (
      <div style={{ display: "flex", gap: "14px", alignItems: "stretch" }}>
        <Modulation mod={mod} set={setMod} />
        <LFOEngine lfo={lfo} set={setLfo} />
        <Envelope env={env} set={setEnv} />
      </div>
    );
  };
})();
