import React from "react";

/**
 * Select — flat combo box (MODULATION source / destination). Warm cream
 * fill, hairline border, a 1.5px chevron in secondary grey. Optional
 * uppercase label above.
 */
export function Select({
  value,
  onChange,
  options = [],
  label,
  disabled = false,
  className = "",
  style = {},
  ...rest
}) {
  const opts = options.map((o) => (typeof o === "string" ? { value: o, label: o } : o));

  return (
    <div
      className={`af-select ${className}`}
      style={{ display: "inline-flex", flexDirection: "column", gap: "6px", ...style }}
    >
      {label ? <span className="af-label">{label}</span> : null}
      <div style={{ position: "relative", display: "inline-block" }}>
        <select
          value={value}
          disabled={disabled}
          onChange={(e) => onChange && onChange(e.target.value)}
          style={{
            appearance: "none", WebkitAppearance: "none", MozAppearance: "none",
            width: "100%",
            height: "var(--h-control)",
            padding: "0 32px 0 12px",
            fontFamily: "var(--font-mono)",
            fontSize: "var(--fs-12)",
            fontWeight: "var(--fw-medium)",
            letterSpacing: "var(--tracking-label)",
            textTransform: "uppercase",
            color: "var(--text-primary)",
            background: "var(--surface-raised)",
            border: "var(--border-default)",
            borderRadius: "var(--radius-sm)",
            cursor: disabled ? "default" : "pointer",
            opacity: disabled ? 0.5 : 1,
            outline: "none",
          }}
          {...rest}
        >
          {opts.map((o) => (
            <option key={o.value} value={o.value}>{o.label}</option>
          ))}
        </select>
        <svg width="11" height="11" viewBox="0 0 11 11" aria-hidden="true"
          style={{ position: "absolute", right: "11px", top: "50%", transform: "translateY(-50%)", pointerEvents: "none" }}>
          <path d="M1.5 4 L5.5 7.5 L9.5 4" fill="none" stroke="var(--text-secondary)"
            strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" />
        </svg>
      </div>
    </div>
  );
}
