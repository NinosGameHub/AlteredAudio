`ModuleTile` is a single row of the plugin's left tile list — chain number, module name, active dot; stack them vertically to build the rack.

```jsx
<div style={{ display: "flex", flexDirection: "column" }}>
  <ModuleTile index={1} name="Filter" category="var(--cat-filtereq)" active selected />
  <ModuleTile index={2} name="Delay" category="var(--cat-time)" active />
  <ModuleTile index={3} name="Reverb" category="var(--cat-time)" active={false} />
</div>
```

- `selected` fills the row, colors the name, and shows the 3px category bar — only one tile is selected at a time.
- `active` toggles the dot green (engaged) vs grey (bypassed).
- Pass the module's `category` color so selection matches its group (dynamics, time, modulation, filter/eq, spatial).
