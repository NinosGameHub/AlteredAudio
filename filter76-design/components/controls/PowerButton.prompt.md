**PowerButton** — the global power / bypass key in the header. Round cream button with the power glyph and a status LED that glows amber when engaged.

```jsx
const [power, setPower] = React.useState(true);
<PowerButton on={power} onChange={setPower} />
```
