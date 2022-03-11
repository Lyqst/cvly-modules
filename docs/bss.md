# bss
_Pronounced "Bass"_
***
![bss](./screenshots/bss.png)

Utilty module that attempts to find a bass note for an incoming chord. It has three modes: Harmonic, Lowest and Random.

In Harmonic mode, it analyses the intervals between the chord notes, and then tries to choose the best root note based on that. This is a work in progress still, any feedback is appreciated.

The Oct and Note parameters are used to specify what is the lowest note that the module will output, and any other notes are put up to an octave above that note.
