#include <SFML/Graphics.hpp>
#include <optional>
#include <algorithm>
#include <string>

// --- Стан симуляції ---
bool isPaused = false;

// --- Фізичні параметри ---
float carMass = 1200.f;          // Маса авто (кг)
float tractionForce = 3000.f;    // Поточна налаштована сила тяги (Н)
float brakingForce = 5000.f;     // Поточна налаштована сила гальмування (Н)
float frictionCoeff = 300.f;     // Коефіцієнт опору кочення/тертя

// --- Змінні стану автомобіля ---
float velocity = 0.f;            // Швидкість (м/с)
float distanceTraveled = 0.f;    // Пройдений шлях (м)
float positionX = 50.f;          // Координата X для відмальовки (пікселі)

void resetSimulation() {
    velocity = 0.f;
    distanceTraveled = 0.f;
    positionX = 50.f;
}

void handleInput(sf::RenderWindow& window, const std::optional<sf::Event>& event) {
    if (event->is<sf::Event::Closed>()) {
        window.close();
    }

    if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
        // Управління станом моделювання
        if (key->code == sf::Keyboard::Key::P) isPaused = !isPaused;
        if (key->code == sf::Keyboard::Key::R) resetSimulation();

        // Зміна параметрів сили (Умови варіанта 1)
        if (key->code == sf::Keyboard::Key::Up) tractionForce += 200.f;
        if (key->code == sf::Keyboard::Key::Down) tractionForce -= 200.f;
        if (key->code == sf::Keyboard::Key::Right) brakingForce += 200.f;
        if (key->code == sf::Keyboard::Key::Left) brakingForce -= 200.f;

        // Запобіжник від від'ємних значень сил
        tractionForce = std::max(0.f, tractionForce);
        brakingForce = std::max(0.f, brakingForce);
    }
}

void updatePhysics(float dt) {
    if (isPaused) return;

    float currentForce = 0.f;
    bool isGasPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    bool isBrakePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::B);

    // Логіка застосування сил
    if (isGasPressed) {
        currentForce += tractionForce;
    }

    // Сила опору (діє проти руху, коли машина їде)
    if (velocity > 0.f) {
        currentForce -= frictionCoeff;
    }

    // Гальмування (діє лише якщо машина рухається вперед)
    if (isBrakePressed && velocity > 0.f) {
        currentForce -= brakingForce;
    }

    // 2-й закон Ньютона (a = F / m)
    float acceleration = currentForce / carMass;

    // Чисельне інтегрування швидкості (Метод Ейлера)
    velocity += acceleration * dt;
    if (velocity < 0.f) velocity = 0.f; // Машина не їде назад від гальмування

    // Чисельне інтегрування шляху
    float deltaDistance = velocity * dt;
    distanceTraveled += deltaDistance;

    // Відображення на екрані (1 метр = 10 пікселів для наочності)
    positionX += deltaDistance * 10.f;

    // Зациклення руху машини на екрані, щоб вона не виїжджала назавжди
    if (positionX > 800.f) {
        positionX = -60.f;
    }
}

void drawScene(sf::RenderWindow& window, sf::Font& font, sf::RectangleShape& car, sf::RectangleShape& road) {
    window.clear(sf::Color(30, 30, 30));

    // Оновлення позиції авто
    car.setPosition({ positionX, 320.f });

    // Малювання об'єктів
    window.draw(road);
    window.draw(car);

    // Формування тексту HUD
    std::string hud = "Controls:\n";
    hud += "[Space] - Gas | [B] - Brake\n";
    hud += "[Up/Down] - Change Traction Force\n";
    hud += "[Left/Right] - Change Braking Force\n";
    hud += "[P] - Pause/Unpause | [R] - Reset\n\n";
    hud += "Status: " + std::string(isPaused ? "PAUSED" : "RUNNING") + "\n";
    hud += "Traction Force: " + std::to_string((int)tractionForce) + " N\n";
    hud += "Braking Force: " + std::to_string((int)brakingForce) + " N\n\n";
    hud += "Speed: " + std::to_string((int)velocity) + " m/s\n";
    hud += "Distance: " + std::to_string((int)distanceTraveled) + " m";

    sf::Text infoText(font, hud, 18);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition({ 20.f, 20.f });
    window.draw(infoText);

    window.display();
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 400 }), "Variant 1 - Car Simulation");
    sf::Clock clock;

    sf::Font font;
    if (!font.openFromFile("../resources/fonts/arial.ttf")) {
        return 1;
    }

    sf::RectangleShape car({ 60.f, 30.f });
    car.setFillColor(sf::Color::Red);

    sf::RectangleShape road({ 800.f, 100.f });
    road.setFillColor(sf::Color(100, 100, 100));
    road.setPosition({ 0.f, 350.f });

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            handleInput(window, event);
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        updatePhysics(dt);
        drawScene(window, font, car, road);
    }
    return 0;
}