import React from "react";

/**
 * CRTDisplay — the amber phosphor spectrum strip (CRTDisplay.cpp).
 * Near-black glass, a faint horizontal grid, scan-line texture, a 32-bar
 * amber spectrum, and mono frequency labels along the bottom. Bars animate
 * gently when `live`. Purely cosmetic — feed `bars` (0..1 heights) or let
 * it self-animate.
 */
export function CRTDisplay({
  bars,
  live = true,
  height = 180,
  label,
  className = "",
  ...rest
}) {
  const N = 32;
  const [phase, setPhase] = React.useState(0);

  React.useEffect(() => {
    if (!live || bars) return;
    let raf;
    const tick = () => {
      setPhase((p) => p + 0.05);
      raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [live, bars]);

  // synthesize a plausible spectrum: tilted down toward highs + wobble
  const heights =
    bars ||
    Array.from({ length: N }, (_, i) => {
      const tilt = 1 - i / N;
      const wob = 0.5 + 0.5 * Math.sin(phase * 2 + i * 0.7) * Math.cos(phase + i * 0.3);
      return Math.max(0.06, Math.min(1, 0.25 + tilt * 0.6 * wob + (i < 3 ? 0.2 : 0)));
    });

  const freqLabels = ["20", "100", "500", "1k", "5k", "20k"];

  return (
    <div
      className={`aa-crt ${className}`}
      style={{
        position: "relative",
        width: "100%",
        height: `${height}px`,
        background: "var(--crt-bg)",
        overflow: "hidden",
      }}
      {...rest}
    >
      {/* horizontal grid */}
      <div
        style={{
          position: "absolute",
          inset: 0,
          backgroundImage:
            "repeating-linear-gradient(to top, transparent 0, transparent 24px, var(--crt-grid) 24px, var(--crt-grid) 25px)",
        }}
      />
      {/* scan lines */}
      <div
        style={{
          position: "absolute",
          inset: 0,
          backgroundImage:
            "repeating-linear-gradient(to bottom, transparent 0, transparent 2px, var(--crt-scanline) 2px, var(--crt-scanline) 3px)",
          pointerEvents: "none",
        }}
      />

      {/* optional corner label */}
      {label ? (
        <span
          style={{
            position: "absolute",
            top: 8,
            left: 12,
            fontFamily: "var(--font-mono)",
            fontSize: "var(--fs-9)",
            letterSpacing: "0.08em",
            color: "var(--crt-amber)",
            opacity: 0.85,
          }}
        >
          {label}
        </span>
      ) : null}

      {/* spectrum bars */}
      <div
        style={{
          position: "absolute",
          left: 12,
          right: 12,
          bottom: 20,
          top: 18,
          display: "flex",
          alignItems: "flex-end",
          gap: "2px",
        }}
      >
        {heights.map((h, i) => (
          <div
            key={i}
            style={{
              flex: 1,
              height: `${h * 100}%`,
              background: "var(--crt-amber)",
              opacity: 0.55 + h * 0.45,
              boxShadow: "0 0 6px rgba(228,162,16,0.55)",
            }}
          />
        ))}
      </div>

      {/* frequency labels */}
      <div
        style={{
          position: "absolute",
          left: 12,
          right: 12,
          bottom: 4,
          display: "flex",
          justifyContent: "space-between",
          fontFamily: "var(--font-mono)",
          fontSize: "var(--fs-9)",
          color: "var(--crt-amber)",
          opacity: 0.6,
        }}
      >
        {freqLabels.map((f) => (
          <span key={f}>{f}</span>
        ))}
      </div>
    </div>
  );
}
