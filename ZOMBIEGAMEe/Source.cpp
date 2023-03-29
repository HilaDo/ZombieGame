#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#define num_walls_lv1 4
using namespace sf;

enum state
{
    dashing, dash_cooldown, normal
};
struct Walls
{
    Vector2f scale;
    Color color;
};
struct bullet
{
    CircleShape shape;
    Vector2f currentvelocity;
    float maxvelocity;
};

void Update(float dt,state& curr_state);
void Draw(RectangleShape wall1, RectangleShape wall2, RectangleShape wall3, RectangleShape wall4, state& curr_state);
void draw_dash_line();
void AddDashLineVertexes();
void calculate_shoot_dir();


RectangleShape Player(Vector2f(50.f,50.f));
RenderWindow window(VideoMode(800, 600), "ZombieGame");

std::vector <FloatRect> Wall_Bounds;
Walls walls[num_walls_lv1];

float x, y,playerspeed = 500.0;
float maxcooldown = 10;
float timesincedash = 0;

Vector2f current_player_pos;
Vector2f previous_player_pos;
Vector2f MousePos;
Vector2f center_of_player_pos;
Vector2f dir_vector;
Vector2f Norm_dir_vector;

VertexArray dasheline(Quads);


int main()
{
    bullet b1;
    std::vector<bullet> bullets;
    bullets.push_back(b1);

    RectangleShape wall1(Vector2f(50.0,10000.0));
    wall1.setFillColor(walls[0].color);
    wall1.setPosition(0, 0);
    Wall_Bounds.push_back(wall1.getGlobalBounds());
    RectangleShape wall2(Vector2f(50.0, 10000.0));
    wall2.setFillColor(walls[1].color);
    wall2.setPosition(10000, 0);
    Wall_Bounds.push_back(wall2.getGlobalBounds());
    RectangleShape wall3(Vector2f(10000.0, 50.0));
    wall3.setFillColor(walls[2].color);
    wall3.setPosition(0, 0);
    Wall_Bounds.push_back(wall3.getGlobalBounds());
    RectangleShape wall4(Vector2f(10000.0, 50.0));
    wall4.setFillColor(walls[3].color);
    wall4.setPosition(0,10000);
    Wall_Bounds.push_back(wall4.getGlobalBounds());

    state state = state::normal;
    Player.setFillColor(Color::Red);
    Player.setPosition(Vector2f(50,0));
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
        Draw(wall1,wall2,wall3,wall4,state);
    }

    return 0;
}
void Update(float dt,state &curr_state)
{
    x = 0;
    y = 0;
    current_player_pos = Player.getPosition();
    MousePos = Vector2f( Mouse::getPosition(window));
    // movement
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
    if (Mouse::isButtonPressed(Mouse::Left))
    {
        std::cout << MousePos.x << " " << MousePos.y << std::endl;
    }
    Player.move(x * dt * playerspeed, y * dt * playerspeed);
    //dashing
    if (Keyboard::isKeyPressed(Keyboard::LShift))
    {
        curr_state = state::dashing;
        playerspeed = 4000.0f;
    }
    if (curr_state == state::dashing)
    {
        timesincedash += dt;

        if (timesincedash > 0.05f)
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
    previous_player_pos = current_player_pos;

    //check collision
    for (int i = 0; i < Wall_Bounds.size(); i++)
    {
        FloatRect Player_Bounds = Player.getGlobalBounds();
        FloatRect intersection;
        FloatRect Wall_bound = Wall_Bounds[i];
        if (Player_Bounds.intersects(Wall_bound))
        {

            Player_Bounds.intersects(Wall_bound, intersection);
            std::cout << intersection.height << std::endl;
            if (intersection.width < intersection.height)
            {
                if (Player_Bounds.left < Wall_bound.left)
                {
                    Player.setPosition(Wall_bound.left - Player_Bounds.width, Player.getPosition().y);               
                }
                else
                {
                    Player.setPosition(Wall_bound.left + Player_Bounds.width, Player.getPosition().y);
                }
            }
            else
            {
                if (Player_Bounds.top < Wall_bound.top)
                {
                    Player.setPosition(Player.getPosition().x, Wall_bound.top - Player_Bounds.height);
                }
                else
                {
                    Player.setPosition(Player.getPosition().x, Wall_bound.top + Wall_bound.height);
                }
            }
        }
    }
}
void calculate_shoot_dir()
{
    dir_vector = MousePos - center_of_player_pos;
    Norm_dir_vector = dir_vector / (sqrt(dir_vector.x * dir_vector.x + dir_vector.y * dir_vector.y));
}
void draw_dash_line()
{
    AddDashLineVertexes();
    for (int i = 0; i < dasheline.getVertexCount(); i++)
    {
        dasheline[i].color = Color(107, 107, 107);
    }
    window.draw(dasheline);
}
void AddDashLineVertexes()
{
    FloatRect playerbounds = Player.getLocalBounds();
    dasheline.append(Vector2f(current_player_pos.x + 10.0f, current_player_pos.y + 10.0f));
    dasheline.append(Vector2f(current_player_pos.x + 10.0f, current_player_pos.y + playerbounds.height - 10.0f));
    dasheline.append(Vector2f(current_player_pos.x + playerbounds.width - 10.0f, current_player_pos.y + playerbounds.height - 10.0f));
    dasheline.append(Vector2f(current_player_pos.x + playerbounds.width - 10.0f, current_player_pos.y + 10.0f));

    dasheline.append(Vector2f(previous_player_pos.x + 10.0f, previous_player_pos.y + 10.0f));
    dasheline.append(Vector2f(previous_player_pos.x + 10.0f, previous_player_pos.y + playerbounds.height - 10.0f));
    dasheline.append(Vector2f(previous_player_pos.x + playerbounds.width - 10.0f, previous_player_pos.y + playerbounds.height - 10.0f));
    dasheline.append(Vector2f(previous_player_pos.x + playerbounds.width - 10.0f, previous_player_pos.y + 10.0f));
}
void Draw(RectangleShape wall1, RectangleShape wall2, RectangleShape wall3, RectangleShape wall4, state& curr_state)
{
    window.clear(Color::White);
    window.draw(Player);
    window.draw(wall1);
    window.draw(wall2);
    window.draw(wall3);
    window.draw(wall4);
    if (curr_state == state :: dashing)
    {
        draw_dash_line();
    }
    else
    {

        dasheline.clear();
    }
    window.display();
}