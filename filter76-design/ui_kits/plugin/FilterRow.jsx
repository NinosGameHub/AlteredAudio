// FilterRow.jsx — FILTER TYPE list · primary knob cluster · SLOPE / MODE.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { Panel, OptionList, Knob } = DS;
  const D = window.AF_DATA;

  const fmtHz = (f) => (f >= 1000 ? (f / 1000).toFixed(2) + " kHz" : Math.round(f) + " Hz");

  function FilterRow({ filter, set }) {
    return (
      <div style={{ display: "flex", gap: "14px", alignItems: "stretch" }}>
        <Panel title="FILTER TYPE" style={{ flex: "0 0 150px" }}>
          <OptionList value={filter.type} onChange={(v) => set({ type: v })} options={D.filterTypes} />
        </Panel>

        <Panel style={{ flex: 1, display: "flex", alignItems: "center", justifyContent: "space-around", padding: "22px 16px" }}>
          <Knob face label="FREQ · HZ" value={filter.freq} min={20} max={20000} defaultValue={1000}
            display={Math.round(filter.freq)} onChange={(v) => set({ freq: v })} />
          <Knob face label="RES · Q" value={filter.q} min={0.1} max={24} defaultValue={0.7}
            display={filter.q.toFixed(1)} onChange={(v) => set({ q: v })} />
          <Knob face label="DRIVE · DB" value={filter.drive} min={0} max={24} defaultValue={0}
            display={filter.drive.toFixed(1)} onChange={(v) => set({ drive: v })} />
          <Knob face label="MIX · %" value={filter.mix} min={0} max={100} defaultValue={100}
            display={Math.round(filter.mix)} onChange={(v) => set({ mix: v })} />
          <Knob face label="OUT · DB" value={filter.output} min={-24} max={24} defaultValue={0} bipolar
            display={filter.output.toFixed(1)} onChange={(v) => set({ output: v })} />
        </Panel>

        <Panel title="SLOPE" style={{ flex: "0 0 150px" }}>
          <OptionList dense value={filter.slope} onChange={(v) => set({ slope: v })}
            options={D.slopes.map((s) => ({ value: s, label: s + " dB" }))} />
          <div className="af-section-label" style={{ marginTop: "16px", marginBottom: "10px", fontSize: "11px" }}>MODE</div>
          <OptionList dense value={filter.mode} onChange={(v) => set({ mode: v })} options={D.modes} />
        </Panel>
      </div>
    );
  }

  window.AF_FilterRow = FilterRow;
})();
