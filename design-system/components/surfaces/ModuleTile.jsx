import React from "react";

/**
 * ModuleTile — one row in the plugin's left tile list (ModuleTileList).
 * A ~40px row with an always-visible 4px category strip at the left edge,
 * a mono chain-number (top-left), the centered bold module name, and an LED
 * dot at the right (category color when engaged, warm-grey when bypassed).
 * When `selected`, the row gets a faint category-tinted background and the
 * name takes the category color.
 */
export function ModuleTile({
  index,
  name,
  category = "var(--accent-primary)",
  active = true,
  selected = false,
  onClick,
  className = "",
  ...rest
}) {
  const num = index != null ? String(index).padStart(2, "0") : "";
  return (
    <button
      type="button"
      className={`aa-module-tile ${selected ? "is-selected" : ""} ${className}`}
      onClick={onClick}
      style={{
        position: "relative",
        display: "block",
        width: "100%",
        height: "var(--h-tile)",
        padding: 0,
        background: "transparent",
        border: "none",
        borderBottom: "1px solid rgba(178,173,161,0.5)",
        cursor: "pointer",
        transition: "background var(--dur-fast) var(--ease-standard)",
      }}
      {...rest}
    >
      {/* selection tint (trimmed left of the strip) */}
      {selected ? (
        <span
          aria-hidden="true"
          style={{
            position: "absolute",
            left: "4px",
            top: 0,
            right: 0,
            bottom: 0,
            background: category,
            opacity: 0.14,
          }}
        />
      ) : null}

      {/* always-on 4px category strip */}
      <span
        aria-hidden="true"
        style={{
          position: "absolute",
          left: 0,
          top: 0,
          bottom: 0,
          width: "4px",
          background: category,
        }}
      />

      {/* mono chain number, top-left */}
      <span
        style={{
          position: "absolute",
          left: "9px",
          top: "4px",
          fontFamily: "var(--font-mono)",
          fontSize: "var(--fs-8)",
          color: "var(--text-muted)",
          opacity: 0.7,
        }}
      >
        {num}
      </span>

      {/* centered module name */}
      <span
        style={{
          position: "absolute",
          left: "10px",
          right: "16px",
          top: 0,
          bottom: 0,
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          fontSize: "var(--fs-10)",
          fontWeight: "var(--fw-bold)",
          letterSpacing: "var(--tracking-label)",
          textTransform: "uppercase",
          color: selected ? category : "var(--text-body)",
          whiteSpace: "nowrap",
          overflow: "hidden",
          textOverflow: "ellipsis",
        }}
      >
        {name}
      </span>

      {/* LED dot, right */}
      <span
        aria-hidden="true"
        style={{
          position: "absolute",
          right: "8px",
          top: "50%",
          transform: "translateY(-50%)",
          width: "6.4px",
          height: "6.4px",
          borderRadius: "var(--radius-pill)",
          background: active ? category : "var(--state-inactive)",
        }}
      />
    </button>
  );
}
