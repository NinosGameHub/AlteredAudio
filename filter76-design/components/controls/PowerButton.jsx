import React from "react";

/**
 * PowerButton — the global power / bypass control in the header. A round
 * cream key with the standard power glyph and a small amber status LED
 * below it that glows when engaged (on) and goes dark when bypassed.
 */
export function PowerButton({
  on = true,
  onChange,
  size = 38,
  disabled = false,
  className = "",
  ...rest
}) {
  return (
    <button
      type="button"
      className={`af-power ${className}`}
      role="switch"
      aria-checked={on}
      disabled={disabled}
      onClick={() => !disabled && onChange && onChange(!on)}
      style={{
        display: "inline-flex",
        flexDirection: "column",
        alignItems: "center",
        gap: "5px",
        background: "transparent",
        border: "none",
        padding: 0,
        cursor: disabled ? "default" : "pointer",
        opacity: disabled ? 0.45 : 1,
      }}
      {...rest}
    >
      <span
        style={{
          width: `${size}px`,
          height: `${size}px`,
          display: "inline-flex",
          alignItems: "center",
          justifyContent: "center",
          borderRadius: "50%",
          background: "var(--knob-bevel)",
          boxShadow: "var(--knob-shadow)",
          transition: "box-shadow var(--dur-fast)",
        }}
      >
        <svg width={size * 0.5} height={size * 0.5} viewBox="0 0 24 24" aria-hidden="true">
          <path d="M12 3 L12 11" fill="none"
            stroke={on ? "var(--text-primary)" : "var(--text-muted)"}
            strokeWidth="2.4" strokeLinecap="round" />
          <path d="M7.0 6.4 A 8 8 0 1 0 17 6.4" fill="none"
            stroke={on ? "var(--text-primary)" : "var(--text-muted)"}
            strokeWidth="2.4" strokeLinecap="round" />
        </svg>
      </span>
      <span
        aria-hidden="true"
        style={{
          width: "6px",
          height: "6px",
          borderRadius: "50%",
          background: on ? "var(--state-engaged)" : "var(--state-bypassed)",
          boxShadow: on ? "var(--led-glow)" : "none",
          transition: "background var(--dur-fast), box-shadow var(--dur-fast)",
        }}
      />
    </button>
  );
}
