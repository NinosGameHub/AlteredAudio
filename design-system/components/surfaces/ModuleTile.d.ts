import * as React from "react";

/**
 * Props for the left-list module tile.
 * @startingPoint section="Surfaces" subtitle="Left-list module tile" viewport="200x48"
 */
export interface ModuleTileProps {
  /** 1-based chain position, rendered as a zero-padded mono number. */
  index?: number;
  /** Module name (e.g. "Filter", "Compressor"). */
  name: string;
  /** Category color (e.g. var(--cat-dynamics)) for selected bar + name. */
  category?: string;
  /** Engaged (green dot) vs bypassed (grey dot). Default true. */
  active?: boolean;
  /** Currently-selected row (surface fill + category bar + colored name). */
  selected?: boolean;
  onClick?: (e: React.MouseEvent<HTMLButtonElement>) => void;
  className?: string;
}

/**
 * One row of the plugin's left tile list: chain number, name, active dot.
 * Stack them in a vertical list to build the module rack.
 */
export function ModuleTile(props: ModuleTileProps): JSX.Element;
