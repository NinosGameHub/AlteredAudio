import * as React from "react";

/**
 * Props for the pill toggle switch.
 * @startingPoint section="Controls" subtitle="Pill toggle — green on / grey off" viewport="160x40"
 */
export interface ToggleSwitchProps {
  /** On/off state. */
  checked?: boolean;
  /** Fired with the next state. */
  onChange?: (checked: boolean) => void;
  /** Optional trailing label (e.g. "Active", "Ping-Pong"). */
  label?: string;
  disabled?: boolean;
  className?: string;
}

/**
 * iOS-style pill switch: green when on, grey when off, white sliding thumb.
 * Altered Audio uses it for module Active/bypass and per-feature toggles.
 */
export function ToggleSwitch(props: ToggleSwitchProps): JSX.Element;
