#pragma once

float ground[] =
{
    // Position           // Texture coordinates (5x on width, 10x on length)
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   10.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   10.0f, 5.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 5.0f
};

unsigned int planeElements[] =
{
    0, 2, 1,
    0, 3, 2
};

float street[] =
{
    // Position           // Texture coordinates (repeat 10x on length)
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   10.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   10.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f
};

