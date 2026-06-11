import * as React from "react";

/**
 * Props for the Altered Audio flat-disc rotary knob.
 * @startingPoint section="Controls" subtitle="Flat Braun-style knob with indicator line" viewport="120x120"
 */
export interface KnobProps {
  /** Current value. */
  value?: number;
  /** Range minimum. Default 0. */
  min?: number;
  /** Range maximum. Default 1. */
  max?: number;
  /** Diameter in px. Default 64. */
  size?: number;
  /** Uppercase micro-label shown above the knob (e.g. "FREQ"). */
  label?: string;
  /** Formatted value string shown below (e.g. "440 Hz", "-12.0 dB"). */
  display?: React.ReactNode;
  /** Category color for the optional value-sweep ring. */
  accent?: string;
  /** Draw a category-colored value-sweep ring outside the disc. */
  showArc?: boolean;
  /** Active modulation routings, drawn as thin colored arcs from the current
   *  value by each (bipolar) depth — concentric when more than one. */
  mods?: { depth: number; color?: string }[];
  /** Greys out the line + disc (bypassed module). */
  disabled?: boolean;
  /** Fired on interaction with the current value. */
  onChange?: (value: number) => void;
  className?: string;
}

/**
 * Altered Audio flat-disc rotary knob: dirty-cream disc, near-black
 * indicator line, range-marker dots at 7 & 5 o'clock — Braun/Moog hardware
 * feel. Optionally a category-colored value-sweep ring.
 */
export function Knob(props: KnobProps): JSX.Element;
