**LED** — a small indicator lamp, glowing amber when on. Use for engage status, signal-present dots, and labelled status markers (e.g. SYSTEM in the footer).

```jsx
<LED on label="SYSTEM" />
<LED on={false} />
```

Props: `color` (lit color, defaults to amber `--state-engaged`), `size`, `label`.
