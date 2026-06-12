import React from "react";

/**
 * Button — the AF-74 text / utility button. Variants:
 *  - "default": flat cream face, hairline border (transport, SOLO/BYPASS).
 *  - "toggle":  amber-filled when `active` (engaged utility toggles).
 *  - "ghost":   transparent, secondary text (low-emphasis).
 * `square` makes an equal-side button (the S / B utility keys); `icon`
 * renders a leading glyph.
 */
export function Button({
  children,
  variant = "default",
  active = false,
  square = false,
  size = "md",     // "sm" | "md"
  disabled = false,
  icon,
  onClick,
  className = "",
  style = {},
  ...rest
}) {
  const h = size === "sm" ? "var(--h-control-sm)" : "var(--h-control)";
  const fill =
    variant === "toggle" && active ? "var(--accent-amber)"
    : variant === "ghost" ? "transparent"
    : "var(--surface-raised)";
  const fg =
    variant === "toggle" && active ? "var(--text-on-accent)"
    : variant === "ghost" ? "var(--text-secondary)"
    : "var(--text-primary)";
  const border =
    variant === "ghost" ? "1px solid transparent"
    : (variant === "toggle" && active) ? "1px solid var(--accent-amber-active)"
    : "var(--border-default)";

  return (
    <button
      type="button"
      className={`af-button af-button--${variant} ${className}`}
      disabled={disabled}
      onClick={onClick}
      aria-pressed={variant === "toggle" ? active : undefined}
      style={{
        display: "inline-flex",
        alignItems: "center",
        justifyContent: "center",
        gap: "7px",
        height: h,
        width: square ? h : "auto",
        padding: square ? 0 : "0 14px",
        fontFamily: "var(--font-mono)",
        fontSize: "var(--fs-11)",
        fontWeight: "var(--fw-medium)",
        letterSpacing: "var(--tracking-label)",
        textTransform: "uppercase",
        color: fg,
        background: fill,
        border,
        borderRadius: "var(--radius-sm)",
        cursor: disabled ? "default" : "pointer",
        opacity: disabled ? 0.45 : 1,
        boxShadow: variant === "default" ? "var(--shadow-raised)" : "none",
        transition: "background var(--dur-fast) var(--ease-standard), color var(--dur-fast), border-color var(--dur-fast)",
        whiteSpace: "nowrap",
        ...style,
      }}
      {...rest}
    >
      {icon ? <span style={{ display: "inline-flex" }} aria-hidden="true">{icon}</span> : null}
      {children}
    </button>
  );
}
