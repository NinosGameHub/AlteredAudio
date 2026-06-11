import * as React from "react";

export interface EQBand {
  /** Center/corner frequency in Hz (20..20000). */
  freq: number;
  /** Gain in dB (-18..18). */
  gain: number;
  /** Resonance / bandwidth. */
  q?: number;
  /** "Peak" | "LowShelf" | "HighShelf" (affects curve shape). */
  type?: string;
  /** Band enabled. */
  on?: boolean;
  /** Optional per-band handle color (defaults to accent). */
  color?: string;
}

/**
 * Props for the EQ frequency-response display.
 * @startingPoint section="Displays" subtitle="EQ response curve with draggable bands" viewport="560x220"
 */
export interface EQCurveProps {
  /** The EQ bands to plot + sum into the response curve. */
  bands?: EQBand[];
  /** Fired with the next bands array when a handle is dragged. */
  onChange?: (bands: EQBand[]) => void;
  width?: number;
  height?: number;
  /** Draw a faint amber spectrum behind the curve. Default true. */
  showSpectrum?: boolean;
  /** Curve + fill color. Default Filter/EQ blue. */
  accent?: string;
  className?: string;
}

/**
 * EQ module's frequency-response graph: CRT-black glass, log-frequency grid,
 * amber spectrum, blue summed-response curve, draggable per-band handles.
 */
export function EQCurve(props: EQCurveProps): JSX.Element;
