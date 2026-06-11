import React from "react";

/**
 * Card — the base white surface. Hairline border, 6px radius, no shadow
 * (the native plugin is flat). `selected` applies the core interaction
 * recipe: faint-blue fill, blue border, 8px radius.
 */
export function Card({
  children,
  selected = false,
  header,
  padding = "var(--space-4)",
  onClick,
  className = "",
  ...rest
}) {
  return (
    <div
      className={`aa-card ${selected ? "is-selected" : ""} ${className}`}
      onClick={onClick}
      style={{
        background: selected ? "var(--surface-selected)" : "var(--surface-card)",
        border: selected
          ? "1px solid var(--accent-primary)"
          : "var(--border-hairline)",
        borderRadius: selected ? "var(--radius-md)" : "var(--radius-sm)",
        overflow: "hidden",
        cursor: onClick ? "pointer" : "default",
        transition:
          "background var(--dur-normal) var(--ease-standard), border-color var(--dur-normal)",
        ...rest.style,
      }}
      {...rest}
    >
      {header ? (
        <div
          style={{
            height: "var(--h-panel-header)",
            display: "flex",
            alignItems: "center",
            padding: "0 var(--space-4)",
            borderBottom: "var(--border-hairline)",
            background: "var(--surface-card)",
            fontSize: "var(--fs-15)",
            fontWeight: "var(--fw-bold)",
            color: "var(--text-body)",
            letterSpacing: "var(--tracking-label)",
          }}
        >
          {header}
        </div>
      ) : null}
      <div style={{ padding }}>{children}</div>
    </div>
  );
}
