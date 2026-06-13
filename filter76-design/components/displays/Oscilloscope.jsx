import React from "react";

/**
 * Oscilloscope — the small amber trace display used by the LFO ENGINE
 * (a scrolling sine / triangle / square / sawtooth / random wave) and the
 * ENVELOPE FOLLOWER (an attack-spike-then-decay trace). Near-black glass,
 * amber line, faint center axis. Cosmetic; animates when `live`.
 */
export function Oscilloscope({
  waveform = "sine",   // sine | triangle | square | sawtooth | random | envelope
  cycles = 3,
  height = 64,
  live = true,
  className = "",
  style = {},
  ...rest
}) {
  const W = 300, H = 100, pad = 8;
  const mid = H / 2;
  const amp = (H - pad * 2) / 2;

  const [phase, setPhase] = React.useState(0);
  React.useEffect(() => {
    if (!live) return;
    let raf;
    const tick = () => { setPhase((p) => p + 0.02); raf = requestAnimationFrame(tick); };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [live]);

  // sample-and-hold values for "random"
  const rnd = React.useRef(Array.from({ length: 64 }, () => Math.random() * 2 - 1));

  const wave = (p) => {
    const x = ((p % 1) + 1) % 1;
    switch (waveform) {
      case "triangle": return x < 0.5 ? 4 * x - 1 : 3 - 4 * x;
      case "square":   return x < 0.5 ? 1 : -1;
      case "sawtooth": return 2 * x - 1;
      case "random":   return rnd.current[Math.floor(x * rnd.current.length) % rnd.current.length];
      case "sine":
      default:         return Math.sin(2 * Math.PI * x);
    }
  };

  let d = "";
  const N = 240;
  if (waveform === "envelope") {
    // attack spike → exponential decay with light ripple
    for (let i = 0; i <= N; i++) {
      const t = i / N;
      let env;
      if (t < 0.08) env = t / 0.08;
      else env = Math.exp(-(t - 0.08) * 4.2);
      const ripple = 0.05 * Math.sin(40 * t + phase * 4) * env;
      const yv = mid - (env * 0.92 + ripple) * amp;
      d += (i ? "L" : "M") + (pad + t * (W - pad * 2)).toFixed(1) + " " + yv.toFixed(1) + " ";
    }
  } else {
    for (let i = 0; i <= N; i++) {
      const t = i / N;
      const yv = mid - wave(t * cycles - phase) * amp * 0.92;
      d += (i ? "L" : "M") + (pad + t * (W - pad * 2)).toFixed(1) + " " + yv.toFixed(1) + " ";
    }
  }

  return (
    <div
      className={`af-scope ${className}`}
      style={{
        width: "100%", height: typeof height === "number" ? `${height}px` : height,
        background: "var(--surface-display)", borderRadius: "var(--radius-sm)",
        boxShadow: "var(--inset-readout)", overflow: "hidden", ...style,
      }}
      {...rest}
    >
      <svg width="100%" height="100%" viewBox={`0 0 ${W} ${H}`} preserveAspectRatio="none" style={{ display: "block" }}>
        <line x1={pad} y1={mid} x2={W - pad} y2={mid} stroke="var(--crt-grid-axis)" strokeWidth="1" vectorEffect="non-scaling-stroke" />
        <path d={d} fill="none" stroke="var(--crt-line)" strokeWidth="2" strokeLinejoin="round" strokeLinecap="round" vectorEffect="non-scaling-stroke" />
      </svg>
    </div>
  );
}
