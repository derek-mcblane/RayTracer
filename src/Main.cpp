#include "Tracer.h"
#include "Scene.hpp"
#include "Timer.hpp"
#include <iostream>
#include <windows.h>


const std::string OUTPUT_FILE = "./scene";
const Vec2 IMAGE_SIZE{ 1500, 1500 };
static constexpr Color SHADOW_COLOR{ 0.05f, 0.05f, 0.05f };
static constexpr Color BACKGROUND_COLOR{ 0.25f, 0.25f, 0.25f };
static constexpr size_t MAX_NUM_REFLECTIONS = 5;

int main()
{
    ViewPlane viewPlane{ 10, 10, 10 };
    Timer timer{};
    Scene scene{};
    Tracer tracer{};
    RenderCam cam{ viewPlane };
    FrameBuffer frameBuffer(IMAGE_SIZE.x, IMAGE_SIZE.y, BACKGROUND_COLOR);
    tracer.setShadowColor(SHADOW_COLOR);
    tracer.setBackgroundColor(BACKGROUND_COLOR);
    tracer.setMaxNumReflections(MAX_NUM_REFLECTIONS);

    // configure lighting
    Light pointLight(Vec3(0, 0, 10), Presets::pureWhite);
    pointLight.setAmbientIntensity(0.50f);
    pointLight.setDiffuseIntensity(0.75f);
    pointLight.setSpecularIntensity(0.50f);
    scene.addLight(pointLight);

    // configure scene composition
    scene.addSceneObject(Triangle(Vec3(-10, 5, 5), Vec3(10, 5, 5), Vec3(0, 0, -25), Presets::flatYellow));
    scene.addSceneObject(Sphere(Vec3( 2, -2, -10), 2.00f, Presets::smoothBlue));
    scene.addSceneObject(Sphere(Vec3(-2,  5, -15), 1.25f, Presets::roughRed));
    scene.addSceneObject(Sphere(Vec3(-2, 10, -15), 0.50f, Presets::smoothBlue));
    scene.addSceneObject(Sphere(Vec3( 0,  5, -30), 1.00f, Presets::shinyBlue));
    scene.addSceneObject(Sphere(Vec3( 0,  5, -15), 1.00f, Presets::shinyBlue));

    // trace scene
    std::cout << "Tracing scene...";
    timer.start();
      tracer.trace(cam, scene, frameBuffer);
    timer.stop();
    std::cout << "finished in " << timer.elapsedTime() << "\n";
    
    // output to file
    std::cout << "Writing frameBuffer to image file: " << OUTPUT_FILE << ".ppm`" << "...";
    timer.start();
      frameBuffer.writeToFile(OUTPUT_FILE);
    timer.stop();
    std::cout << "finished in " << timer.elapsedTime() << "\n";

    // prompt user to 
    std::cout << "Press ENTER to end...";
    std::cin.get();
}
