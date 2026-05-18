# Fader Locks

A VST3 mix-fader plugin that locks every fader's mouse-drag motion to a step
size you control globally. Pick a resolution — 10, 5, 3, 2, 1, 0.5, 0.25 dB,
or any custom value — and every Fader Locks instance in the session snaps to
that grid on drag, while keeping its current value untouched when the grid
changes.

It's a way to mix without typing values: pick a coarse step for big moves,
then narrow it as you get into finer details. Think big brush → small brush.

## What's on the plugin

- **Vertical fader** (per channel, −60 dB to +12 dB). Drag is constrained
  to multiples of the current step size *relative to where the drag began*,
  so existing values are preserved when the step changes. Double-click to
  return to 0 dB. The text box accepts typed values without snapping.
- **Step knob** (shared across every instance in the host). Detent
  positions for `Off, 10, 5, 3, 2, 1, 0.5, 0.25 dB`. Double-click the value
  below the knob to type any custom step. Turning the knob on one instance
  updates every other open instance and the saved state of every closed one.

Host automation on the fader is **not** constrained — the lock only applies
to mouse drag. So automation lanes still work the way they always do.

## Build

Requirements: CMake ≥ 3.22, Xcode CLT, git. JUCE is fetched automatically.

```sh
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

The built bundle is auto-installed to `~/Library/Audio/Plug-Ins/VST3/Fader Locks.vst3`.

### Universal binary (Apple Silicon + Intel)

The default build is arm64-native. For a universal bundle that also runs
under Rosetta or on Intel Macs:

```sh
cmake -S . -B build -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release -j
```

## Use

Drop Fader Locks on any track's insert chain (typically last in the chain so
the per-channel main fader can be freed up for automation). Open a second
instance on another track and you'll see the step knobs stay in sync.

## Caveats

- **VST3 only** for now. Logic Pro / GarageBand don't load VST3 — they need
  AU. Adding AU is a one-line `CMakeLists.txt` change (`FORMATS VST3 AU`).
- **"Global" means same DAW process.** Every major VST3 host (Reaper,
  Cubase, Studio One, Ableton, Bitwig, Pro Tools) loads the plugin library
  once per project, which is what makes the shared knob work. A host that
  sandboxes each plugin instance in its own subprocess would break the sync.
- **Ad-hoc signed.** Fine for the machine you build it on. Distributing the
  bundle to another Mac requires either rebuilding from source there or
  Developer ID signing + notarization.
