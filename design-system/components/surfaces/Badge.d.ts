import * as React from "react";

/**
 * Props for the status / count pill.
 * @startingPoint section="Surfaces" subtitle="Status / count pill" viewport="200x48"
 */
export interface BadgeProps {
  children?: React.ReactNode;
  /** "neutral" | "accent" | "active" | "muted". */
  tone?: "neutral" | "accent" | "active" | "muted";
  /** Show a leading status dot. */
  dot?: boolean;
  className?: string;
}

/**
 * Small uppercase status / count pill in the brand's two-accent palette.
 * Use sparingly — for module state ("ACTIVE", "BYPASSED") or counts.
 */
export function Badge(props: BadgeProps): JSX.Element;
