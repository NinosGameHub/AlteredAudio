`ToggleSwitch` is the iOS-style pill switch — green track when on, grey when off; use it for binary on/off state like a module's Active/bypass toggle or a per-feature switch.

```jsx
<ToggleSwitch checked={active} onChange={setActive} label="Active" />
<ToggleSwitch checked={pingPong} onChange={setPingPong} label="Ping-Pong" />
```

- The label sits to the right in plain 11px text (capitalized phrase, not all-caps).
- Green = engaged; this is the brand's single "active" signal — don't recolor it.
