import React from "react";

/**
 * CRTDisplay — the amber phosphor spectrum strip (CRTDisplay.cpp), sci-fi
 * treatment. Canvas-rendered: a single glowing spectral line with a soft
 * gradient fill beneath it, a sweeping scan highlight, a perspective grid
 * and a vignette. Amber-on-near-black. Feed `bars` (0..1) or let it animate.
 */
export function CRTDisplay({
  bars,
  live = true,
  height = 180,
  label,
  bins = 56,
  className = "",
  ...rest
}) {
  const wrapRef = React.useRef(null);
  const canvasRef = React.useRef(null);
  const peaks = React.useRef(new Float32Array(bins));
  const data = React.useRef(new Float32Array(bins));
  const barsRef = React.useRef(bars);
  barsRef.current = bars;

  React.useEffect(() => {
    const canvas = canvasRef.current;
    const wrap = wrapRef.current;
    if (!canvas || !wrap) return;
    const ctx = canvas.getContext("2d");
    let raf, t = 0, sweep = -0.2, running = true;

    const AMBER = [228, 162, 16];
    const rgba = (c, a) => `rgba(${c[0]},${c[1]},${c[2]},${a})`;

    const synth = (i, n) => {
      const f = i / n;
      const tilt = Math.pow(1 - f, 0.8);
      // a few drifting spectral peaks for life
      const p1 = Math.exp(-Math.pow((f - (0.12 + 0.03 * Math.sin(t * 0.7))) / 0.05, 2));
      const p2 = Math.exp(-Math.pow((f - (0.34 + 0.05 * Math.sin(t * 0.5 + 1))) / 0.07, 2));
      const p3 = Math.exp(-Math.pow((f - (0.62 + 0.04 * Math.sin(t * 0.9 + 2))) / 0.09, 2));
      const wob = 0.5 + 0.5 * Math.sin(t * 2.0 + i * 0.6) * Math.cos(t * 1.1 + i * 0.25);
      let v = 0.16 + tilt * 0.5 * wob + 0.55 * p1 + 0.4 * p2 + 0.3 * p3;
      return Math.max(0.04, Math.min(1, v));
    };

    const draw = () => {
      if (!running) return;
      const cssW = wrap.clientWidth || 600;
      const cssH = height;
      const dpr = Math.min(window.devicePixelRatio || 1, 2);
      if (canvas.width !== Math.round(cssW * dpr) || canvas.height !== Math.round(cssH * dpr)) {
        canvas.width = Math.round(cssW * dpr);
        canvas.height = Math.round(cssH * dpr);
      }
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      const W = cssW, H = cssH;
      const n = bins;

      // advance source data
      const ext = barsRef.current;
      for (let i = 0; i < n; i++) {
        const target = ext ? (ext[Math.floor((i / n) * ext.length)] || 0) : (live ? synth(i, n) : 0.0);
        // attack fast, release slow
        const cur = data.current[i];
        data.current[i] = target > cur ? cur + (target - cur) * 0.5 : cur + (target - cur) * 0.12;
        peaks.current[i] = Math.max(peaks.current[i] * 0.965, data.current[i]);
      }

      ctx.clearRect(0, 0, W, H);

      // background glass + radial vignette
      const bg = ctx.createRadialGradient(W / 2, H * 0.55, H * 0.2, W / 2, H * 0.55, W * 0.7);
      bg.addColorStop(0, "#0b0907");
      bg.addColorStop(1, "#040302");
      ctx.fillStyle = bg;
      ctx.fillRect(0, 0, W, H);

      const padX = 12, padTop = 16, padBot = 22;
      const plotW = W - padX * 2;
      const baseY = H - padBot;
      const maxH = baseY - padTop;

      // perspective-ish grid
      ctx.lineWidth = 1;
      ctx.strokeStyle = rgba(AMBER, 0.07);
      for (let g = 0; g <= 4; g++) {
        const yy = padTop + (g / 4) * maxH;
        ctx.beginPath(); ctx.moveTo(padX, yy); ctx.lineTo(W - padX, yy); ctx.stroke();
      }
      const fxs = [0.0, 0.16, 0.34, 0.5, 0.68, 0.84, 1.0];
      fxs.forEach((fx) => {
        const xx = padX + fx * plotW;
        ctx.beginPath(); ctx.moveTo(xx, padTop); ctx.lineTo(xx, baseY); ctx.stroke();
      });
      // baseline
      ctx.strokeStyle = rgba(AMBER, 0.35);
      ctx.beginPath(); ctx.moveTo(padX, baseY); ctx.lineTo(W - padX, baseY); ctx.stroke();

      const linePts = [];
      for (let i = 0; i < n; i++) {
        const h = data.current[i] * maxH;
        const x = padX + (i + 0.5) * (plotW / n);
        linePts.push([x, baseY - h]);
      }
      // clamp ends to the plot edges
      linePts.unshift([padX, linePts[0][1]]);
      linePts.push([W - padX, linePts[linePts.length - 1][1]]);

      // smooth path through the points (midpoint quadratics)
      const tracePath = () => {
        ctx.beginPath();
        ctx.moveTo(linePts[0][0], linePts[0][1]);
        for (let i = 1; i < linePts.length - 1; i++) {
          const mx = (linePts[i][0] + linePts[i + 1][0]) / 2;
          const my = (linePts[i][1] + linePts[i + 1][1]) / 2;
          ctx.quadraticCurveTo(linePts[i][0], linePts[i][1], mx, my);
        }
        const last = linePts[linePts.length - 1];
        ctx.lineTo(last[0], last[1]);
      };

      // soft gradient fill under the line
      tracePath();
      ctx.lineTo(W - padX, baseY);
      ctx.lineTo(padX, baseY);
      ctx.closePath();
      const fillG = ctx.createLinearGradient(0, padTop, 0, baseY);
      fillG.addColorStop(0, rgba(AMBER, 0.28));
      fillG.addColorStop(1, rgba(AMBER, 0));
      ctx.fillStyle = fillG;
      ctx.fill();

      // glowing spectral line
      tracePath();
      ctx.strokeStyle = "rgba(255,236,180,0.95)";
      ctx.lineWidth = 1.8;
      ctx.shadowBlur = 14; ctx.shadowColor = rgba(AMBER, 0.95);
      ctx.stroke();
      // second pass for a brighter core glow
      ctx.shadowBlur = 6; ctx.shadowColor = "rgba(255,240,200,0.9)";
      ctx.stroke();
      ctx.shadowBlur = 0;

      // sweeping scan highlight
      sweep += 0.004;
      if (sweep > 1.25) sweep = -0.25;
      const sx = padX + sweep * plotW;
      const sw = ctx.createLinearGradient(sx - 40, 0, sx + 40, 0);
      sw.addColorStop(0, rgba(AMBER, 0));
      sw.addColorStop(0.5, rgba(AMBER, 0.10));
      sw.addColorStop(1, rgba(AMBER, 0));
      ctx.fillStyle = sw;
      ctx.fillRect(sx - 40, padTop, 80, maxH);
      // bright sweep edge
      ctx.fillStyle = "rgba(255,240,200,0.10)";
      ctx.fillRect(sx, padTop, 1, maxH);

      t += 0.05;
      raf = requestAnimationFrame(draw);
    };
    raf = requestAnimationFrame(draw);
    return () => { running = false; cancelAnimationFrame(raf); };
  }, [live, height, bins]);

  const freqLabels = ["20", "100", "500", "1k", "5k", "20k"];

  return (
    <div ref={wrapRef} className={`aa-crt ${className}`}
      style={{ position: "relative", width: "100%", height: `${height}px`, background: "var(--crt-bg)", overflow: "hidden" }} {...rest}>
      <canvas ref={canvasRef} style={{ display: "block", width: "100%", height: "100%" }} />

      {/* scan-line texture overlay */}
      <div style={{ position: "absolute", inset: 0, pointerEvents: "none",
        backgroundImage: "repeating-linear-gradient(to bottom, transparent 0, transparent 2px, var(--crt-scanline) 2px, var(--crt-scanline) 3px)" }} />
      {/* inner vignette / curvature */}
      <div style={{ position: "absolute", inset: 0, pointerEvents: "none",
        boxShadow: "inset 0 0 60px rgba(0,0,0,0.7), inset 0 0 14px rgba(228,162,16,0.10)" }} />

      {label ? (
        <span style={{ position: "absolute", top: 8, left: 12, fontFamily: "var(--font-mono)", fontSize: "var(--fs-9)",
          letterSpacing: "0.12em", color: "var(--crt-amber)", textShadow: "0 0 8px rgba(228,162,16,0.7)", opacity: 0.9 }}>{label}</span>
      ) : null}

      <div style={{ position: "absolute", left: 12, right: 12, bottom: 4, display: "flex", justifyContent: "space-between",
        fontFamily: "var(--font-mono)", fontSize: "var(--fs-9)", color: "var(--crt-amber)", opacity: 0.55 }}>
        {freqLabels.map((f) => (<span key={f}>{f}</span>))}
      </div>
    </div>
  );
}
