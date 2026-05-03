# Gloubiboulga — Logic Pro AU Testing Guide

## Prerequisites

- macOS 13 (Ventura) or later
- Logic Pro 10.7 or later
- The **Gloubiboulga-macOS.pkg** installer from the Releases page

---

## Step 1 — Install the plugin

1. Double-click **Gloubiboulga-macOS.pkg** and follow the prompts.  
   An administrator password is required.
2. The installer copies two files:
   - `Gloubiboulga.vst3` → `/Library/Audio/Plug-Ins/VST3/`
   - `Gloubiboulga.component` → `/Library/Audio/Plug-Ins/Components/`

---

## Step 2 — Allow the plugin in System Settings

Because Gloubiboulga is not signed with an Apple Developer certificate, macOS blocks it on first use.

1. **Open Logic Pro.** Logic will scan the new plugin and macOS will silently block it.
2. Open **System Settings → Privacy & Security**.
3. Scroll down to the **Security** section. You will see:  
   > *"Gloubiboulga.component" was blocked because it is not from an identified developer.*
4. Click **Allow Anyway**.
5. You may see a second message for `Gloubiboulga.vst3` — click **Allow Anyway** again if so.
6. **Quit and relaunch Logic Pro.** macOS will show a final confirmation dialog — click **Open** or **Allow**. The plugin loads from this point on.

> If the "Allow Anyway" button does not appear, make sure you opened Logic first so that macOS had a chance to register the block.

---

## Step 3 — Validate the plugin with auval (recommended)

Before testing in Logic, run Apple's AU validator in Terminal. This catches any initialization problems and gives a clear error message instead of a silent crash.

Open **Terminal** and run:

```bash
auval -v aufx GLOU FXME
```

A successful run ends with:

```
--------------------------------------------------
AU VALIDATION SUCCEEDED.
--------------------------------------------------
```

If you see failures, copy the full output and send it to the developer.

---

## Step 4 — Rescan in Logic's Plug-in Manager

Logic caches plugin scan results. If you see Gloubiboulga listed as *Failed* or it does not appear at all:

1. Open **Logic Pro → Settings → Plug-in Manager**.
2. Search for **Gloubiboulga** in the list.
3. Click **Reset & Rescan Selection**.
4. Wait for the scan to complete. The status should change to a green checkmark.

---

## Step 5 — Test in a Logic session

1. Create a new Empty Project with one **Audio** track (stereo).
2. On the track's channel strip, click the first **Audio FX** slot.
3. Navigate to **Audio Units → Fx-Mechanics → Gloubiboulga**.
4. The plugin window should open without crashing.
5. Start playback. The glitch visualizer at the top should animate.

### Golden-path checks

| Test | Expected result |
|------|----------------|
| Plugin loads on an audio track | Window opens, no crash |
| Playback starts | Visualizer animates, audio passes through |
| Mix knob at 0% | Fully dry signal, no effect |
| Mix knob at 100% | Glitch effect audible |
| Change any knob while playing | New glitch cycle generated, no glitch or crash |
| Save and reopen the session | Plugin reloads with the same parameter values |
| Playback stops and restarts | Plugin resynchronises to the transport, no crash |

---

## Troubleshooting

**Logic crashes immediately on opening the session**  
Run `auval -v aufx GLOU FXME` in Terminal and send the output to the developer.

**Plugin shows as "Failed" in Plug-in Manager even after Allow Anyway**  
Repeat Step 2 — sometimes the allow dialog appears a second time after the first rescan.

**No audio output**  
Check that the Mix knob is above 0% and that the track has audio playing into it. The plugin is an effect — it needs an input signal.

**Visualizer is blank**  
The visualizer only animates during Logic playback. Start the transport.

---

## Reporting a crash

If Logic crashes, please send:

1. The crash report from **Console.app**:  
   Open Console → select **Crash Reports** in the sidebar → find the most recent `Logic Pro` entry → copy its full text.
2. The `auval` output from Step 3.
3. A description of what you were doing when the crash occurred (loading the plugin, starting playback, changing a parameter, etc.).
