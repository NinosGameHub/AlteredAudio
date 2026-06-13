**Select** — flat combo box for routing pickers (MODULATION SOURCE / DESTINATION). Uppercase mono, cream fill, hairline border, chevron.

```jsx
<Select label="SOURCE" value={src} onChange={setSrc}
  options={["OFF","LFO A","LFO B","ENV"]} />
```

Use a vertical `OptionList` when the choices benefit from an LED (filter type / slope); use `Select` for compact dropdown routing.
