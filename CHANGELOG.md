## 2.0.1
### Fixes:
* Fixed Font references stored across frames in bss and spc ([issue #9](https://github.com/Lyqst/cvly-modules/issues/9))
### Other changes:
* Changed manual urls to point to v2 page and added missing manual url for vbrt

## 2.0.0
### Main:
* Updated for Rack 2.
* UI rework.
### New modules
* **brst**: Burst generator with up to 8 outputs.
* **vbrt**: Multiple pitch voltage detuner with incorporated per-channel LFO.
### Other changes:
* Added polyphonic gate input to **bss** and **spc**.
* Added context menu option to **bss** to select minimum working polyphony.

## 1.0.4
### New modules:
* **txt**: Informative expander for all cvly modules.
* **whl**: 3hp blank panel with cvly whale logo.
### Fixes:
* Fixed crash in Mac/Linux when connecting a cable to the input of **bss** or **spc**.

## 1.0.3
### New modules:
* **bss**: Generates a bass note for the incoming poly input, using one of three modes.
* **spc**: Spreads the notes for incoming poly input, with a settable minimum interval distance.
### UI:
* Added stroke to output backgrounds.
* Changed color of LED lights to match cvly colors.
* Logo now black with green stroke.
### Other changes:
* **ntrvlc**: Added LED screen that shows corresponding note when a knob is being adjusted (if the quantizer is on).

## 1.0.2
- New module: **crcl**.
- **ntrvlx**: added connection light, and context menu option to use first trigger out as poly out.
- **stpr**: changed trigger leds to button leds, to be able to turn on/off steps without input triggers.

## 1.0.1
- New expander for ntrvlc: ntrvlx.
- Changed reset button in ntrvlc for a trigger input.
- Added whale logo to modules :whale2:

## 1.0.0
- First release with two modules: ntrvlc and stpr
