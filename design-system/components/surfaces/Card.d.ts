import * as React from "react";

/**
 * Props for the flat surface card.
 * @startingPoint section="Surfaces" subtitle="White surface card with selected state" viewport="320x180"
 */
export interface CardProps {
  children?: React.ReactNode;
  /** Applies the selected recipe: faint-blue fill, blue border, 8px radius. */
  selected?: boolean;
  /** Optional header row (44px) with a bold title; e.g. a module name. */
  header?: React.ReactNode;
  /** Body padding. Default --space-4 (16px). */
  padding?: string;
  onClick?: (e: React.MouseEvent<HTMLDivElement>) => void;
  className?: string;
  style?: React.CSSProperties;
}

/**
 * Flat white surface card with a hairline border and 6px radius. The
 * building block for module panels and any boxed content.
 */
export function Card(props: CardProps): JSX.Element;
