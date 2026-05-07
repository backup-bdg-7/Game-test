// TextureManager.hpp - C++ Texture Management for Doomnite Metal Renderer
// Handles texture loading, caching, and Metal texture creation.

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Doomnite {

// MARK: - Texture Format

enum class TextureFormat {
    RGBA8,      // Standard
    RGBA16F,    // HDR
    RGBA32F,    // High precision
    RG11B10F,   // HDR without alpha
    DXT1,       // Compressed
    DXT5        // Compressed with alpha
};

// MARK: - Texture Type

enum class TextureType {
    Albedo,     // Base color
    Normal,     // Normal map
    Metallic,   // Metallic map
    Roughness,  // Roughness map
    AO,         // Ambient occlusion
    Emissive,   // Self-illumination
    Height      // Height/displacement map
};

// MARK: - Texture Descriptor

struct TextureDescriptor {
    std::string id;
    std::string filename;
    TextureType type;
    TextureFormat format;
    bool generateMipmaps;
    bool sRGB;       // For albedo textures
    int anisotropy;   // Anisotropic filtering level (1, 2, 4, 8, 16)
    
    TextureDescriptor(const std::string& texId, const std::string& file,
                   TextureType texType = TextureType::Albedo,
                   TextureFormat fmt = TextureFormat::RGBA8,
                   bool mipmaps = true, bool srgb = true, int aniso = 4)
        : id(texId), filename(file), type(texType), format(fmt),
          generateMipmaps(mipmaps), sRGB(srgb), anisotropy(aniso) {}
};

// MARK: - Texture (Represents loaded texture)

struct Texture {
    std::string id;
    TextureType type;
    int width;
    int height;
    TextureFormat format;
    void* metalTexture;  // MTLTexture pointer (void* for C++/ObjC++ bridging)
    bool hasMipmaps;
    int anisotropy;
    
    Texture(const std::string& texId = "", TextureType texType = TextureType::Albedo)
        : id(texId), type(texType), width(0), height(0),
          format(TextureFormat::RGBA8), metalTexture(nullptr),
          hasMipmaps(false), anisotropy(1) {}
    
    bool isLoaded() const { return metalTexture != nullptr; }
};

// MARK: - Texture Atlas (for batching draw calls)

struct TextureAtlas {
    std::string id;
    std::string atlasFilename;
    std::unordered_map<std::string, Vec4> subTextureCoords;  // name -> (u, v, width, height) in atlas space
    std::shared_ptr<Texture> atlasTexture;
    
    TextureAtlas(const std::string& atlasId, const std::string& filename)
        : id(atlasId), atlasFilename(filename) {}
    
    void addSubTexture(const std::string& name, const Vec4& coords) {
        subTextureCoords[name] = coords;
    }
    
    Vec4 getSubTextureCoords(const std::string& name) const {
        auto it = subTextureCoords.find(name);
        if (it != subTextureCoords.end()) {
            return it->second;
        }
        return Vec4(0, 0, 1, 1);  // Default: entire texture
    }
};

// MARK: - Predefined Game Textures

namespace Textures {
    
    // Character textures
    static TextureDescriptor PlayerAlbedo("tex_player_albedo", "textures/player_albedo.png",
                                             TextureType::Albedo);
    static TextureDescriptor PlayerNormal("tex_player_normal", "textures/player_normal.png",
                                            TextureType::Normal);
    
    // Weapon textures
    static TextureDescriptor SwordAlbedo("tex_sword_albedo", "textures/weapons/sword_albedo.png",
                                            TextureType::Albedo);
    static TextureDescriptor StaffEmissive("tex_staff_emissive", "textures/weapons/staff_emissive.png",
                                             TextureType::Emissive, TextureFormat::RGBA16F);
    
    // Dragon textures
    static TextureDescriptor DragonAlbedo("tex_dragon_albedo", "textures/companions/dragon_albedo.png",
                                            TextureType::Albedo);
    static TextureDescriptor DragonNormal("tex_dragon_normal", "textures/companions/dragon_normal.png",
                                           TextureType::Normal);
    
    // Environment
    static TextureDescriptor TerrainAlbedo("tex_terrain_albedo", "textures/world/terrain_albedo.png",
                                             TextureType::Albedo);
    static TextureDescriptor Skybox("tex_skybox", "textures/world/skybox.png",
                                        TextureType::Albedo, TextureFormat::RGBA16F);
    
    static std::vector<TextureDescriptor> AllTextures = {
        PlayerAlbedo, PlayerNormal,
        SwordAlbedo, StaffEmissive,
        DragonAlbedo, DragonNormal,
        TerrainAlbedo, Skybox
    };
}

// MARK: - Texture Manager

class TextureManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<TextureAtlas>> atlases;
    
public:
    TextureManager() {}
    
    bool loadTexture(const TextureDescriptor& descriptor, void* device) {
        // In real implementation:
        // 1. Check if already loaded
        // 2. Load image data (using stb_image or MetalKit)
        // 3. Create MTLTexture with device
        // 4. Upload data to texture
        
        auto texture = std::make_shared<Texture>(descriptor.id, descriptor.type);
        texture->format = descriptor.format;
        // texture->metalTexture = createdTexture;
        
        textures[descriptor.id] = texture;
        return true;
    }
    
    std::shared_ptr<Texture> getTexture(const std::string& textureId) {
        auto it = textures.find(textureId);
        if (it != textures.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    bool loadAtlas(const TextureAtlas& atlas, void* device) {
        // Load atlas texture
        // Parse atlas descriptor file (JSON/XML) for sub-texture coordinates
        // Store in atlas.subTextureCoords
        atlases[atlas.id] = std::make_shared<TextureAtlas>(atlas);
        return true;
    }
    
    std::shared_ptr<TextureAtlas> getAtlas(const std::string& atlasId) {
        auto it = atlases.find(atlasId);
        if (it != atlases.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    void unloadTexture(const std::string& textureId) {
        textures.erase(textureId);
    }
    
    void unloadAll() {
        textures.clear();
        atlases.clear();
    }
    
    int getLoadedCount() const {
        return static_cast<int>(textures.size());
    }
};

} // namespace Doomnite
