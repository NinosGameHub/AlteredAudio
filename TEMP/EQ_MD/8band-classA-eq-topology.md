# 8-Band Class A EQ — Topology & Band Map

A working reference for understanding how an analog **Class A**, **8-band** EQ is built and what each band does — then translating that into DSP so your VST inherits the "Class A" character instead of just a clean digital curve.

Important framing up front: **"Class A" is a circuit-design class, not a filter type.** In software there's no transistor biasing, so you don't get Class A "for free." What you actually model is the *behavior* Class A produces: a specific harmonic signature, soft saturation, and frequency-dependent coloration. The filters do the EQ; the Class A modeling does the *vibe*. This doc covers both.

---

## Part 1 — What "Class A topology" actually means

### 1.1 Amplifier classes (the 30-second version)

Every active EQ has gain stages (amplifiers). How those amplifiers conduct current defines their "class":

- **Class A** — the amplifying device (tube or transistor) conducts current through the **entire** signal cycle, 100% of the time. It's always "on," even with no signal.
- **Class B** — each device conducts only half the cycle (one for the positive half, one for the negative), handing off at the zero-crossing.
- **Class AB** — a compromise; both devices conduct, overlapping near the zero-crossing. Most cheap/efficient gear is AB.

### 1.2 Why Class A sounds the way it does

- **No crossover distortion.** Because one device handles the whole waveform, there's no hand-off glitch at the zero-crossing that Class B/AB suffer. The signal stays continuous.
- **Even-order harmonics dominate.** A single-ended Class A stage driven into nonlinearity tends to add **2nd, 4th** harmonics — octaves of the fundamental, which the ear hears as "warm," "thick," "musical." (Class B/push-pull tends to cancel evens and emphasize odd harmonics — 3rd, 5th — which read as "harder" or "edgier.")
- **Soft, gradual saturation.** Class A stages don't clip hard; they round off progressively as they're pushed. This is the "glue" and "analog smoothness" people chase.
- **Costs:** runs hot, inefficient, more expensive, lower headroom before saturation. In hardware these are real downsides; in your VST they cost nothing.

### 1.3 Class A signal flow in an 8-band EQ

A typical topology, input to output:

```
IN → Input buffer/gain (Class A) → [ Band 1 ] → [ Band 2 ] → ... → [ Band 8 ]
   → Make-up / output amp (Class A) → OUT
```

Two structural variants you'll see:

- **Active series (parametric) topology.** Each band is an active filter stage in series. Common in parametric EQs (e.g. GML-style). Surgical, clean, flexible Q. The Class A coloration comes mostly from the input and output amps, plus each band's op-amp stage.
- **Passive (inductor/LC) + Class A make-up gain.** The EQ curves are formed by *passive* components (inductors, capacitors, resistors) which **lose** level, then a Class A amplifier restores the lost gain. Classic in Pultec-style and many "vintage" units. The passive network sounds smooth and the make-up amp adds the Class A color. This is why "passive EQ" and "Class A" so often appear together.

The key DSP insight: **the coloration is concentrated in the gain stages, and it's level-dependent.** Push more signal in, get more harmonics. That's the thing to model.

---

## Part 2 — The 8-Band Map

This is a sensible full-range musical split. Centers are starting points, not law — in a real design the end bands are shelves/cuts and the middle bands are bells, often with selectable or sweepable frequencies.

| # | Name | Range (center) | Filter type | What it does |
|---|------|----------------|-------------|--------------|
| 1 | **Sub / Infra** | 20–60 Hz (~30 Hz) | High-pass + low shelf | Rumble removal, sub weight. The high-pass cleans subsonic mud; the shelf adds or trims felt-not-heard low end. |
| 2 | **Low / Bass** | 60–120 Hz (~80 Hz) | Bell or low shelf | Fundamental weight of kick, bass, floor toms. Boost for power, cut to tighten. |
| 3 | **Low-mid** | 120–400 Hz (~250 Hz) | Bell | The "mud/warmth" zone. Cut to clean up boxiness and congestion; small boost adds body. The most-cut band in mixing. |
| 4 | **Mid** | 400 Hz–1 kHz (~600 Hz) | Bell | Body and "honk/boxiness." Vocals and snares live here. Cut to de-box, boost for fullness. |
| 5 | **Upper-mid** | 1–3 kHz (~2 kHz) | Bell | Presence and attack — pick attack, vocal intelligibility, snare crack. Sensitive band; small moves are loud. |
| 6 | **Presence** | 3–6 kHz (~4 kHz) | Bell | Definition, edge, bite. Boost for clarity/forwardness, cut to tame harshness and listening fatigue. |
| 7 | **High / Treble** | 6–12 kHz (~8 kHz) | Bell or high shelf | Brightness, sheen, sibilance. Shelf boost for sparkle; watch for "ess" harshness on vocals. |
| 8 | **Air / Ultra** | 12–20 kHz+ (~16 kHz) | High shelf | Open, airy top. A gentle shelf here is the classic "expensive" sheen (Maag/Pultec-style air band). |

Per-band controls in a Class A-style design typically expose: **frequency** (fixed-step or sweepable), **gain**, and often **Q/bandwidth** on the bell bands. End bands frequently offer a **bell-vs-shelf** switch.

A design note that makes it feel analog: in many classic units **Q narrows automatically as gain increases** (proportional-Q). Boosting +3 dB gives a broad, gentle bump; boosting +12 dB tightens it. Build this as an optional behavior — it's a big part of why hardware EQs feel musical rather than clinical.

---

## Part 3 — Modeling Class A in your VST (the part that makes it "better")

You already have the filters from your existing build. Class A character is what you layer on top. Three ingredients:

### 3.1 Harmonic saturation (the core of it)

Add a gentle nonlinear waveshaper to emulate the always-on tube/transistor stage. Key requirements:

- **Emphasize even harmonics (2nd, 4th).** A purely odd-harmonic shaper (like a symmetric `tanh`) sounds more like Class B/distortion. For Class A warmth you want an **asymmetric** transfer curve so even harmonics appear. A common approach: a polynomial or `tanh` with a DC/bias offset, or blend a squared term in.
  - Symmetric (odd-only): `y = tanh(drive * x)`
  - Asymmetric (adds evens): `y = tanh(drive * (x + bias)) - tanh(drive * bias)` — the bias is what generates the even-harmonic content; subtract the DC so output stays centered.
- **Make it level-dependent.** Low signal ≈ nearly linear (clean). As level rises, harmonics increase smoothly. This is the single most important realism cue — static distortion sounds fake.
- **Keep it subtle.** Real Class A coloration at normal levels is small (fractions of a percent THD). Resist the urge to overdo it; "warmth," not "fuzz."

### 3.2 Placement matters

Mirror the analog signal flow. Don't smear saturation evenly across the whole chain:

- A saturation stage at the **input** (drives the whole signal) and/or at the **output** (make-up amp) matches the passive-EQ-plus-Class-A-makeup topology and is the most authentic-sounding placement.
- Per-band saturation (a touch of drive inside each band) is more like the active-series topology and gives a more "colored when boosted" feel. Both are valid — pick based on which classic units you're chasing.

### 3.3 Oversampling (non-negotiable for saturation)

Any nonlinearity generates harmonics above Nyquist that fold back as **aliasing** — harsh, inharmonic junk that instantly sounds digital. Run the saturation stages at **2x–4x oversampling** (upsample → saturate → downsample with a clean filter). Your linear filters don't need it, but the waveshapers do. This is the difference between "analog warmth" and "harsh fizz."

### 3.4 Optional realism touches

- **Frequency-dependent saturation.** Real Class A stages don't saturate flat across the band — often a gentle high-frequency softening as they're pushed. A light low-pass in the saturation feedback path emulates this.
- **Subtle noise floor / hum.** Some emulations add a faint, level-appropriate noise bed for "alive" feel. Easy to overdo — make it defeatable.
- **Auto make-up / gain compensation.** Since saturation changes perceived loudness, give users a level-matched bypass so they judge tone, not volume.

---

## Part 4 — Build checklist to upgrade your existing EQ

1. Keep your current 8-band filter engine (the curves are correct already).
2. Add an **asymmetric, level-dependent waveshaper** as a "Drive" / "Class A" stage. Start with the biased-`tanh` above.
3. Wrap the waveshaper in **2x–4x oversampling**.
4. Place one stage at input, one at output (passive-makeup model) — or expose a per-band drive (active model). A/B both, keep what sounds right.
5. Add a **proportional-Q** option so boosts auto-narrow — instant "analog" feel.
6. Add **gain-matched bypass** so the warmth isn't just "louder = better."
7. Tune harmonic balance by ear against a reference (a known Class A plugin or hardware capture): you want predominantly 2nd/4th, low total THD at nominal level, rising smoothly with drive.

The honest summary: your filters give you the *EQ*. The asymmetric, oversampled, level-dependent saturation in the gain stages is what earns the words "Class A." Get those two right and the rest is taste.

---

*Note: amplifier-class behavior and EQ band conventions described here are standard analog-audio engineering. Specific frequency centers and "what each band does" are typical practice, not fixed standards — adjust to your target sound. If you're cloning a specific hardware unit, measure its actual band centers, Q behavior, and harmonic profile rather than relying on these defaults.*
