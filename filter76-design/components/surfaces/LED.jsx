import React from "react";

/**
 * LED — a small indicator lamp. Amber and glowing when `on`; a dark
 * recessed bezel when off. Optional trailing label. Used for engage
 * status, signal-present dots and the SYSTEM marker in the footer.
 */
export function LED({
  on = true,
  color = "var(--state-engaged)",
  size = 8,
  label,
  className = "",
  style = {},
  ...rest
}) {
  const dot = (
    <span
      aria-hidden="true"
      style={{
        width: `${size}px`,
        height: `${size}px`,
        borderRadius: "50%",
        background: on ? color : "var(--state-bypassed)",
        border: on ? "none" : "1px solid rgba(0,0,0,0.18)",
        boxShadow: on ? "var(--led-glow)" : "0 1px 1px rgba(0,0,0,0.15) inset",
        flex: "0 0 auto",
        transition: "background var(--dur-fast), box-shadow var(--dur-fast)",
      }}
    />
  );

  if (label == null) {
    return <span className={`af-led ${className}`} style={style} {...rest}>{dot}</span>;
  }
  return (
    <span
      className={`af-led ${className}`}
      style={{ display: "inline-flex", alignItems: "center", gap: "8px", ...style }}
      {...rest}
    >
      {dot}
      <span className="af-label">{label}</span>
    </span>
  );
}
