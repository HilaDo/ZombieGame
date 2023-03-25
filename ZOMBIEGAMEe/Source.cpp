#include <SFML/Graphics.hpp>
#include <iostream>
using namespace sf;

enum state
{
    dashing, dash_cooldown, normal
};

void Update(float dt,state& curr_state);
void Draw();
void Dash(float dt);


RectangleShape Player(Vector2f(50.f,50.f));
RectangleShape Block(Vector2f(50.f,50.f));
RenderWindow window(VideoMode(800, 600), "ZombieGame");

float x, y,playerspeed = 500.0;
float maxcooldown = 10;
int keytime = 0;
float timesincedash = 0;
int main()
{
    state state = state::normal;
    Player.setFillColor(Color::Red);
    Block.setFillColor(Color::Blue);
    Player.setPosition(Vector2f(window.getSize().x / 2, window.getSize().y / 2));
    Block.setPosition(Vector2f(0,0));
    Clock clock;
    Event event;
    View view(Vector2f(0, 0), Vector2f(window.getSize().x, window.getSize().y));
    while (window.isOpen()) {
        float elapsed = clock.restart().asSeconds();
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed) {

                window.close();
            }
        }
        view.setCenter(Player.getPosition());
        window.setView(view);
        Update(elapsed,state);
        Draw();
    }

    return 0;
}
void Update(float dt,state &curr_state)
{

    x = 0;
    y = 0;
    if (Keyboard::isKeyPressed(Keyboard :: A))
    {
        x = -1;
    }
    if (Keyboard::isKeyPressed(Keyboard::D))
    {
        x = 1;
    }
    if (Keyboard::isKeyPressed(Keyboard::W))
    {
        y = -1;
    }
    if (Keyboard::isKeyPressed(Keyboard::S))
    {
        y = 1;
    }
    Player.move(x * dt * playerspeed, y * dt * playerspeed);
    if (Keyboard::isKeyPressed(Keyboard::LShift))
    {
        curr_state = state::dashing;
        playerspeed = 2000.0;
    }
    if (curr_state == state::dashing)
    {
        timesincedash += dt;
        if (timesincedash > 0.1)
        {
            curr_state = state::dash_cooldown;
            playerspeed = 500;
        }
    }
    if (curr_state == state::dash_cooldown)
    {
        timesincedash += dt;
        if (timesincedash > 3.0)
        {
            curr_state = state::normal;
            timesincedash = 0;
        }
    }
    FloatRect Player_Bounds = Player.getGlobalBounds();
    FloatRect Wall_Bounds = Block.getGlobalBounds();
    FloatRect intersection;
    if (Player_Bounds.intersects(Wall_Bounds))
    {

        Player_Bounds.intersects(Wall_Bounds,intersection);
        std::cout << intersection.height << std::endl;
        if (intersection.width < intersection.height)
        {
            if (Player_Bounds.left < Wall_Bounds.left)
            {
                Player.setPosition(Wall_Bounds.left - Player_Bounds.width, Player.getPosition().y);
            }
            else
            {
                Player.setPosition(Wall_Bounds.left + Player_Bounds.width, Player.getPosition().y);
            }
        }
        else
        {
            if (Player_Bounds.top < Wall_Bounds.top)
            {
                Player.setPosition(Player.getPosition().x, Wall_Bounds.top - Player_Bounds.height);
            }
            else
            {
                Player.setPosition(Player.getPosition().x, Wall_Bounds.top + Wall_Bounds.height);
            }
        }
    }
}
void Draw()
{
    window.clear(Color::White);
    window.draw(Player);
    window.draw(Block);
    window.display();
}