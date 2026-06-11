`CRTDisplay` is the amber phosphor spectrum strip that sits above the detail panel; use it as the plugin's hero visualization or any analyzer readout.

```jsx
<CRTDisplay label="SPECTRUM" />
<CRTDisplay bars={myFftBins} live={false} height={120} />
```

- Omit `bars` to let it self-animate a plausible tilted spectrum; pass a 32-length 0..1 array to drive it.
- Always on near-black glass with amber bars — don't recolor; the amber-on-black CRT is a fixed brand surface.
