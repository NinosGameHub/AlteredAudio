import React from "react";

/**
 * EQCurve — the EQ module's frequency-response display (backlog item #2).
 * Near-black CRT glass with a log-frequency grid, a faint amber spectrum
 * behind, the summed EQ response curve drawn in Filter/EQ blue, and one
 * draggable handle per band. Drag a handle horizontally to move FREQ,
 * vertically to move GAIN. Cosmetic — drives the `bands` you pass in.
 *
 * Band: { freq (Hz 20..20000), gain (dB -18..18), q, type, on, color? }
 */
export function EQCurve({
  bands = [],
  onChange,
  width = 560,
  height = 220,
  showSpectrum = true,
  accent = "var(--cat-filtereq)",
  className = "",
  ...rest
}) {
  const padL = 8, padR = 8, padT = 10, padB = 18;
  const W = width, H = height;
  const plotW = W - padL - padR;
  const plotH = H - padT - padB;

  const FMIN = 20, FMAX = 20000;
  const GMAX = 18;
  const logF = (f) => Math.log10(f);
  const x = (f) => padL + ((logF(f) - logF(FMIN)) / (logF(FMAX) - logF(FMIN))) * plotW;
  const y = (g) => padT + (1 - (g + GMAX) / (2 * GMAX)) * plotH;
  const invX = (px) => Math.pow(10, logF(FMIN) + ((px - padL) / plotW) * (logF(FMAX) - logF(FMIN)));
  const invY = (py) => (1 - (py - padT) / plotH) * 2 * GMAX - GMAX;

  // peaking/shelf gaussian-ish bump model for the summed curve
  const bandGain = (b, f) => {
    if (!b.on) return 0;
    const oct = Math.log2(f / b.freq);
    const bw = 1 / Math.max(0.3, b.q || 1);
    if (b.type === "LowShelf") return b.gain / (1 + Math.pow(Math.max(0.0001, f / b.freq), 2.2));
    if (b.type === "HighShelf") return b.gain / (1 + Math.pow(Math.max(0.0001, b.freq / f), 2.2));
    return b.gain * Math.exp(-(oct * oct) / (2 * bw * bw));
  };

  const N = 120;
  const pts = [];
  for (let i = 0; i <= N; i++) {
    const f = Math.pow(10, logF(FMIN) + (i / N) * (logF(FMAX) - logF(FMIN)));
    let g = 0;
    bands.forEach((b) => (g += bandGain(b, f)));
    pts.push([x(f), y(Math.max(-GMAX, Math.min(GMAX, g)))]);
  }
  const curve = pts.map((p, i) => (i ? "L" : "M") + p[0].toFixed(1) + " " + p[1].toFixed(1)).join(" ");
  const fill = curve + ` L ${x(FMAX).toFixed(1)} ${y(0).toFixed(1)} L ${x(FMIN).toFixed(1)} ${y(0).toFixed(1)} Z`;

  // fake spectrum behind
  const spec = [];
  if (showSpectrum) {
    for (let i = 0; i < 48; i++) {
      const f = Math.pow(10, logF(FMIN) + (i / 47) * (logF(FMAX) - logF(FMIN)));
      const tilt = 1 - i / 48;
      const h = Math.max(0.05, 0.2 + tilt * 0.55 + 0.12 * Math.sin(i * 1.3));
      spec.push([x(f), h]);
    }
  }

  const drag = React.useRef(null);
  const svgRef = React.useRef(null);

  const onDown = (i) => (e) => { drag.current = i; e.preventDefault(); };
  React.useEffect(() => {
    const move = (e) => {
      if (drag.current == null || !onChange || !svgRef.current) return;
      const r = svgRef.current.getBoundingClientRect();
      const cx = ((e.clientX - r.left) / r.width) * W;
      const cy = ((e.clientY - r.top) / r.height) * H;
      const f = Math.max(FMIN, Math.min(FMAX, invX(cx)));
      const g = Math.max(-GMAX, Math.min(GMAX, invY(cy)));
      const next = bands.map((b, idx) => (idx === drag.current ? { ...b, freq: Math.round(f), gain: +g.toFixed(1), on: true } : b));
      onChange(next);
    };
    const up = () => (drag.current = null);
    window.addEventListener("mousemove", move);
    window.addEventListener("mouseup", up);
    return () => { window.removeEventListener("mousemove", move); window.removeEventListener("mouseup", up); };
  }, [bands, onChange]);

  const gridF = [20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000];
  const labF = { 20: "20", 100: "100", 1000: "1k", 10000: "10k", 20000: "20k" };
  const gridG = [-12, -6, 0, 6, 12];

  return (
    <div className={`aa-eqcurve ${className}`} style={{ position: "relative", width: W, maxWidth: "100%" }} {...rest}>
      <svg ref={svgRef} width="100%" viewBox={`0 0 ${W} ${H}`} style={{ display: "block", background: "var(--crt-bg)", borderRadius: "var(--radius-sm)" }}>
        {/* vertical freq grid */}
        {gridF.map((f) => (
          <line key={f} x1={x(f)} y1={padT} x2={x(f)} y2={padT + plotH} stroke="var(--crt-grid)" strokeWidth="1" />
        ))}
        {/* horizontal gain grid */}
        {gridG.map((g) => (
          <line key={g} x1={padL} y1={y(g)} x2={padL + plotW} y2={y(g)} stroke={g === 0 ? "rgba(228,162,16,0.32)" : "var(--crt-grid)"} strokeWidth={g === 0 ? 1.2 : 1} />
        ))}
        {/* spectrum */}
        {showSpectrum && spec.map((s, i) => (
          <rect key={i} x={s[0] - 4} y={padT + plotH - s[1] * plotH} width="6" height={s[1] * plotH}
            fill="var(--crt-amber)" opacity="0.16" />
        ))}
        {/* response fill + curve */}
        <path d={fill} fill={accent} opacity="0.14" />
        <path d={curve} fill="none" stroke={accent} strokeWidth="2" strokeLinejoin="round" />
        {/* band handles */}
        {bands.map((b, i) => (
          <g key={i} style={{ cursor: onChange ? "grab" : "default" }} onMouseDown={onDown(i)}>
            <circle cx={x(b.freq)} cy={y(b.on ? b.gain : 0)} r="7"
              fill={b.on ? (b.color || accent) : "var(--state-inactive)"} opacity={b.on ? 0.9 : 0.5}
              stroke="var(--crt-bg)" strokeWidth="1.5" />
            <text x={x(b.freq)} y={y(b.on ? b.gain : 0) + 3.2} textAnchor="middle"
              fontFamily="var(--font-mono)" fontSize="8" fill="#fff" style={{ pointerEvents: "none" }}>{i + 1}</text>
          </g>
        ))}
        {/* freq labels */}
        {gridF.filter((f) => labF[f]).map((f) => (
          <text key={f} x={x(f)} y={H - 5} textAnchor="middle" fontFamily="var(--font-mono)" fontSize="9" fill="var(--crt-amber)" opacity="0.6">{labF[f]}</text>
        ))}
      </svg>
    </div>
  );
}
