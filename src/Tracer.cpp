#include "Tracer.h"
#include "Math.hpp"
#include "Color.hpp"
#include "Camera.h"
#include "Lights.h"
#include "Objects.h"
#include "Scene.h"
#include "FrameBuffer.h"
#include <omp.h>


Tracer::Tracer() :
    shadowColor      (DEFAULT_SHADOW_COLOR),
    backgroundColor  (DEFAULT_BACKGROUND_COLOR),
    shadowBias       (DEFAULT_SHADOW_BIAS),
    reflectionBias   (DEFAULT_REFLECTION_BIAS),
    maxNumReflections(DEFAULT_MAX_NUM_REFLECTIONS)
{}

void Tracer::setShadowColor(const Color& shadowColor) {
    this->shadowColor = shadowColor;
}
void Tracer::setBackgroundColor(const Color& backgroundColor) {
    this->backgroundColor = backgroundColor;
}
void Tracer::setShadowBias(float shadowBias) {
    this->shadowBias = shadowBias;
}
void Tracer::setReflectionBias(float reflectionBias) {
    this->reflectionBias = reflectionBias;
}
void Tracer::setMaxNumReflections(size_t maxNumReflections) {
    this->maxNumReflections = maxNumReflections;
}

// for each pixel in buffer shoot ray from camera position to its projected point on the image plane,
// trace it through the scene and write computed color to buffer (dynamically scheduled in parallel using openMp)
void Tracer::trace(const Camera& renderCam, const Scene& scene, FrameBuffer& frameBuffer) {
    const int width        = static_cast<int>(frameBuffer.width());
    const int height       = static_cast<int>(frameBuffer.height());
    const float invWidth   = 1.00f / width;
    const float invHeight  = 1.00f / height;
    const float nearZ      = renderCam.getNearClip();
    const Vec3 eyePosition = renderCam.getPosition();
    #pragma omp for schedule(dynamic)
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const Vec3 pixelWorldPosition = renderCam.viewportToWorld(Vec3(
                (col + 0.50f) * invWidth,
                (row + 0.50f) * invHeight,
                nearZ
            ));
            Ray primaryRay{ eyePosition, Math::direction(eyePosition, pixelWorldPosition) };
            Color pixelColor = traceRay(renderCam, scene, primaryRay, 0);
            frameBuffer.setPixel(row, col, pixelColor);
        }
    }
}

// todo: account for near and far clip culling (probably need to determine z dist pf object to camera and clamp on that)
Color Tracer::traceRay(const Camera& renderCam, const Scene& scene, const Ray& ray, size_t iteration=0) const {
    if (iteration >= maxNumReflections) {
        return Color{ 0, 0, 0 };
    }

    Intersection intersection{};
    if (!findNearestIntersection(scene, ray, intersection)) {
        return backgroundColor;
    }

    Color reflectedColor{ 0, 0, 0 };
    if (intersection.object->material().reflectivity() > 0.00f) {
        reflectedColor = traceRay(renderCam, scene, reflectRay(ray, intersection), iteration + 1);
    }
    
    Color nonReflectedColor = intersection.object->material().ambientColor();
    for (size_t index = 0; index < scene.getNumLights(); index++) {
       const ILight& light = scene.getLight(index);
       if (!isInShadow(intersection, light, scene)) {
           Color diffuse = computeDiffuseColor(intersection, light);
           Color specular = computeSpecularColor(intersection, light, renderCam);
           Color lightIntensityAtPoint = light.computeIntensityAtPoint(intersection.point);
           nonReflectedColor = nonReflectedColor + lightIntensityAtPoint * (diffuse + specular);
       } else {
           nonReflectedColor = nonReflectedColor + shadowColor;
       }
    }
    
    // blend intrinsic and reflected color using our light and intersected object
    return (intersection.object->material().intrinsity()   * nonReflectedColor) + 
           (intersection.object->material().reflectivity() * reflectedColor);
}

// reflect our ray using a slight direction offset to avoid infinite reflections
Ray Tracer::reflectRay(const Ray& ray, const Intersection& intersection) const {
    Vec3 reflectedDirection = Math::normalize(
        (-1 * ray.direction) + (2 * Math::dot(ray.direction, intersection.normal) * intersection.normal)
    );
    return Ray(intersection.point + (reflectionBias * reflectedDirection), reflectedDirection);
}
bool Tracer::findNearestIntersection(const Scene& scene, const Ray& ray, Intersection& result) const {
    float tClosest = Math::INF;
    Intersection closestIntersection{};
    for (size_t index = 0; index < scene.getNumObjects(); index++) {
        Intersection intersection;
        if (scene.getObject(index).intersect(ray, intersection) && intersection.t < tClosest) {
            tClosest = intersection.t;
            closestIntersection = intersection;
        }
    }

    if (closestIntersection.object == nullptr) {
        return false;
    } else {
        result = closestIntersection;
        return true;
    }
}
// check if there exists another object blocking light from reaching our hit-point
bool Tracer::isInShadow(const Intersection& intersection, const ILight& light, const Scene& scene) const {
    Ray shadowRay{ intersection.point + (shadowBias * intersection.normal), Math::direction(intersection.point, light.getPosition()) };
    float distanceToLight = Math::distance(shadowRay.origin, light.getPosition());
    for (size_t index = 0; index < scene.getNumObjects(); index++) {
        Intersection occlusion;
        if (scene.getObject(index).intersect(shadowRay, occlusion) &&
                occlusion.object != intersection.object &&
                Math::distance(occlusion.point, light.getPosition()) < distanceToLight) {
            return true;
        }
    }
    return false;
}

Color Tracer::computeDiffuseColor(const Intersection& intersection, const ILight& light) const {
    Vec3 directionToLight = Math::direction(intersection.point, light.getPosition());

    const Material surfaceMaterial = intersection.object->material();
    float strengthAtLightAngle = Math::max(0.00f, Math::dot(intersection.normal, directionToLight));
    return strengthAtLightAngle * surfaceMaterial.diffuseColor();
}

Color Tracer::computeSpecularColor(const Intersection& intersection, const ILight& light, const Camera& renderCam) const {
    Vec3 directionToCam = Math::direction(intersection.point, renderCam.getPosition());
    Vec3 halfwayVec = Math::normalize(directionToCam + light.getPosition());

    const Material surfaceMaterial = intersection.object->material();
    float strengthAtCamAngle = Math::max(0.00f, Math::dot(intersection.normal, halfwayVec));
    return Math::pow(strengthAtCamAngle, surfaceMaterial.shininess()) * surfaceMaterial.specularColor();
}

std::ostream& operator<<(std::ostream& os, const Tracer& tracer) {
    os << "Tracer("
         << "shadow-color:("       << tracer.getShadowColor()       << "),"
         << "background-color:("   << tracer.getBackgroundColor()   << "),"
         << "shadow-bias:"         << tracer.getShadowBias()        << ","
         << "reflection-bias:"     << tracer.getReflectionBias()    << ","
         << "max-num-reflections:" << tracer.getMaxNumReflections()
       << ")";
    return os;
}
