#pragma once
#include "Math.hpp"
#include "Material.hpp"


// represents a point light in 3d space
// note: only diffuse/ambient/specular colors from material are used, other values are ignored
class Light {
private:
    Vec3 position;
    Material material;

    static constexpr Vec3 DEFAULT_POSITION { 0, 0, 0 };
    static constexpr Color DEFAULT_AMBIENT_COLOR  { 0.20f, 0.20f, 0.20f };
    static constexpr Color DEFAULT_DIFFUSE_COLOR  { 1.00f, 1.00f, 1.00f };
    static constexpr Color DEFAULT_SPECULAR_COLOR { 1.00f, 1.00f, 1.00f };

public:
    Light(const Vec3& position, const Color& ambientColor, const Color& diffuseColor, const Color& specularColor) :
        position(position),
        material(ambientColor, diffuseColor, specularColor, 1.00f, 0.00f, 0.00f, 1.00f) {}

    Light(const Vec3& position, const Material& material) :
        Light(position, material.getAmbientColor(), material.getDiffuseColor(), material.getSpecularColor()) {}
    
    Light(const Vec3& position=Vec3(0, 0, 0)) :
        Light(position, DEFAULT_AMBIENT_COLOR, DEFAULT_DIFFUSE_COLOR, DEFAULT_SPECULAR_COLOR) {}

    void setColors(const Color& ambientColor, const Color& diffuseColor, const Color& specularColor) {
        this->material.setAmbientColor(ambientColor);
        this->material.setDiffuseColor(diffuseColor);
        this->material.setSpecularColor(specularColor);
    }
    void setAmbientColor(const Color& ambientColor)   { this->material.setAmbientColor(ambientColor);   }
    void setDiffuseColor(const Color& diffuseColor)   { this->material.setDiffuseColor(diffuseColor);   }
    void setSpecularColor(const Color& specularColor) { this->material.setSpecularColor(specularColor); }

    const Vec3&     getPosition()   const { return position;                    }
    const Material& getMaterial()   const { return material;                    }
    const Color& getAmbientColor()  const { return material.getAmbientColor();  }
    const Color& getDiffuseColor()  const { return material.getDiffuseColor();  }
    const Color& getSpecularColor() const { return material.getSpecularColor(); }
};
inline std::ostream& operator<<(std::ostream& os, const Light& light) {
    os << std::fixed << std::setprecision(2)
       << "Light("
         << "position:(" << light.getPosition() << "), "
         << "material:"  << light.getMaterial()
       << ")";
    return os;
}
