import React from "react";

/**
 * Panel — a module enclosure. A flat warm-plastic surface with a hairline
 * border and a tight 6px radius; an optional uppercase title row with a
 * small status LED (MODULATION •, ENVELOPE FOLLOWER •, UTILITY •) and
 * optional right-aligned header actions.
 */
export function Panel({
  title,
  led,                 // undefined = no LED; true = engaged; false = bypassed
  actions,
  inset = false,       // recessed beige instead of base
  padding,
  children,
  className = "",
  style = {},
  ...rest
}) {
  return (
    <section
      className={`af-panel ${className}`}
      style={{
        background: inset ? "var(--surface-panel)" : "var(--surface-base)",
        border: "var(--border-default)",
        borderRadius: "var(--radius-lg)",
        boxShadow: "var(--shadow-panel)",
        padding: padding != null ? padding : "var(--panel-pad)",
        ...style,
      }}
      {...rest}
    >
      {title != null ? (
        <header
          style={{
            display: "flex",
            alignItems: "center",
            gap: "8px",
            marginBottom: "16px",
          }}
        >
          {led !== undefined ? (
            <span
              aria-hidden="true"
              style={{
                width: "7px", height: "7px", borderRadius: "50%",
                background: led ? "var(--state-engaged)" : "var(--state-bypassed)",
                boxShadow: led ? "var(--led-glow-soft)" : "none",
                flex: "0 0 auto",
              }}
            />
          ) : null}
          <span className="af-section-label" style={{ color: "var(--text-secondary)" }}>{title}</span>
          {actions ? <span style={{ marginLeft: "auto", display: "inline-flex", alignItems: "center", gap: "8px" }}>{actions}</span> : null}
        </header>
      ) : null}
      {children}
    </section>
  );
}
