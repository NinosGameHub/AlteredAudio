// DisplayRow.jsx — the dominant response display (with ghost curve) + OUTPUT meter.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { SpectrumDisplay, Meter } = DS;

  function DisplayRow({ filter, ghost, onNodeChange, outL, outR }) {
    return (
      <div style={{ display: "flex", gap: "14px", height: "330px" }}>
        <div style={{ flex: 1 }}>
          <SpectrumDisplay
            type={filter.type} freq={filter.freq} q={filter.q} gain={filter.gain}
            slope={Number(filter.slope)} height="100%"
            showSpectrum={false} ghost={ghost}
            onNodeChange={onNodeChange} />
        </div>

        <section style={{
          flex: "0 0 168px", display: "flex", flexDirection: "column", alignItems: "center", gap: "16px",
          background: "var(--surface-base)", border: "var(--border-default)", borderRadius: "var(--radius-lg)",
          boxShadow: "var(--shadow-panel)", padding: "16px 12px",
        }}>
          <span className="af-section-label" style={{ alignSelf: "flex-start" }}>OUTPUT</span>
          <Meter height={236} showReadout={false}
            channels={[
              { value: outL, label: "L" },
              { value: outR, label: "R" },
            ]} />
        </section>
      </div>
    );
  }

  window.AF_DisplayRow = DisplayRow;
})();
