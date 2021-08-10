#include <string>

namespace scales
{
    const std::string name[12] = {
        "Ionian/Major",
        "Dorian",
        "Phrygian",
        "Lydian",
        "Mixolydian",
        "Aeolian/Minor",
        "Locrian",
        "Harmonic Minor",
        "Whole Tone",
        "Major Pentatonic",
        "Minor Pentatonic",
        "Blues Scale"};

    const int notes[][12] = {
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},
        {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0},
        {1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0},
        {1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0},
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0},
        {1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0},
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0},
        {1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0}};

    int getNumberOfScales();

    const int* getScale(int i);

    const std::string getScaleName(int i);
}