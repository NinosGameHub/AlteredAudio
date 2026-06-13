**Panel** — the module enclosure. Wrap every section (FILTER TYPE, MODULATION, LFO ENGINE, ENVELOPE FOLLOWER, UTILITY) in one. Flat plastic, hairline border, optional titled header with a status LED.

```jsx
<Panel title="MODULATION" led={true} actions={<Badge>ON</Badge>}>
  …controls…
</Panel>
```

Props: `led` (omit = none, `true` = amber engaged, `false` = grey bypassed), `actions` (right-aligned header slot), `inset` (recessed beige fill), `padding`.
