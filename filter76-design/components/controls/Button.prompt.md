**Button** — text & utility button. `variant="default"` is the flat cream key (transport ‹ ›, SOLO, BYPASS); `variant="toggle"` fills amber when `active`; `variant="ghost"` is low-emphasis.

```jsx
<Button square>S</Button>
<Button variant="toggle" active={solo} onClick={() => setSolo(!solo)}>SOLO</Button>
<Button icon={<LucideChevronLeft/>} variant="default">PREV</Button>
```

Use `square` for the single-glyph S / B keys. For multi-option exclusive choices use `SegmentedControl` / `OptionList` instead.
