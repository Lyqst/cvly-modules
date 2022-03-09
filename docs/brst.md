# brst
_Pronounced "Burst"_
***
![brst](https://library.vcvrack.com/screenshots/cvly/brst.m.png)

Burst generator with up to 8 outputs.

The module receives a CV input and a Gate input. As long as the gate is on, it will generate bursts that will be output to one of eight channels, depending on the mode:

* **Single**: each burst will output to a single channel, chosen randomly using the weight parameter of each channel, so that channels with a higher weight have a better chance of being selected.
* **Multiple**: each burst will output to multiple channels, chosen individually according to the weight, so that channels with weight equal to 1 are chosen everytime.
* **Sequencer**: each burst will move to the next sequential channel. In this mode, weight is the chance that the burst will actually move to the next channel, so at weight 0.5, there's equal chance that the next channel is selected or that the currently selected channel is selected once again.

In all modes, channels with weight equal to 0 are inactive.

Time between bursts is controlled by the _time_ and _var_ parameters, with _time_ being the base, and _var_ the variation in percentage, so for example at 50%, the actual time will vary between 0.5 * base and 1.5 * base.

The CV output of each channel can be set to Sample & Hold or pass-thru from the CV input.