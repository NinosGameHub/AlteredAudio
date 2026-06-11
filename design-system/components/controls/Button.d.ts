import * as React from "react";

/**
 * Props for the text button.
 * @startingPoint section="Controls" subtitle="Primary / secondary / ghost text button" viewport="220x60"
 */
export interface ButtonProps {
  children?: React.ReactNode;
  /** "primary" filled blue · "secondary" sunken grey · "ghost" transparent. */
  variant?: "primary" | "secondary" | "ghost";
  /** "sm" | "md" | "lg". Default "md". */
  size?: "sm" | "md" | "lg";
  /** Fully rounded pill corners (Apple CTA style). */
  pill?: boolean;
  disabled?: boolean;
  /** Optional leading icon node. */
  iconLeft?: React.ReactNode;
  onClick?: (e: React.MouseEvent<HTMLButtonElement>) => void;
  className?: string;
}

/**
 * Text button in three weights. Primary is filled Apple-blue; secondary
 * mirrors the native sunken-grey TextButton; ghost is transparent accent text.
 */
export function Button(props: ButtonProps): JSX.Element;
