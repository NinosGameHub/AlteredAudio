`Card` is the flat white surface — hairline border, 6px radius, no shadow; use it for module panels and any boxed content. Pass `header` for a 44px titled module-panel header.

```jsx
<Card header="COMPRESSOR">
  <KnobRow />
</Card>

<Card selected onClick={choose}>Tap to select</Card>
```

- `selected` switches to the faint-blue / blue-border / 8px recipe — the brand's "chosen" motif.
- Don't add drop shadows; separation comes from the border.
