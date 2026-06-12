// DisplayRow.jsx — the dominant spectrum display + tabs, and the OUTPUT meter.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { SpectrumDisplay, SegmentedControl, Meter } = DS;

  function DisplayRow({ filter, onNodeChange, tab, onTab, outL, outR }) {
    return (
      <div style={{ display: "flex", gap: "14px", height: "300px" }}>
        <div style={{ position: "relative", flex: 1 }}>
          <SpectrumDisplay
            type={filter.type} freq={filter.freq} q={filter.q} gain={filter.gain}
            slope={Number(filter.slope)} height="100%"
            showSpectrum={tab !== "RESPONSE"} showNode={tab !== "SPECTRUM"}
            onNodeChange={onNodeChange} />
          <div style={{ position: "absolute", top: "12px", left: "14px" }}>
            <SegmentedControl tone="display" size="sm" value={tab} onChange={onTab}
              options={["SPECTRUM", "RESPONSE", "NODE"]} />
          </div>
        </div>

        <section style={{
          flex: "0 0 168px", display: "flex", flexDirection: "column", alignItems: "center", gap: "14px",
          background: "var(--surface-base)", border: "var(--border-default)", borderRadius: "var(--radius-lg)",
          boxShadow: "var(--shadow-panel)", padding: "16px 12px",
        }}>
          <span className="af-section-label">OUTPUT</span>
          <Meter scale={[24, 12, 0, -12, -24]} height={196}
            channels={[
              { value: outL, label: "L", readout: ((outL - 1) * 24).toFixed(1) },
              { value: outR, label: "R", readout: ((outR - 1) * 24).toFixed(1) },
            ]} />
        </section>
      </div>
    );
  }

  window.AF_DisplayRow = DisplayRow;
})();
