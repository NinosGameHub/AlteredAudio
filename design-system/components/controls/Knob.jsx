import React from "react";

/**
 * Knob — Altered Audio's flat Braun/Moog-style rotary control.
 * Mirrors AaLookAndFeel::drawRotarySlider: a flat dirty-cream disc with a
 * 1px warm outline, a near-black indicator line from ~0.2r to ~0.86r, and
 * two range-marker dots at the 7-o'clock start and 5-o'clock end angles.
 * Optional accent ring shows the value sweep in the module's category color.
 */
export function Knob({
  value = 0,
  min = 0,
  max = 1,
  size = 64,
  label,
  display,
  accent = "var(--accent-primary)",
  showArc = false,
  mods = [],
  disabled = false,
  onChange,
  className = "",
  ...rest
}) {
  const START = -135; // 7 o'clock
  const END = 135;    // 5 o'clock
  const t = Math.max(0, Math.min(1, (value - min) / (max - min || 1)));
  const angleAt = (frac) => START + frac * (END - START);

  const cx = size / 2;
  const cy = size / 2;
  const r = size * 0.4;          // disc radius
  const dotR = r + 5.5;          // range dots sit just outside the disc

  const pt = (deg, radius) => {
    const a = (deg * Math.PI) / 180;
    return [cx + radius * Math.sin(a), cy - radius * Math.cos(a)];
  };
  const arc = (a0, a1, radius) => {
    const [x0, y0] = pt(a0, radius);
    const [x1, y1] = pt(a1, radius);
    const large = a1 - a0 > 180 ? 1 : 0;
    return `M ${x0} ${y0} A ${radius} ${radius} 0 ${large} 1 ${x1} ${y1}`;
  };

  const ang = angleAt(t);
  const [lx0, ly0] = pt(ang, r * 0.2);
  const [lx1, ly1] = pt(ang, r * 0.86);
  const [d0x, d0y] = pt(START, dotR);
  const [d1x, d1y] = pt(END, dotR);
  const lineColor = disabled ? "var(--text-disabled)" : "var(--knob-line)";

  return (
    <div
      className={`aa-knob ${className}`}
      data-disabled={disabled || undefined}
      style={{
        display: "inline-flex",
        flexDirection: "column",
        alignItems: "center",
        gap: "5px",
        opacity: disabled ? 0.45 : 1,
        userSelect: "none",
      }}
      {...rest}
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

      <svg
        width={size}
        height={size}
        viewBox={`0 0 ${size} ${size}`}
        role="slider"
        aria-valuenow={value}
        aria-valuemin={min}
        aria-valuemax={max}
        style={{ display: "block", cursor: disabled ? "default" : "ns-resize" }}
        onClick={() => !disabled && onChange && onChange(value)}
      >
        {/* optional value sweep ring in category color */}
        {showArc ? (
          <>
            <path d={arc(START, END, dotR)} fill="none" stroke="var(--border-default)" strokeWidth="2" strokeLinecap="round" />
            {!disabled && t > 0.001 ? (
              <path d={arc(START, ang, dotR)} fill="none" stroke={accent} strokeWidth="2" strokeLinecap="round" />
            ) : null}
          </>
        ) : null}

        {/* range-marker dots */}
        <circle cx={d0x} cy={d0y} r="1.5" fill="var(--knob-dot)" />
        <circle cx={d1x} cy={d1y} r="1.5" fill="var(--knob-dot)" />

        {/* modulation arcs — each routing draws a thin arc from the current
           value by its (bipolar) depth, in the source color, just outside
           the value ring; concentric when multiple sources target the knob */}
        {!disabled && mods && mods.length ? mods.map((md, i) => {
          const depth = Math.max(-1, Math.min(1, md.depth || 0));
          if (Math.abs(depth) < 0.001) return null;
          const modR = dotR + 3 + i * 3;
          const tEnd = Math.max(0, Math.min(1, t + depth));
          const a0 = depth >= 0 ? ang : angleAt(tEnd);
          const a1 = depth >= 0 ? angleAt(tEnd) : ang;
          const [ex, ey] = pt(angleAt(tEnd), modR);
          return (
            <g key={i}>
              <path d={arc(a0, a1, modR)} fill="none" stroke={md.color || "var(--cat-modulation)"} strokeWidth="1.6" strokeLinecap="round" opacity="0.95" />
              <circle cx={ex} cy={ey} r="1.6" fill={md.color || "var(--cat-modulation)"} />
            </g>
          );
        }) : null}

        {/* flat cream disc */}
        <circle cx={cx} cy={cy} r={r} fill="var(--knob-body)" stroke="var(--border-default)" strokeWidth="1" />

        {/* near-black indicator line */}
        <line x1={lx0} y1={ly0} x2={lx1} y2={ly1} stroke={lineColor} strokeWidth="2.5" strokeLinecap="round" />
      </svg>

      {display != null ? (
        <span
          style={{
            fontFamily: "var(--font-mono)",
            fontSize: "var(--fs-10)",
            color: "var(--text-muted)",
            lineHeight: 1,
          }}
        >
          {display}
        </span>
      ) : null}
    </div>
  );
}
