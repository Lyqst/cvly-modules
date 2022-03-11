# spc
_Pronounced "Space"_
***
![spc](./screenshots/spc.png)

Utility module that spreads an incoming chord using a settable minimum interval distance between notes.

The Oct and Note parameters sets which is the lowest note that the module will output.

For example, for polyphonic input D5 D4 G3 A5 D3, if the minimum distance is set to 3 semitones and lowest note is set as C1, the output will be D1 D2 G2 A3 D4.

An option in the right click menu can be turned on to sort the input before spreading, so for the same case above, the input will be sorted first to D3 G3 D4 A5 D5, and the output will be D1 G1 D2 A2 D3.
