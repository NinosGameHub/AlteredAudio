import React from "react";

/**
 * OptionList — the vertical, LED-marked selector stack used for FILTER
 * TYPE (LP/HP/BP/NOTCH/PEAK/SHELF), SLOPE (12/24/36/48 dB) and MODE
 * (ANALOG/CLEAN). Each row is a flat cream button with a small indicator
 * LED at the left — amber and glowing when selected, dark ring when not.
 */
export function OptionList({
  value,
  onChange,
  options = [],
  disabled = false,
  dense = false,
  className = "",
  ...rest
}) {
  const opts = options.map((o) => (typeof o === "string" ? { value: o, label: o } : o));

  return (
    <div
      className={`af-optionlist ${className}`}
      role="radiogroup"
      style={{ display: "flex", flexDirection: "column", gap: "6px", opacity: disabled ? 0.5 : 1 }}
      {...rest}
    >
      {opts.map((o) => {
        const sel = o.value === value;
        return (
          <button
            key={o.value}
            type="button"
            role="radio"
            aria-checked={sel}
            disabled={disabled}
            onClick={() => !disabled && onChange && onChange(o.value)}
            style={{
              display: "flex",
              alignItems: "center",
              gap: "10px",
              height: dense ? "26px" : "30px",
              padding: "0 12px",
              fontFamily: "var(--font-mono)",
              fontSize: "var(--fs-11)",
              fontWeight: "var(--fw-medium)",
              letterSpacing: "var(--tracking-label)",
              textTransform: "uppercase",
              color: "var(--text-primary)",
              background: sel ? "var(--surface-raised)" : "transparent",
              border: sel ? "1px solid var(--c-border)" : "1px solid var(--c-border-soft)",
              borderRadius: "var(--radius-md)",
              cursor: disabled ? "default" : "pointer",
              textAlign: "left",
              transition: "background var(--dur-fast) var(--ease-standard), border-color var(--dur-fast)",
              boxShadow: sel ? "var(--shadow-raised)" : "none",
            }}
          >
            <span
              aria-hidden="true"
              style={{
                flex: "0 0 auto",
                width: "9px",
                height: "9px",
                borderRadius: "50%",
                background: sel ? "var(--state-engaged)" : "transparent",
                border: sel ? "1px solid var(--accent-amber-active)" : "1px solid var(--state-bypassed)",
                boxShadow: sel ? "var(--led-glow)" : "none",
                transition: "background var(--dur-fast), box-shadow var(--dur-fast)",
              }}
            />
            {o.label}
          </button>
        );
      })}
    </div>
  );
}
