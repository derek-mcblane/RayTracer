#pragma once
#include "Math.hpp"
#include "Colors.hpp"
#include <ostream>
#include <vector>
#include <fstream>


// framebuffer
class FrameBuffer {
private:
    const size_t width;
    const size_t height;
    const size_t numPixels;
    const float megaPixels;
    const Color defaultColor;
    std::vector<std::vector<Color>> pixels;

    float computeMegaPixels(size_t width, size_t height, size_t numDigits=2) {
        return Math::roundToNearestDigit((width * height) / 1000000.0f, numDigits);
    }
public:
    FrameBuffer(size_t width, size_t height, const Color& defaultColor) :
            width(width),
            height(height),
            numPixels(width * height),
            megaPixels(computeMegaPixels(width, height)),
            pixels(),
            defaultColor(defaultColor) {
        pixels.resize(height, std::vector<Color>(width, defaultColor));
    }
    FrameBuffer(const Vec2& size, const Color& defaultColor) :
        FrameBuffer(static_cast<size_t>(size.x), static_cast<size_t>(size.y), defaultColor)
    {}

    constexpr size_t getWidth()                   const noexcept { return width;            }
    constexpr size_t getHeight()                  const noexcept { return height;           }
    constexpr size_t getNumPixels()               const noexcept { return numPixels;        }
    constexpr float getMegaPixels()               const noexcept { return megaPixels;       }
    constexpr const Color& getDefaultColor()      const noexcept { return defaultColor;     }
    const Color& getColor(size_t row, size_t col) const noexcept { return pixels[row][col]; }
    std::string getImageDescription() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2)
            << width << "X" << height
            << " (" << getMegaPixels() << " MP)";
        return ss.str();
    }

    void setColor(size_t row, size_t col, const Color& color) noexcept {
        pixels[row][col] = color;
    }

    // save framebuffer as an array of 256 rgb-colored pixels, written to file at given location
    // note that the buffer stores colors relative to top left corner, while ppm is relative to the bottom left
    void writeToFile(const std::string& filename) {
        std::ofstream ofs(filename + ".ppm", std::ios::out | std::ios::binary);
        ofs << "P6\n" << width << " " << height << "\n255\n";
        for (size_t row = 0; row < height; row++) {
            for (size_t col = 0; col < width; col++) {
                Color color = pixels[height - 1 - row][col];
                ofs << static_cast<unsigned char>(color.r * 255)
                    << static_cast<unsigned char>(color.g * 255)
                    << static_cast<unsigned char>(color.b * 255);
            }
        }
        ofs.close();
    }
};
inline std::ostream& operator<<(std::ostream& os, const FrameBuffer& frameBuffer) {
    os << std::fixed << std::setprecision(2)
        << "FrameBuffer("
         << "width:"        << frameBuffer.getWidth()     << ", "
         << "height:"       << frameBuffer.getHeight()    << ", "
         << "numPixels:"    << frameBuffer.getNumPixels() << ", "
         << "defaultColor:" << frameBuffer.getDefaultColor()
        << ")";
    return os;
}
