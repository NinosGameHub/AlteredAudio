import React from "react";

/**
 * Select — flat combo box from AaLookAndFeel::drawComboBox.
 * White fill, 6px radius, hairline border, a 1.5px chevron in secondary
 * grey at the right. Optional uppercase label above.
 */
export function Select({
  value,
  onChange,
  options = [],
  label,
  disabled = false,
  className = "",
  ...rest
}) {
  const opts = options.map((o) =>
    typeof o === "string" ? { value: o, label: o } : o
  );

  return (
    <div
      className={`aa-select ${className}`}
      style={{ display: "inline-flex", flexDirection: "column", gap: "4px" }}
    >
      {label ? (
        <span
          style={{
            fontSize: "var(--fs-10)",
            fontWeight: "var(--fw-semibold)",
            letterSpacing: "var(--tracking-label)",
            textTransform: "uppercase",
            color: "var(--text-muted)",
          }}
        >
          {label}
        </span>
      ) : null}
      <div style={{ position: "relative", display: "inline-block" }}>
        <select
          value={value}
          disabled={disabled}
          onChange={(e) => onChange && onChange(e.target.value)}
          style={{
            appearance: "none",
            WebkitAppearance: "none",
            MozAppearance: "none",
            width: "100%",
            height: "var(--h-control)",
            padding: "0 28px 0 10px",
            fontFamily: "var(--font-system)",
            fontSize: "var(--fs-13)",
            color: "var(--text-body)",
            background: "var(--surface-card)",
            border: "var(--border-hairline)",
            borderRadius: "var(--radius-sm)",
            cursor: disabled ? "default" : "pointer",
            opacity: disabled ? 0.5 : 1,
            outline: "none",
          }}
          {...rest}
        >
          {opts.map((o) => (
            <option key={o.value} value={o.value}>
              {o.label}
            </option>
          ))}
        </select>
        <svg
          width="10"
          height="10"
          viewBox="0 0 10 10"
          aria-hidden="true"
          style={{
            position: "absolute",
            right: "10px",
            top: "50%",
            transform: "translateY(-50%)",
            pointerEvents: "none",
          }}
        >
          <path
            d="M1 3.5 L5 7 L9 3.5"
            fill="none"
            stroke="var(--text-muted)"
            strokeWidth="1.5"
            strokeLinecap="round"
            strokeLinejoin="round"
          />
        </svg>
      </div>
    </div>
  );
}
