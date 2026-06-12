import React from "react";

/**
 * Knob — Filter 76's rotary control, matching the AB·VALUE reference dial:
 * a large flat matte-cream puck with a thin dark edge, a faint top-light
 * crescent and a soft drop shadow; a ring of long fine ticks floating
 * outside it (majors every 90°, a bold mark at top); a bold tick that
 * tracks the current value, aligned with a small glowing amber dot on the
 * face. In `face` mode the value prints big (light weight) in the center
 * with a small tracked label beneath — the unit lives in the label
 * (e.g. "1563" / "FREQ · HZ"). Drag vertically or scroll; double-click
 * resets.
 */
export function Knob({
  value = 0,
  min = 0,
  max = 1,
  defaultValue,
  size = "var(--knob-lg)",
  label,
  display,
  minLabel,
  maxLabel,
  accent = "#EFA42F",
  face = false,
  ticks = 40,
  bipolar = false,
  disabled = false,
  onChange,
  className = "",
  showRing,            // deprecated
  ...rest
}) {
  const uid = React.useId().replace(/:/g, "");
  const px = typeof size === "number" ? `${size}px` : size;
  const START = -135, END = 135;
  const t = Math.max(0, Math.min(1, (value - min) / (max - min || 1)));
  const ang = START + t * (END - START);

  const VB = 100, c = VB / 2;
  const rDisc = 30;

  const pt = (deg, r) => {
    const a = (deg * Math.PI) / 180;
    return [c + r * Math.sin(a), c - r * Math.cos(a)];
  };
  const arc = (a0, a1, r) => {
    const [x0, y0] = pt(a0, r), [x1, y1] = pt(a1, r);
    const large = Math.abs(a1 - a0) > 180 ? 1 : 0;
    return `M ${x0.toFixed(2)} ${y0.toFixed(2)} A ${r} ${r} 0 ${large} 1 ${x1.toFixed(2)} ${y1.toFixed(2)}`;
  };

  // tick ring: fine minors, majors every 90°, bold top mark; minors near
  // the moving value tick are skipped so it reads clearly
  const tickEls = [];
  const majorEvery = Math.max(1, Math.round(ticks / 4));
  for (let i = 0; i < ticks; i++) {
    const deg = (i / ticks) * 360;
    const major = i % majorEvery === 0;
    const top = i === 0;
    const dAng = Math.abs(((deg - ang + 540) % 360) - 180);
    if (dAng < 5 && !top) continue;
    const rin = 37.5;
    const rout = top ? 49 : (major ? 47 : 43);
    const w = top ? 1.5 : (major ? 1.0 : 0.7);
    const [x0, y0] = pt(deg, rin), [x1, y1] = pt(deg, rout);
    tickEls.push(<line key={i} x1={x0.toFixed(2)} y1={y0.toFixed(2)} x2={x1.toFixed(2)} y2={y1.toFixed(2)}
      stroke="var(--knob-line)" strokeWidth={w} strokeLinecap="round" opacity={disabled ? 0.3 : 0.8} />);
  }
  const [v0x, v0y] = pt(ang, 37.5), [v1x, v1y] = pt(ang, 49);
  const [dx, dy] = pt(ang, rDisc * 0.8);

  const drag = React.useRef(null);
  const onPointerDown = (e) => {
    if (disabled || !onChange) return;
    e.currentTarget.setPointerCapture?.(e.pointerId);
    drag.current = { y: e.clientY, v: value };
  };
  const onPointerMove = (e) => {
    if (!drag.current) return;
    const dyv = drag.current.y - e.clientY;
    const span = max - min;
    const fine = e.shiftKey ? 0.25 : 1;
    let next = drag.current.v + (dyv / 180) * span * fine;
    onChange(Math.max(min, Math.min(max, next)));
  };
  const onPointerUp = (e) => {
    if (drag.current) { e.currentTarget.releasePointerCapture?.(e.pointerId); drag.current = null; }
  };
  const onWheel = (e) => {
    if (disabled || !onChange) return;
    const span = max - min;
    const step = span * (e.shiftKey ? 0.005 : 0.02);
    onChange(Math.max(min, Math.min(max, value + (e.deltaY < 0 ? step : -step))));
  };
  const onDouble = () => { if (!disabled && onChange && defaultValue != null) onChange(defaultValue); };

  const labelStr = typeof label === "string" ? label : "";
  const faceLabelSize = labelStr.length > 11 ? 5.6 : 6.2;   // larger, tighter — matches JUCE 6.2%
  const faceLabelSpacing = labelStr.length > 11 ? 0.2 : 0.3;

  const disc = (
    <svg
      viewBox={`0 0 ${VB} ${VB}`}
      role="slider" aria-valuenow={value} aria-valuemin={min} aria-valuemax={max} aria-label={labelStr || undefined}
      onPointerDown={onPointerDown} onPointerMove={onPointerMove}
      onPointerUp={onPointerUp} onPointerCancel={onPointerUp}
      onWheel={onWheel} onDoubleClick={onDouble}
      style={{ width: px, height: px, display: "block", cursor: disabled ? "default" : "ns-resize", touchAction: "none", overflow: "visible" }}
    >
      <defs>
        <linearGradient id={`f-${uid}`} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stopColor="#E9E1CC" />
          <stop offset="0.55" stopColor="#E1D8C1" />
          <stop offset="1" stopColor="#D7CDB5" />
        </linearGradient>
        <filter id={`s-${uid}`} x="-50%" y="-50%" width="200%" height="200%">
          <feDropShadow dx="0" dy="2.2" stdDeviation="2.4" floodColor="#2A251C" floodOpacity="0.38" />
        </filter>
        <filter id={`g-${uid}`} x="-400%" y="-400%" width="900%" height="900%">
          <feGaussianBlur stdDeviation="0.9" />
        </filter>
      </defs>

      {tickEls}
      {/* bold tick tracking the value */}
      {!disabled ? (
        <line x1={v0x.toFixed(2)} y1={v0y.toFixed(2)} x2={v1x.toFixed(2)} y2={v1y.toFixed(2)}
          stroke="var(--knob-line)" strokeWidth="1.6" strokeLinecap="round" />
      ) : null}

      {/* matte cream puck */}
      <circle cx={c} cy={c} r={rDisc} fill={`url(#f-${uid})`} filter={`url(#s-${uid})`} />
      <circle cx={c} cy={c} r={rDisc} fill="none" stroke="rgba(45,38,26,0.5)" strokeWidth="0.8" />
      <path d={arc(-62, 62, rDisc - 0.7)} fill="none" stroke="rgba(255,255,255,0.5)" strokeWidth="0.8" strokeLinecap="round" />

      {/* amber indicator dot on the face */}
      {!disabled ? (
        <>
          <circle cx={dx.toFixed(2)} cy={dy.toFixed(2)} r="2.2" fill={accent} opacity="0.4" filter={`url(#g-${uid})`} />
          <circle cx={dx.toFixed(2)} cy={dy.toFixed(2)} r="1.5" fill={accent} />
          <circle cx={(dx - 0.4).toFixed(2)} cy={(dy - 0.4).toFixed(2)} r="0.5" fill="#FFE2A6" />
        </>
      ) : (
        <circle cx={dx.toFixed(2)} cy={dy.toFixed(2)} r="1.4" fill="var(--state-bypassed)" />
      )}

      {/* on-face value + label */}
      {face && (display != null || labelStr) ? (
        <>
          {display != null ? (
            <text x={c} y={labelStr ? c + 1.5 : c + 5} textAnchor="middle" fontFamily="var(--font-mono)"
              fontSize="15" fontWeight="300" fill="#2A2620" letterSpacing="0.4">{display}</text>
          ) : null}
          {labelStr ? (
            <text x={c} y={display != null ? c + 11 : c + 2} textAnchor="middle" fontFamily="var(--font-mono)"
              fontSize={faceLabelSize} fontWeight="400" fill="#6B6353" letterSpacing={faceLabelSpacing}>{labelStr}</text>
          ) : null}
        </>
      ) : null}
    </svg>
  );

  if (face) {
    return (
      <div className={`af-knob af-knob--face ${className}`} data-disabled={disabled || undefined}
        style={{ display: "inline-flex", opacity: disabled ? 0.5 : 1, userSelect: "none" }} {...rest}>
        {disc}
      </div>
    );
  }

  return (
    <div className={`af-knob ${className}`} data-disabled={disabled || undefined}
      style={{ display: "inline-flex", flexDirection: "column", alignItems: "center", gap: "8px",
        opacity: disabled ? 0.5 : 1, userSelect: "none" }} {...rest}>
      {label != null ? <span className="af-label" style={{ color: "var(--text-secondary)" }}>{label}</span> : null}
      {disc}
      {display != null ? (
        <span className="af-readout" style={{
          fontFamily: "var(--font-mono)", fontSize: "var(--fs-13)", fontWeight: "var(--fw-medium)",
          color: "var(--text-on-display)", background: "var(--surface-display)", padding: "4px 12px",
          borderRadius: "var(--radius-xs)", minWidth: "62px", textAlign: "center",
          letterSpacing: "var(--tracking-mono)", boxShadow: "var(--inset-readout)",
        }}>{display}</span>
      ) : null}
      {(minLabel != null || maxLabel != null) ? (
        <div style={{ display: "flex", justifyContent: "space-between", width: "100%",
          fontFamily: "var(--font-mono)", fontSize: "var(--fs-9)", color: "var(--text-muted)" }}>
          <span>{minLabel}</span><span>{maxLabel}</span>
        </div>
      ) : null}
    </div>
  );
}
