#pragma once

// TODO: Compléter les coordonnées de texture.
//       On veut que les texture se répètent 10 fois sur le long du sol.
//       Pour le sol, on aura aussi une répétition de 5 sur le large du sol.

float ground[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 0.0f
};

unsigned int planeElements[] =
{
    0, 2, 1,
    0, 3, 2
};

float street[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 0.0f
};

