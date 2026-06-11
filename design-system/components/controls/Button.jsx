import React from "react";

/**
 * Button — Altered Audio text button.
 * variant: "primary" (filled Apple blue, white text), "secondary"
 * (sunken grey fill — mirrors the native TextButton), "ghost"
 * (transparent, accent text). Rounded 6px; Apple-style pill via `pill`.
 */
export function Button({
  children,
  variant = "secondary",
  size = "md",
  pill = false,
  disabled = false,
  iconLeft,
  onClick,
  className = "",
  ...rest
}) {
  const sizes = {
    sm: { h: 28, px: 12, fs: "var(--fs-11)" },
    md: { h: 34, px: 16, fs: "var(--fs-13)" },
    lg: { h: 44, px: 22, fs: "var(--fs-15)" },
  };
  const s = sizes[size] || sizes.md;

  const variants = {
    primary: {
      background: "var(--accent-primary)",
      color: "var(--text-on-accent)",
      border: "1px solid transparent",
    },
    secondary: {
      background: "var(--surface-sunken)",
      color: "var(--text-body)",
      border: "var(--border-hairline)",
    },
    ghost: {
      background: "transparent",
      color: "var(--accent-primary)",
      border: "1px solid transparent",
    },
  };
  const v = variants[variant] || variants.secondary;

  return (
    <button
      type="button"
      className={`aa-button aa-button--${variant} ${className}`}
      disabled={disabled}
      onClick={onClick}
      style={{
        display: "inline-flex",
        alignItems: "center",
        justifyContent: "center",
        gap: "6px",
        height: `${s.h}px`,
        padding: `0 ${s.px}px`,
        fontFamily: "var(--font-system)",
        fontSize: s.fs,
        fontWeight: "var(--fw-medium)",
        lineHeight: 1,
        borderRadius: pill ? "var(--radius-pill)" : "var(--radius-sm)",
        cursor: disabled ? "default" : "pointer",
        opacity: disabled ? 0.45 : 1,
        transition:
          "background var(--dur-fast) var(--ease-standard), opacity var(--dur-fast)",
        ...v,
      }}
      {...rest}
    >
      {iconLeft ? (
        <span style={{ display: "inline-flex" }} aria-hidden="true">
          {iconLeft}
        </span>
      ) : null}
      {children}
    </button>
  );
}
