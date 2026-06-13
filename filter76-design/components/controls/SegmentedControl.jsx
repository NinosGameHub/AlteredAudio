import React from "react";

/**
 * SegmentedControl — a horizontal row of mutually-exclusive options
 * (A/B compare, signal-path MONO/STEREO/MID·SIDE, display tabs). The
 * selected segment fills amber with dark text on a panel; on the dark
 * display (`tone="display"`) the selected segment fills cream.
 */
export function SegmentedControl({
  value,
  onChange,
  options = [],
  tone = "panel",   // "panel" | "display"
  size = "md",      // "sm" | "md"
  disabled = false,
  className = "",
  ...rest
}) {
  const opts = options.map((o) => (typeof o === "string" ? { value: o, label: o } : o));
  const h = size === "sm" ? "var(--h-control-sm)" : "var(--h-control)";
  const onDisplay = tone === "display";

  return (
    <div
      className={`af-segmented ${className}`}
      role="radiogroup"
      style={{
        display: "inline-flex",
        padding: "2px",
        gap: "2px",
        background: onDisplay ? "rgba(0,0,0,0.35)" : "var(--surface-sunken)",
        border: onDisplay ? "1px solid rgba(240,181,71,0.18)" : "var(--border-default)",
        borderRadius: "var(--radius-sm)",
        opacity: disabled ? 0.5 : 1,
      }}
      {...rest}
    >
      {opts.map((o) => {
        const sel = o.value === value;
        const selBg = onDisplay ? "var(--surface-raised)" : "var(--accent-amber)";
        const selFg = onDisplay ? "var(--text-primary)" : "var(--text-on-accent)";
        const idleFg = onDisplay ? "var(--text-on-display)" : "var(--text-secondary)";
        return (
          <button
            key={o.value}
            type="button"
            role="radio"
            aria-checked={sel}
            disabled={disabled}
            onClick={() => !disabled && onChange && onChange(o.value)}
            style={{
              height: h,
              padding: "0 14px",
              display: "inline-flex",
              alignItems: "center",
              justifyContent: "center",
              fontFamily: "var(--font-mono)",
              fontSize: "var(--fs-11)",
              fontWeight: "var(--fw-medium)",
              letterSpacing: "var(--tracking-label)",
              textTransform: "uppercase",
              color: sel ? selFg : idleFg,
              background: sel ? selBg : "transparent",
              border: "none",
              borderRadius: "var(--radius-xs)",
              cursor: disabled ? "default" : "pointer",
              transition: "background var(--dur-fast) var(--ease-standard), color var(--dur-fast)",
              whiteSpace: "nowrap",
            }}
          >
            {o.label}
          </button>
        );
      })}
    </div>
  );
}
