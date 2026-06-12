// BottomRow.jsx — MODULATION · LFO ENGINE · ENVELOPE FOLLOWER · UTILITY.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { Panel, Select, Knob, Oscilloscope, Button, Meter } = DS;
  const D = window.AF_DATA;

  const WaveButton = ({ kind, active, onClick }) => {
    const g = window.AF_WAVE_GLYPH(kind, active);
    return (
      <Button variant="toggle" active={active} square onClick={onClick}
        icon={<svg width="20" height="17" viewBox="0 0 20 17" aria-hidden="true">
          <path d={g.d} fill="none" stroke={g.stroke} strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round" />
        </svg>} />
    );
  };

  function Modulation({ mod, set }) {
    return (
      <Panel title="MODULATION" led style={{ flex: "1.05 1 0" }}>
        <div style={{ display: "flex", gap: "20px", alignItems: "flex-end" }}>
          <div style={{ display: "flex", flexDirection: "column", gap: "12px", flex: 1 }}>
            <Select label="SOURCE" value={mod.source} onChange={(v) => set({ source: v })} options={D.lfoSources} style={{ width: "100%" }} />
            <Select label="DESTINATION" value={mod.dest} onChange={(v) => set({ dest: v })} options={D.destinations} style={{ width: "100%" }} />
          </div>
          <Knob label="AMOUNT" value={mod.amount} min={-100} max={100} defaultValue={0} bipolar
            size="var(--knob-md)" ticks={28} display={(mod.amount >= 0 ? "+" : "") + Math.round(mod.amount) + " %"}
            minLabel="−100%" maxLabel="+100%" onChange={(v) => set({ amount: v })} />
        </div>
      </Panel>
    );
  }

  function LFOEngine({ lfo, set }) {
    return (
      <Panel title="LFO ENGINE" style={{ flex: "1.2 1 0" }}>
        <div style={{ display: "flex", gap: "6px", marginBottom: "10px" }}>
          {D.waveforms.map((w) => (
            <WaveButton key={w} kind={w} active={lfo.waveform === w} onClick={() => set({ waveform: w })} />
          ))}
        </div>
        <Oscilloscope waveform={lfo.waveform} cycles={3} height={52} />
        <div style={{ display: "flex", justifyContent: "space-around", marginTop: "12px" }}>
          <Knob label="RATE" value={lfo.rate} min={0.01} max={20} defaultValue={1} size="var(--knob-md)" ticks={28}
            display={lfo.rate.toFixed(2) + " Hz"} minLabel="0.01" maxLabel="20.0" onChange={(v) => set({ rate: v })} />
          <Knob label="DEPTH" value={lfo.depth} min={0} max={100} defaultValue={37} size="var(--knob-md)" ticks={28}
            display={Math.round(lfo.depth) + " %"} minLabel="0" maxLabel="100" onChange={(v) => set({ depth: v })} />
          <Knob label="PHASE" value={lfo.phase} min={-180} max={180} defaultValue={0} bipolar size="var(--knob-md)" ticks={28}
            display={Math.round(lfo.phase) + "°"} minLabel="−180" maxLabel="+180" onChange={(v) => set({ phase: v })} />
        </div>
      </Panel>
    );
  }

  function Envelope({ env, set }) {
    return (
      <Panel title="ENVELOPE FOLLOWER" led style={{ flex: "1.2 1 0" }}>
        <Oscilloscope waveform="envelope" height={66} />
        <div style={{ display: "flex", justifyContent: "space-around", marginTop: "12px" }}>
          <Knob label="ATTACK" value={env.attack} min={1} max={1000} defaultValue={10} size="var(--knob-md)" ticks={28}
            display={Math.round(env.attack) + " ms"} minLabel="1" maxLabel="1k" onChange={(v) => set({ attack: v })} />
          <Knob label="RELEASE" value={env.release} min={10} max={5000} defaultValue={200} size="var(--knob-md)" ticks={28}
            display={Math.round(env.release) + " ms"} minLabel="10" maxLabel="5k" onChange={(v) => set({ release: v })} />
          <Knob label="SENSITIVITY" value={env.sens} min={0} max={100} defaultValue={50} size="var(--knob-md)" ticks={28}
            display={Math.round(env.sens) + " %"} minLabel="0" maxLabel="100" onChange={(v) => set({ sens: v })} />
        </div>
      </Panel>
    );
  }

  function Utility({ util, set, inL, inR }) {
    return (
      <Panel title="UTILITY" led style={{ flex: "1 1 0" }}>
        <div style={{ display: "flex", gap: "18px", alignItems: "flex-start" }}>
          <Meter title="INPUT" height={116} showReadout
            channels={[
              { value: inL, label: "L", readout: ((inL - 1) * 24).toFixed(1) },
              { value: inR, label: "R", readout: ((inR - 1) * 24).toFixed(1) },
            ]} />
          <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "14px", flex: 1 }}>
            <Knob label="DRY / WET" value={util.drywet} min={0} max={100} defaultValue={100} size="var(--knob-md)" ticks={28}
              display={Math.round(util.drywet) + " %"} minLabel="0" maxLabel="100" onChange={(v) => set({ drywet: v })} />
            <div style={{ display: "flex", gap: "16px" }}>
              <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "6px" }}>
                <span className="af-label" style={{ fontSize: "9px" }}>SOLO</span>
                <Button square variant="toggle" active={util.solo} onClick={() => set({ solo: !util.solo })}>S</Button>
              </div>
              <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "6px" }}>
                <span className="af-label" style={{ fontSize: "9px" }}>BYPASS</span>
                <Button square variant="toggle" active={util.bypass} onClick={() => set({ bypass: !util.bypass })}>B</Button>
              </div>
            </div>
          </div>
        </div>
      </Panel>
    );
  }

  window.AF_BottomRow = function BottomRow({ mod, setMod, lfo, setLfo, env, setEnv, util, setUtil, inL, inR }) {
    return (
      <div style={{ display: "flex", gap: "14px", alignItems: "stretch" }}>
        <Modulation mod={mod} set={setMod} />
        <LFOEngine lfo={lfo} set={setLfo} />
        <Envelope env={env} set={setEnv} />
        <Utility util={util} set={setUtil} inL={inL} inR={inR} />
      </div>
    );
  };
})();
