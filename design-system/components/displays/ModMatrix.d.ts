import * as React from "react";

export interface ModSource {
  id: string;
  label: string;
  /** Row accent + positive-depth bar color. */
  color?: string;
}
export interface ModDest {
  id: string;
  label: string;
}

/**
 * Props for the modulation routing matrix.
 * @startingPoint section="Displays" subtitle="Modulation routing matrix" viewport="560x320"
 */
export interface ModMatrixProps {
  /** Mod sources (rows): LFOs, envelopes, macros. */
  sources?: ModSource[];
  /** Destination parameters (columns). */
  destinations?: ModDest[];
  /** Depth map keyed "<sourceId>:<destId>" → -1..+1. */
  values?: Record<string, number>;
  /** Fired with the next values map on drag / clear. */
  onChange?: (values: Record<string, number>) => void;
  /** Cell size in px. Default 38. */
  cell?: number;
  className?: string;
}

/**
 * Modulation routing grid: sources down, destinations across, each cell a
 * bipolar depth bar (positive in the source color, negative in warm-grey).
 * Drag a cell vertically to set depth, double-click to clear.
 */
export function ModMatrix(props: ModMatrixProps): JSX.Element;
