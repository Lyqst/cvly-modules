#include "scales.hpp"

int scales::getNumberOfScales()
{
    return sizeof(notes) / sizeof(notes[0]);
}

const int *scales::getScale(int i)
{
    return notes[i];
}

const std::string scales::getScaleName(int i)
{
    return name[i];
}
