import * as React from "react";

/**
 * Props for the amber CRT spectrum display.
 * @startingPoint section="Displays" subtitle="Amber CRT spectrum strip" viewport="400x180"
 */
export interface CRTDisplayProps {
  /** 0..1 bar heights (32 bars). Omit to self-animate a plausible spectrum. */
  bars?: number[];
  /** Self-animate when no bars are supplied. Default true. */
  live?: boolean;
  /** Strip height in px. Default 180. */
  height?: number;
  /** Number of spectrum bins/bars. Default 56. */
  bins?: number;
  /** Optional mono corner label (e.g. "SPECTRUM"). */
  label?: string;
  className?: string;
}

/**
 * The plugin's amber phosphor spectrum strip, sci-fi treatment: canvas
 * hot-core bars with bloom, peak-hold caps, neon spectral line, mirrored
 * reflection, sweeping scan highlight, vignette — amber on near-black.
 */
export function CRTDisplay(props: CRTDisplayProps): JSX.Element;
