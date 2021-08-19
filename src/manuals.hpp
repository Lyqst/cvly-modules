namespace manuals
{
    enum Modules
    {
        NONE,
        BSS,
        CRCL,
        NTRVLC,
        NTRVLX,
        SPC,
        STPR,
        WHL,
        NUM_MODULES
    };

    const std::string slugs[NUM_MODULES] = {
        "",
        "bss",
        "crcl",
        "ntrvlc",
        "ntrvlx",
        "spc",
        "stpr",
        "whl"
    };

    const std::string text[NUM_MODULES] = {
        "Put me to the right of a cvly module!",
        "Module: bss\nAttempts to find a bass note for an incoming chord. It has three modes: Harmonic, Lowest and Random.\nIn Harmonic mode, it analyses the intervals between the chord notes, and then tries to choose the best root note based on that. This is a work in progress still, any feedback is appreciated.\nThe Oct and Note parameters are used to specify what is the lowest note that the module will output, and any other notes are put up to an octave above that note.",
        "Module: crcl\nMode 1: Quantizer for the voltage coming through the CV input. If the trigger input is connected, it will also act as a Sample & Hold.\n\nMode 2: Sequencer, receiving the clock in the trigger input, and ciclying through the circle of fifths. The output CV is kept inside one octave from the root.\nThe CV input can be used to reverse the direction of the sequence:\n0v or higher: clockwise.\nLower than 0v: counter-clockwise.\n\nMode 3: Exactly like mode 1, except that after quantizing the input signal, it will output the mirror note using the line between the tonic and the fifth as axis. Based on the music theory concept of Negative harmony.",
        "Module: ntrvlc\nFour 8-step sequences with an incorporated equi-likely quantizer. Each of the four sequences has an independent clock input and a separate output, along with a common polyphonic output. Only sequences with a connected clock input will be sent to the poly out.\nIf the quantizer does not have any notes active, the module will output raw voltage.\nVoltage range can be set in the context menu.\n\nIf the stack button is on, the first active sequencer will work normally, but each subsequent sequence will be built on top of the last one, adding or subtracting from the previous voltage. If it's off, each sequencer will function independently.",
        "Module: ntrvlx\n'Weight' attenuates the base CV before adding, so at 1 it works the same as without the expander, and at 0 is the same as stacking off.\n'Snap' makes it so that each output is within the selected voltage range when stacking, taking weight into account:\nOUTn = ((CVn-1 * weight) + CVn) / (1 + weight)\nIt also adds 4 triggers, one for each sequence, that fire whenever the output of its sequence changes. Option to turn first output to poly in the context menu.",
        "Module: spc\nSpreads an incoming chord using a settable minimum interval distance between notes.\nThe Oct and Note parameters sets which is the lowest note that the module will output.\n\nFor example, for polyphonic input D5 D4 G3 A5 D3, if the minimum distance is set to 3 semitones and lowest note is set as C1, the output will be D1 D2 G2 A3 D4.\nAn option in the right click menu can be turned on to sort the input before spreading, so for the same case above, the input will be sorted first to D3 G3 D4 A5 D5, and the output will be D1 G1 D2 A2 D3.",
        "Module: stpr\nA 8-step sequencer. Each step can be activated or deactivated using its trigger input. When the module receives a clock signal, it looks for the next active step, and moves to it. If no step is active, it will remain in the current step (regardless of its active status).\nVoltage range can be set in the context menu.",
        "And wow! Hey! What’s this thing suddenly coming towards me very fast? Very very fast. So big and flat and round, it needs a big wide sounding name like … ow … ound … round … ground! That’s it! That’s a good name – ground!\n\nI wonder if it will be friends with me?\n\n- Douglas Adams, The Hitchhiker's Guide to the Galaxy "
        };

}