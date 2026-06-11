`Badge` is a small uppercase status pill; use it for module state or counts, keeping to the two-accent palette.

```jsx
<Badge tone="active" dot>Active</Badge>
<Badge tone="muted" dot>Bypassed</Badge>
<Badge tone="accent">VST3</Badge>
```

- `tone="active"` (green) signals engaged; `muted` signals bypassed — same semantics as the chain-strip dot.
- Keep text terse and uppercase to match the UI voice.
