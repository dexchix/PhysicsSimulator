#include <SFML/Graphics.hpp>
#include <iostream>

// Вектор
struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
};

// Частица
class Particle {
public:
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    Vec2 forceAccum;
    float mass;
    float invMass;
    float radius;
    float restitution;

    Particle(Vec2 pos = Vec2(), float mass = 1.0f, float radius = 10.0f, float restitution = 0.8f)
        : position(pos), mass(mass), radius(radius), restitution(restitution) {
        invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
        velocity = Vec2(0, 0);
        acceleration = Vec2(0, 0);
        forceAccum = Vec2(0, 0);
    }

    void ApplyForce(const Vec2& force) {
        forceAccum += force;
    }

    void Integrate(float dt) {
        if (invMass == 0.0f) return;

        acceleration = forceAccum * invMass;
        velocity += acceleration * dt;
        position += velocity * dt;
        forceAccum = Vec2(0, 0);
    }
};

// Мир
class PhysicsWorld {
public:
    std::vector<std::unique_ptr<Particle>> particles;
    Vec2 gravity;

    PhysicsWorld(Vec2 gravity = Vec2(0.0f, 500.0f)) : gravity(gravity) {}

    Particle* AddParticle(const Vec2& pos, float mass, float radius, float restitution) {
        particles.push_back(std::make_unique<Particle>(pos, mass, radius, restitution));
        return particles.back().get();
    }

    void Update(float dt) {
        std::cout << "=== Physics Update (dt: " << dt << ") ===\n";

        for (auto& p : particles) {
            if (p->invMass > 0.0f) {
                Vec2 gravityForce = Vec2(0, p->mass * gravity.y);
                std::cout << "Applying force: (" << gravityForce.x << ", " << gravityForce.y << ")\n";
                p->ApplyForce(gravityForce);
                std::cout << "Force accum after: (" << p->forceAccum.x << ", " << p->forceAccum.y << ")\n";
            }
        }

        for (auto& p : particles) {
            std::cout << "Before integrate - Pos: (" << p->position.x << ", " << p->position.y << ")\n";
            p->Integrate(dt);
            std::cout << "After integrate - Pos: (" << p->position.x << ", " << p->position.y << ")\n";
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Physics Test");
    window.setFramerateLimit(60);

    PhysicsWorld world(Vec2(0, 500.0f));

    Particle* ball = world.AddParticle(Vec2(400.0f, 50.0f), 1.0f, 20.0f, 0.8f);
    Particle* ground = world.AddParticle(Vec2(400.0f, 550.0f), 0.0f, 100.0f, 0.2f);

    sf::CircleShape ballShape(ball->radius);
    ballShape.setFillColor(sf::Color::Red);
    ballShape.setOrigin(ball->radius, ball->radius);

    sf::CircleShape groundShape(ground->radius);
    groundShape.setFillColor(sf::Color::Green);
    groundShape.setOrigin(ground->radius, ground->radius);
    groundShape.setScale(4.0f, 0.2f);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        world.Update(dt);

        ballShape.setPosition(ball->position.x, ball->position.y);
        groundShape.setPosition(ground->position.x, ground->position.y);

        window.clear(sf::Color::Black);
        window.draw(groundShape);
        window.draw(ballShape);
        window.display();

        std::cout << "Ball position: (" << ball->position.x << ", " << ball->position.y << ")\n";

        // Сброс шара если улетел
        if (ball->position.y > 800) {
            ball->position = Vec2(400.0f, 50.0f);
            ball->velocity = Vec2(0, 0);
        }
    }

    return 0;
}
