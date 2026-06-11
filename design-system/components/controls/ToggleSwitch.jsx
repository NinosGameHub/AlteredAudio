import React from "react";

/**
 * ToggleSwitch — iOS-style pill switch from AaLookAndFeel::drawToggleButton.
 * 32×18 track, green when on / hairline grey when off, white thumb that
 * slides. Optional trailing label (e.g. "Active", "Ping-Pong", "Auto Gain").
 */
export function ToggleSwitch({
  checked = false,
  onChange,
  label,
  disabled = false,
  className = "",
  ...rest
}) {
  const W = 32;
  const H = 18;
  const pad = 2;
  const thumb = H - 4;

  return (
    <label
      className={`aa-toggle ${className}`}
      style={{
        display: "inline-flex",
        alignItems: "center",
        gap: "8px",
        cursor: disabled ? "default" : "pointer",
        opacity: disabled ? 0.5 : 1,
        userSelect: "none",
      }}
      {...rest}
    >
      <span
        role="switch"
        aria-checked={checked}
        onClick={() => !disabled && onChange && onChange(!checked)}
        style={{
          position: "relative",
          flex: "0 0 auto",
          width: `${W}px`,
          height: `${H}px`,
          borderRadius: "var(--radius-pill)",
          background: checked ? "var(--state-active)" : "var(--aa-surface-alt)",
          border: checked ? "1px solid var(--state-active)" : "1px solid var(--border-default)",
          transition: "background var(--dur-normal) var(--ease-standard), border-color var(--dur-normal)",
        }}
      >
        <span
          style={{
            position: "absolute",
            top: `${pad}px`,
            left: checked ? `${W - thumb - pad}px` : `${pad}px`,
            width: `${thumb}px`,
            height: `${thumb}px`,
            borderRadius: "var(--radius-pill)",
            background: "var(--surface-card)",
            transition: "left var(--dur-normal) var(--ease-standard)",
          }}
        />
      </span>
      {label ? (
        <span
          style={{
            fontSize: "var(--fs-11)",
            color: "var(--text-body)",
          }}
        >
          {label}
        </span>
      ) : null}
    </label>
  );
}
