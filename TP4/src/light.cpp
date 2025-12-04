#include "light.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Light::Light() {
    sunLight_.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -0.5f));
    sunLight_.color = glm::vec3(1.0f, 1.0f, 0.95f);
    sunLight_.intensity = 0.8f;
    sunLight_.castShadows = true;
    sunLight_.shadowStrength = 0.7f;
    sunLight_.shadowSoftness = 1.5f;
    sunLight_.isDirectional = true;
}

Light::~Light() {
}

void Light::initialize() {
    std::cout << "Light system initialized (simple projected shadows)" << std::endl;
    std::cout << "Light direction: " << sunLight_.direction.x << ", "
        << sunLight_.direction.y << ", " << sunLight_.direction.z << std::endl;
    std::cout << "Shadow strength: " << sunLight_.shadowStrength << std::endl;
}