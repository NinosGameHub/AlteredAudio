// Header.jsx — AF-74 top band: wordmark, preset transport, A/B, oversampling, mix, power.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { Readout, SegmentedControl, Button, PowerButton } = DS;

  const Chevron = ({ dir }) => (
    <svg width="11" height="13" viewBox="0 0 11 13" aria-hidden="true">
      <path d={dir === "left" ? "M7.5 2 L3 6.5 L7.5 11" : "M3.5 2 L8 6.5 L3.5 11"}
        fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  );

  function Header({ presetIndex, presetName, onPrev, onNext, ab, onAb, oversampling, mix, power, onPower }) {
    const num = String(presetIndex + 1).padStart(3, "0");
    return (
      <header style={{ display: "flex", alignItems: "center", gap: "24px", padding: "0 6px", height: "64px" }}>
        {/* wordmark */}
        <div style={{ display: "flex", flexDirection: "column", gap: "5px", flex: "0 0 auto" }}>
          <div className="af-wordmark" style={{ fontSize: "26px", lineHeight: 1 }}>ALTERED AUDIO</div>
          <div style={{ fontFamily: "var(--font-display)", fontSize: "12px", letterSpacing: "var(--tracking-wide)",
            textTransform: "uppercase", color: "var(--accent-amber-active)" }}>FILTER 76</div>
        </div>

        {/* preset transport */}
        <div style={{ display: "flex", flexDirection: "column", gap: "5px", flex: "0 1 auto" }}>
          <span className="af-label" style={{ textAlign: "center", fontSize: "9px" }}>PRESET</span>
          <div style={{ display: "flex", alignItems: "center", gap: "6px" }}>
            <Button square size="sm" onClick={onPrev} icon={<Chevron dir="left" />} />
            <span style={{ fontFamily: "var(--font-mono)", fontSize: "13px", fontWeight: 500, letterSpacing: ".04em",
              color: "var(--text-on-display)", background: "var(--surface-display)", boxShadow: "var(--inset-readout)",
              borderRadius: "var(--radius-xs)", padding: "7px 18px", minWidth: "260px", textAlign: "center" }}>
              <span style={{ opacity: 0.6, marginRight: "12px" }}>{num}</span>{presetName}
            </span>
            <Button square size="sm" onClick={onNext} icon={<Chevron dir="right" />} />
          </div>
        </div>

        {/* right cluster */}
        <div style={{ display: "flex", alignItems: "center", gap: "26px", marginLeft: "auto" }}>
          <SegmentedControl value={ab} onChange={onAb} options={["A", "B"]} />
          <Readout label="OVERSAMPLING" value={oversampling} size="sm" />
          <Readout label="MIX" value={mix} size="sm" />
          <PowerButton on={power} onChange={onPower} />
        </div>
      </header>
    );
  }

  window.AF_Header = Header;
})();
