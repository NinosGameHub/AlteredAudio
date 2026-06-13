// Footer.jsx — system status strip (v0.7.x: signal path is a text readout + version).
(function () {
  const DS = window.AuroraFilterDesignSystem_83b750;
  const { LED } = DS;

  const Stat = ({ label, value }) => (
    <span style={{ display: "inline-flex", alignItems: "baseline", gap: "10px" }}>
      <span className="af-label" style={{ fontSize: "9px" }}>{label}</span>
      <span style={{ fontFamily: "var(--font-mono)", fontSize: "12px", fontWeight: 500, color: "var(--text-primary)" }}>{value}</span>
    </span>
  );

  function Footer() {
    const D = window.AF_DATA;
    return (
      <footer style={{
        display: "flex", alignItems: "center", gap: "30px", height: "40px", padding: "0 14px",
        background: "var(--surface-panel)", border: "var(--border-default)", borderRadius: "var(--radius-md)",
      }}>
        <LED on label="SYSTEM" />
        <Stat label="SAMPLE RATE" value="44.1 kHz" />
        <Stat label="OVERSAMPLING" value="1x" />
        <Stat label="LATENCY" value="0.00 ms" />
        <Stat label="CPU" value="0.5 %" />
        <Stat label="SIGNAL PATH" value="STEREO" />
        <span style={{ marginLeft: "auto", fontFamily: "var(--font-mono)", fontSize: "11px", color: "var(--text-muted)" }}>{D.version}</span>
      </footer>
    );
  }

  window.AF_Footer = Footer;
})();
