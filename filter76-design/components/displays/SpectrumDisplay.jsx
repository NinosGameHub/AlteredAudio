import React from "react";

/**
 * SpectrumDisplay — the Filter 76 dominant graph, designed in the clean
 * Pro-Q idiom: a dark precision display with a subtle log grid, a smooth
 * translucent real-time analyzer (gradient-filled), a crisp glowing
 * frequency-response curve, and a round draggable cutoff node (drag X =
 * frequency, drag Y = resonance). Renders at native pixel resolution so
 * curves and the node stay crisp and round at any size.
 *
 * Filter params: type LP|HP|BP|NOTCH|PEAK|SHELF, freq Hz, q, gain dB,
 * slope dB/oct.
 */
export function SpectrumDisplay({
  type = "LP",
  freq = 1000,
  q = 1.0,
  gain = 0,
  slope = 12,
  qMin = 0.1,
  qMax = 24,
  height = 300,
  showSpectrum = true,
  showNode = true,
  live = true,
  onNodeChange,
  className = "",
  ...rest
}) {
  const uid = React.useId().replace(/:/g, "");
  const wrapRef = React.useRef(null);
  const [size, setSize] = React.useState({ w: 760, h: typeof height === "number" ? height : 300 });

  React.useLayoutEffect(() => {
    if (!wrapRef.current) return;
    const ro = new ResizeObserver((entries) => {
      const r = entries[0].contentRect;
      if (r.width > 0 && r.height > 0) setSize({ w: Math.round(r.width), h: Math.round(r.height) });
    });
    ro.observe(wrapRef.current);
    return () => ro.disconnect();
  }, []);

  const W = size.w, H = size.h;
  const padL = 46, padR = 18, padT = 16, padB = 26;
  const plotW = Math.max(10, W - padL - padR);
  const plotH = Math.max(10, H - padT - padB);

  const FMIN = 20, FMAX = 20000;
  const GTOP = 12, GBOT = -60;
  const lf = (f) => Math.log10(f);
  const x = (f) => padL + ((lf(f) - lf(FMIN)) / (lf(FMAX) - lf(FMIN))) * plotW;
  const y = (g) => padT + (1 - (g - GBOT) / (GTOP - GBOT)) * plotH;
  const invX = (px) => Math.pow(10, lf(FMIN) + ((px - padL) / plotW) * (lf(FMAX) - lf(FMIN)));
  const invY = (py) => GBOT + (1 - (py - padT) / plotH) * (GTOP - GBOT);

  const respDb = (f) => {
    const oct = Math.log2(f / freq);
    const bw = 1 / Math.max(0.3, q * 0.6);
    const reso = Math.min(18, (q - 0.7) * 1.4);
    const bump = reso > 0 ? reso * Math.exp(-(oct * oct) / (2 * 0.35 * 0.35)) : 0;
    switch (type) {
      case "HP": return (f >= freq ? 0 : -slope * Math.abs(oct)) + bump;
      case "BP": return -slope * 0.7 * Math.abs(oct) + Math.max(0, reso) * Math.exp(-(oct * oct) / (2 * 0.4 * 0.4));
      case "NOTCH": return -14 * Math.exp(-(oct * oct) / (2 * (bw * 0.5) * (bw * 0.5)));
      case "PEAK": return (gain || 6) * Math.exp(-(oct * oct) / (2 * bw * bw));
      case "SHELF": return (gain || 6) / (1 + Math.pow(Math.max(1e-4, f / freq), 2.2));
      case "LP":
      default: return (f <= freq ? 0 : -slope * oct) + bump;
    }
  };

  // Catmull-Rom → cubic-bezier smoothing for silky curves
  const smooth = (pts) => {
    if (pts.length < 3) return pts.map((p, i) => (i ? "L" : "M") + p[0].toFixed(1) + " " + p[1].toFixed(1)).join(" ");
    let d = `M ${pts[0][0].toFixed(1)} ${pts[0][1].toFixed(1)}`;
    for (let i = 0; i < pts.length - 1; i++) {
      const p0 = pts[i - 1] || pts[i], p1 = pts[i], p2 = pts[i + 1], p3 = pts[i + 2] || p2;
      const c1x = p1[0] + (p2[0] - p0[0]) / 6, c1y = p1[1] + (p2[1] - p0[1]) / 6;
      const c2x = p2[0] - (p3[0] - p1[0]) / 6, c2y = p2[1] - (p3[1] - p1[1]) / 6;
      d += ` C ${c1x.toFixed(1)} ${c1y.toFixed(1)}, ${c2x.toFixed(1)} ${c2y.toFixed(1)}, ${p2[0].toFixed(1)} ${p2[1].toFixed(1)}`;
    }
    return d;
  };

  // response curve
  const NR = 160;
  const curvePts = [];
  for (let i = 0; i <= NR; i++) {
    const f = Math.pow(10, lf(FMIN) + (i / NR) * (lf(FMAX) - lf(FMIN)));
    curvePts.push([x(f), y(Math.max(GBOT, Math.min(GTOP, respDb(f))))]);
  }
  const curve = smooth(curvePts);

  // smooth animated analyzer (Pro-Q style: gentle, gradient-filled)
  const [phase, setPhase] = React.useState(0);
  React.useEffect(() => {
    if (!live || !showSpectrum) return;
    let raf;
    const tick = () => { setPhase((p) => p + 0.018); raf = requestAnimationFrame(tick); };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [live, showSpectrum]);

  let specPath = "", specArea = "";
  if (showSpectrum) {
    const M = Math.max(48, Math.round(plotW / 6));
    const p1 = 0.18 + 0.02 * Math.sin(phase * 0.5);
    const p2 = 0.34 + 0.015 * Math.sin(phase * 0.4 + 2);
    const pts = [];
    for (let i = 0; i <= M; i++) {
      const u = i / M;
      const f = Math.pow(10, lf(FMIN) + u * (lf(FMAX) - lf(FMIN)));
      const tilt = -4 - u * 22;                         // gentle pink slope
      const r = respDb(f);
      const n =
          3.2 * Math.sin(phase * 1.3 + i * 0.5)
        + 1.8 * Math.sin(phase * 2.1 + i * 1.07 + 1.1)
        + 1.0 * Math.sin(phase * 0.7 + i * 0.23);
      const peak1 = 9 * Math.exp(-Math.pow(u - p1, 2) / (2 * 0.02));
      const peak2 = 6 * Math.exp(-Math.pow(u - p2, 2) / (2 * 0.014));
      let g = tilt + Math.min(2, r) + n + peak1 + peak2;
      g = Math.max(GBOT, Math.min(GTOP - 1, g));
      pts.push([x(f), y(g)]);
    }
    specPath = smooth(pts);
    specArea = specPath + ` L ${x(FMAX).toFixed(1)} ${y(GBOT).toFixed(1)} L ${x(FMIN).toFixed(1)} ${y(GBOT).toFixed(1)} Z`;
  }

  // node
  const nodeY = y(Math.max(GBOT, Math.min(GTOP, respDb(freq))));
  const nodeX = x(freq);
  const svgRef = React.useRef(null);
  const dragging = React.useRef(false);
  const onDown = (e) => { if (onNodeChange) { dragging.current = true; e.preventDefault(); } };
  React.useEffect(() => {
    const move = (e) => {
      if (!dragging.current || !onNodeChange || !svgRef.current) return;
      const r = svgRef.current.getBoundingClientRect();
      const cx = ((e.clientX - r.left) / r.width) * W;
      const cy = ((e.clientY - r.top) / r.height) * H;
      const nf = Math.max(FMIN, Math.min(FMAX, invX(cx)));
      const frac = Math.max(0, Math.min(1, (invY(cy) - GBOT) / (GTOP - GBOT)));
      const nq = qMin + frac * (qMax - qMin);
      onNodeChange({ freq: Math.round(nf), q: +nq.toFixed(2) });
    };
    const up = () => (dragging.current = false);
    window.addEventListener("mousemove", move);
    window.addEventListener("mouseup", up);
    return () => { window.removeEventListener("mousemove", move); window.removeEventListener("mouseup", up); };
  }, [onNodeChange, W, H]);

  const gridF = [20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000];
  const labF = { 20: "20", 50: "50", 100: "100", 200: "200", 500: "500", 1000: "1k", 2000: "2k", 5000: "5k", 10000: "10k", 20000: "20k" };
  const gridG = [12, 6, 0, -6, -12, -18, -24, -36, -48, -60];

  return (
    <div
      ref={wrapRef}
      className={`af-spectrum ${className}`}
      style={{
        position: "relative", width: "100%", height: typeof height === "number" ? `${height}px` : height,
        background: "radial-gradient(130% 140% at 50% 30%, #201e1a 0%, #15140f 52%, #0b0a08 100%)",
        borderRadius: "var(--radius-lg)", overflow: "hidden",
        boxShadow: "var(--inset-well), 0 0 0 1px rgba(0,0,0,0.5)",
      }}
      {...rest}
    >
      <svg ref={svgRef} width={W} height={H} viewBox={`0 0 ${W} ${H}`}
        style={{ display: "block", cursor: onNodeChange ? "crosshair" : "default" }}>
        <defs>
          <linearGradient id={`sg-${uid}`} x1="0" y1={padT} x2="0" y2={padT + plotH} gradientUnits="userSpaceOnUse">
            <stop offset="0" stopColor="var(--crt-line)" stopOpacity="0.55" />
            <stop offset="0.4" stopColor="var(--crt-line)" stopOpacity="0.20" />
            <stop offset="1" stopColor="var(--crt-line)" stopOpacity="0.0" />
          </linearGradient>
          <filter id={`glow-${uid}`} x="-20%" y="-40%" width="140%" height="180%">
            <feGaussianBlur stdDeviation="3.2" />
          </filter>
        </defs>

        {/* grid */}
        {gridF.map((f) => (
          <line key={f} x1={x(f)} y1={padT} x2={x(f)} y2={padT + plotH} stroke="rgba(240,181,71,0.07)" strokeWidth="1" />
        ))}
        {gridG.map((g) => (
          <line key={g} x1={padL} y1={y(g)} x2={padL + plotW} y2={y(g)}
            stroke={g === 0 ? "rgba(240,181,71,0.16)" : "rgba(240,181,71,0.06)"} strokeWidth="1" />
        ))}

        {/* analyzer */}
        {showSpectrum ? (
          <>
            <path d={specArea} fill={`url(#sg-${uid})`} />
            <path d={specPath} fill="none" stroke="var(--crt-line)" strokeWidth="1.25" opacity="0.85" strokeLinejoin="round" />
          </>
        ) : null}

        {/* response curve — soft glow + crisp bright line */}
        <path d={curve} fill="none" stroke="#FBD27A" strokeWidth="3.5" opacity="0.5" filter={`url(#glow-${uid})`} strokeLinejoin="round" strokeLinecap="round" />
        <path d={curve} fill="none" stroke="#FFE0A0" strokeWidth="2" strokeLinejoin="round" strokeLinecap="round" />

        {/* node */}
        {showNode ? (
          <g style={{ cursor: onNodeChange ? "grab" : "default" }} onMouseDown={onDown}>
            <line x1={nodeX} y1={padT} x2={nodeX} y2={padT + plotH} stroke="#FBD27A" strokeWidth="1" opacity="0.35" />
            <circle cx={nodeX} cy={nodeY} r="11" fill="rgba(251,210,122,0.12)" stroke="#FBD27A" strokeWidth="1.5" />
            <circle cx={nodeX} cy={nodeY} r="3.4" fill="#FFE6B0" />
          </g>
        ) : null}

        {/* dB labels */}
        {gridG.map((g) => (
          <text key={g} x={padL - 8} y={y(g) + 3.5} textAnchor="end"
            fontFamily="var(--font-mono)" fontSize="10.5" fill="var(--crt-line)" opacity="0.42">{g}</text>
        ))}
        <text x={padL - 8} y={padT - 3} textAnchor="end" fontFamily="var(--font-mono)" fontSize="9.5" fill="var(--crt-line)" opacity="0.55">dB</text>

        {/* freq labels */}
        {gridF.map((f) => (
          <text key={f} x={x(f)} y={H - 8} textAnchor="middle"
            fontFamily="var(--font-mono)" fontSize="10.5" fill="var(--crt-line)" opacity="0.42">{labF[f]}</text>
        ))}
        <text x={W - padR} y={H - 8} textAnchor="end" fontFamily="var(--font-mono)" fontSize="9.5" fill="var(--crt-line)" opacity="0.55">Hz</text>
      </svg>

      {/* subtle CRT vignette */}
      <div style={{ position: "absolute", inset: 0, background: "var(--crt-vignette)", pointerEvents: "none" }} />
    </div>
  );
}
