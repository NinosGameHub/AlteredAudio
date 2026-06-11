import * as React from "react";

export type SelectOption = string | { value: string; label: string };

/**
 * Props for the flat combo box.
 * @startingPoint section="Controls" subtitle="Flat combo box with chevron" viewport="200x64"
 */
export interface SelectProps {
  value?: string;
  onChange?: (value: string) => void;
  /** Choices — plain strings or {value,label} objects. */
  options?: SelectOption[];
  /** Uppercase micro-label above the box (e.g. "TYPE", "ALGORITHM"). */
  label?: string;
  disabled?: boolean;
  className?: string;
}

/**
 * Flat combo box: white fill, 6px radius, hairline border, grey chevron.
 * Used for module type/mode pickers (Filter type, Waveshaper algorithm…).
 */
export function Select(props: SelectProps): JSX.Element;
