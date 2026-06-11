import * as React from "react";

/**
 * Props for the segmented level / gain-reduction meter.
 * @startingPoint section="Displays" subtitle="Segmented I/O & GR meter" viewport="120x120"
 */
export interface MeterProps {
  /** Normalized 0..1 level (or reduction amount in "gr" mode). */
  value?: number;
  /** "level" lights green→amber→orange upward; "gr" lights orange downward. */
  mode?: "level" | "gr";
  /** "vertical" (default) or "horizontal". */
  orientation?: "vertical" | "horizontal";
  /** Number of segments. Default 16. */
  segments?: number;
  /** Long-axis length in px. Default 80. */
  length?: number;
  /** Short-axis thickness in px. Default 6. */
  thickness?: number;
  /** Optional mono label below (e.g. "L", "R", "GR"). */
  label?: string;
  className?: string;
}

/**
 * Thin segmented meter on CRT-black: header I/O VU meters and the
 * compressor/limiter gain-reduction readouts.
 */
export function Meter(props: MeterProps): JSX.Element;
