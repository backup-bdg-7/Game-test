// ShaderTypes.h - Metal Shader Types for Doomnite
// Shared between C++ and Metal shaders

#pragma once

#include <simd/simd.h>

namespace Doomnite {

// MARK: - Vertex Input

struct Vertex {
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texCoord;
    simd::float3 tangent;
    simd::float3 bitangent;
};

// MARK: - Shader Uniforms

struct ModelConstants {
    simd::float4x4 modelMatrix;
    simd::float4x4 normalMatrix;  // Inverse-transpose of model matrix
};

struct ViewConstants {
    simd::float4x4 viewMatrix;
    simd::float4x4 projectionMatrix;
    simd::float3 cameraPosition;
    float time;
};

struct MaterialConstants {
    simd::float3 albedo;        // Base color
    float metallic;         // 0.0 = dielectric, 1.0 = metal
    float roughness;        // 0.0 = smooth, 1.0 = rough
    float ao;              // Ambient occlusion
    float emissive;        // Self-illumination
    simd::float3 emissiveColor;  // Emissive tint
};

struct LightConstants {
    simd::float3 position;
    float intensity;
    simd::float3 color;
    float radius;           // For point lights
    simd::float3 direction;      // For directional lights
    int type;               // 0 = directional, 1 = point, 2 = spot
    float innerCutoff;     // For spot lights
    float outerCutoff;
    int shadowIndex;        // Shadow map index (-1 = no shadow)
};

// MARK: - PBR Shader Textures

struct TextureIndices {
    int albedoIndex;        // Albedo/Base color texture
    int normalIndex;       // Normal map
    int metallicIndex;      // Metallic map
    int roughnessIndex;     // Roughness map
    int aoIndex;           // Ambient occlusion map
    int emissiveIndex;     // Emissive map
};

// MARK: - Shadow Constants

struct ShadowConstants {
    simd::float4x4 lightSpaceMatrix;  // For shadow mapping
    float shadowBias;
    float shadowStrength;
    int cascadeIndex;      // For cascaded shadow maps
    int pcfSamples;         // Percentage closer filtering samples
};

// MARK: - Particle Shader Types

struct ParticleVertex {
    simd::float3 position;
    simd::float4 color;
    float size;
    float lifetime;
};

struct ParticleUniforms {
    simd::float4x4 viewProjection;
    float time;
    float deltaTime;
};

// MARK: - Post-Processing Types

struct PostProcessConstants {
    float exposure;         // For tone mapping
    float gamma;            // Gamma correction
    float bloomThreshold;   // Bloom extraction threshold
    float bloomIntensity;   // Bloom blur intensity
    simd::float3 fogColor;       // Fog color
    float fogDensity;        // Fog density
};

// MARK: - Skinning (for animated models)

struct SkinningConstants {
    simd::float4x4 jointMatrices[128];  // Maximum joints
    int jointCount;
};

} // namespace Doomnite
