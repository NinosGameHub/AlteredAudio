// Footer.jsx — system status strip.
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { LED, SegmentedControl } = DS;

  const Stat = ({ label, value }) => (
    <span style={{ display: "inline-flex", alignItems: "baseline", gap: "8px" }}>
      <span className="af-label" style={{ fontSize: "9px" }}>{label}</span>
      <span style={{ fontFamily: "var(--font-mono)", fontSize: "12px", color: "var(--text-primary)" }}>{value}</span>
    </span>
  );

  function Footer({ path, onPath }) {
    return (
      <footer style={{
        display: "flex", alignItems: "center", gap: "28px", height: "40px", padding: "0 14px",
        background: "var(--surface-panel)", border: "var(--border-default)", borderRadius: "var(--radius-md)",
      }}>
        <LED on label="SYSTEM" />
        <Stat label="SAMPLE RATE" value="96.0 kHz" />
        <Stat label="OVERSAMPLING" value="4x" />
        <Stat label="LATENCY" value="0.23 ms" />
        <Stat label="CPU" value="1.7 %" />
        <div style={{ display: "flex", alignItems: "center", gap: "10px", marginLeft: "auto" }}>
          <span className="af-label" style={{ fontSize: "9px" }}>SIGNAL PATH</span>
          <SegmentedControl size="sm" value={path} onChange={onPath} options={["MONO", "STEREO", "MID/SIDE"]} />
        </div>
      </footer>
    );
  }

  window.AF_Footer = Footer;
})();
