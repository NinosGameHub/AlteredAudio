import React from "react";

/**
 * Readout — the dark, amber-text inset value box used across the AF-74
 * (preset name, oversampling "4x", knob values, meter dB numbers). A
 * near-black well with monospace amber digits. Optional uppercase label
 * sits to the left or above.
 */
export function Readout({
  value,
  label,
  labelPlacement = "left", // "left" | "top" | "none"
  align = "center",
  size = "md",             // "sm" | "md" | "lg"
  tone = "amber",          // "amber" | "muted"
  className = "",
  style = {},
  ...rest
}) {
  const sizes = {
    sm: { fs: "var(--fs-11)", pad: "3px 8px", min: "44px" },
    md: { fs: "var(--fs-13)", pad: "5px 12px", min: "60px" },
    lg: { fs: "var(--fs-22)", pad: "8px 16px", min: "92px" },
  };
  const s = sizes[size] || sizes.md;
  const color = tone === "muted" ? "rgba(240,181,71,0.55)" : "var(--text-on-display)";

  const box = (
    <span
      className="af-readout-box"
      style={{
        display: "inline-block",
        fontFamily: "var(--font-mono)",
        fontWeight: "var(--fw-medium)",
        fontSize: s.fs,
        letterSpacing: "var(--tracking-mono)",
        color,
        background: "var(--surface-display)",
        padding: s.pad,
        minWidth: s.min,
        textAlign: align,
        borderRadius: "var(--radius-xs)",
        boxShadow: "var(--inset-readout)",
        whiteSpace: "nowrap",
      }}
    >
      {value}
    </span>
  );

  if (!label || labelPlacement === "none") {
    return <span className={`af-readout ${className}`} style={style} {...rest}>{box}</span>;
  }

  return (
    <span
      className={`af-readout ${className}`}
      style={{
        display: "inline-flex",
        flexDirection: labelPlacement === "top" ? "column" : "row",
        alignItems: labelPlacement === "top" ? "flex-start" : "center",
        gap: labelPlacement === "top" ? "5px" : "10px",
        ...style,
      }}
      {...rest}
    >
      <span className="af-label">{label}</span>
      {box}
    </span>
  );
}
