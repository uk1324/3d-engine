#include "PerlinNoise.hpp">
#include "Utils.hpp"
#include "Angles.hpp"

PerlinNoise::PerlinNoise(uint64_t seed)
{
    std::mt19937 randomGenerator(seed);

    for (size_t i = 0; i < GRADIENT_TABLE_SIZE; i++)
    {
        m_permutations[i] = i;
        m_permutations[GRADIENT_TABLE_SIZE + i] = i;
    }

    std::shuffle(m_permutations, m_permutations + GRADIENT_TABLE_SIZE * 2, randomGenerator);

    std::uniform_real_distribution<float> random(0.0f, 1.0f);
    for (size_t i = 0; i < GRADIENT_TABLE_SIZE; i++)
    {
        // This ditributes direction vectors uniformly.
        float a = acos(2 * random(randomGenerator) - 1);
        float b = 2 * random(randomGenerator) * PI<float>;
        m_gradients[i] = Vec3(
            cos(b) * sin(a),
            sin(b) * sin(a),
            cos(a)
        );
    }
}

float PerlinNoise::value3d(const Vec3& p) const
{
    return at(p);
}

float PerlinNoise::value2d(const Vec2& p) const
{
    return at(Vec3(p.x, p.y, 0.5f));
}

float PerlinNoise::accumulatedValue3d(const Vec3& point, int octaves, float lacunarity, float persistence) const
{
    float value = 0.0f;
    float l = 1.0f;
    float p = 1.0f;
    for (int i = 0; i < octaves; i++)
    {
        l *= lacunarity;
        p *= persistence;
        value += (value3d(point * l) * p);
    }
    return value;
}

float PerlinNoise::accumulatedValue2d(const Vec2& p, int octaves, float lacunarity, float persistence) const
{
    return accumulatedValue3d(Vec3(p.x, p.y, 0.5f), octaves, lacunarity, persistence);
}

float PerlinNoise::value3d01(const Vec3& p) const
{
    return (value3d(p) / +1.0f) / 2.0f;
}

float PerlinNoise::value2d01(const Vec2& p) const
{
    return (value2d(p) / +1.0f) / 2.0f;
}

int PerlinNoise::hash(int x, int y, int z) const
{
    return m_permutations[m_permutations[m_permutations[x] + y] + z];
}

float PerlinNoise::accumulatedValueMax(int octaves, float persistence)
{
    // Geometric series sum.
    return persistence * ((1.0f - pow(persistence, octaves)) / (1.0f - persistence));
}

float PerlinNoise::smoothstep(float t)
{
    return t * t * (3 - 2 * t);
}

float PerlinNoise::at(const Vec3& p) const
{
    static const unsigned int tableSizeMask = GRADIENT_TABLE_SIZE - 1;
    int xi0 = ((int)std::floor(p.x)) & tableSizeMask;
    int yi0 = ((int)std::floor(p.y)) & tableSizeMask;
    int zi0 = ((int)std::floor(p.z)) & tableSizeMask;

    int xi1 = (xi0 + 1) & tableSizeMask;
    int yi1 = (yi0 + 1) & tableSizeMask;
    int zi1 = (zi0 + 1) & tableSizeMask;

    float tx = p.x - ((int)std::floor(p.x));
    float ty = p.y - ((int)std::floor(p.y));
    float tz = p.z - ((int)std::floor(p.z));

    float u = smoothstep(tx);
    float v = smoothstep(ty);
    float w = smoothstep(tz);

    // gradients at the corner of the cell
    const Vec3& c000 = m_gradients[hash(xi0, yi0, zi0)];
    const Vec3& c100 = m_gradients[hash(xi1, yi0, zi0)];
    const Vec3& c010 = m_gradients[hash(xi0, yi1, zi0)];
    const Vec3& c110 = m_gradients[hash(xi1, yi1, zi0)];

    const Vec3& c001 = m_gradients[hash(xi0, yi0, zi1)];
    const Vec3& c101 = m_gradients[hash(xi1, yi0, zi1)];
    const Vec3& c011 = m_gradients[hash(xi0, yi1, zi1)];
    const Vec3& c111 = m_gradients[hash(xi1, yi1, zi1)];

    // generate vectors going from the grid points to p
    float x0 = tx, x1 = tx - 1;
    float y0 = ty, y1 = ty - 1;
    float z0 = tz, z1 = tz - 1;

    Vec3 p000 = Vec3(x0, y0, z0);
    Vec3 p100 = Vec3(x1, y0, z0);
    Vec3 p010 = Vec3(x0, y1, z0);
    Vec3 p110 = Vec3(x1, y1, z0);

    Vec3 p001 = Vec3(x0, y0, z1);
    Vec3 p101 = Vec3(x1, y0, z1);
    Vec3 p011 = Vec3(x0, y1, z1);
    Vec3 p111 = Vec3(x1, y1, z1);

    // Trilinear interpolation
    float a = lerp(dot(c000, p000), dot(c100, p100), u);
    float b = lerp(dot(c010, p010), dot(c110, p110), u);
    float c = lerp(dot(c001, p001), dot(c101, p101), u);
    float d = lerp(dot(c011, p011), dot(c111, p111), u);

    float e = lerp(a, b, v);
    float f = lerp(c, d, v);

    float g = lerp(e, f, w);

    return g;
}