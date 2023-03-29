#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#define num_walls_lv1 4
#define pi 3.14159265

#define pistolfirerate 0.2f
#define pistoldamage 5
#define pistolspread 0
#define pistolbulletpershot 1

#define riflefirerate 0.1f
#define rifledamage 3
#define riflespread 0
#define riflebulletpershot 1

#define shotgunfirerate 2
#define shotgundamage 1
#define shotgunspread 1
#define shotgunbulletpershot 10

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
    float maxvelocity = 5000.0f;
    float damage;
};

void Update(float dt,state& curr_state);
void Draw(state& curr_state);
void draw_dash_line();
void AddDashLineVertexes();
void calculate_shoot_dir();
void select_guns();
void init_walls();
void Player_Movement(float dt);
void Shooting();
void Bullet_Movement_Collision(float dt);
void Dashing(state& curr_state, float dt);
void Player_Collision();


RectangleShape Player(Vector2f(50.f,50.f));
RectangleShape Gun(Vector2f(35.f, 10.f));
RenderWindow window(VideoMode(1920, 1080), "ZombieGame");
RectangleShape wall1(Vector2f(50.0, 10000.0));
RectangleShape wall2(Vector2f(50.0, 10000.0));
RectangleShape wall3(Vector2f(10000.0, 50.0));
RectangleShape wall4(Vector2f(10000.0, 50.0));

std::vector <FloatRect> Wall_Bounds;
Walls walls[num_walls_lv1];

float x, y,playerspeed = 500.0;
float maxcooldown = 10;
float timesincedash = 0;
float fire_rate_counter;


float current_fire_rate = pistolfirerate;
float current_damage = pistoldamage;
float current_spread = pistolspread;
float current_bullets_per_shot = pistolbulletpershot;

Vector2f current_player_pos;
Vector2f previous_player_pos;
Vector2f MousePos;
Vector2f center_of_player_pos;
Vector2f dir_vector;
Vector2f Norm_dir_vector;

VertexArray dasheline(Quads);
std::vector<bullet> bullets;

int main()
{
    init_walls();
    Gun.setFillColor(Color::Green);
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
        Draw(state);
    }

    return 0;
}
void Update(float dt,state &curr_state)
{
    fire_rate_counter += dt;
    current_player_pos = Player.getPosition();
    MousePos = Vector2f(Mouse::getPosition(window));
    Gun.setPosition(current_player_pos.x + 25.f, current_player_pos.y + 25.f);
    calculate_shoot_dir();
    select_guns();
    // movement
    Player_Movement(dt);
    // shooting
    Shooting();
    Bullet_Movement_Collision(dt);
    //dashing
    Dashing(curr_state, dt);
    previous_player_pos = current_player_pos;

    //check collision
    Player_Collision();
}
void Player_Collision()
{
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
void Dashing(state& curr_state,float dt)
{
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
}
void Bullet_Movement_Collision(float dt)
{
    for (int i = 0; i < bullets.size(); i++)
    {
        bullets[i].shape.move(bullets[i].currentvelocity * dt);
        for (int k = 0; k < Wall_Bounds.size(); k++)
        {
            if (bullets[i].shape.getGlobalBounds().intersects(Wall_Bounds[k]))
            {
                bullets.erase(bullets.begin() + i);
                break;
            }
        }
    }
}
void Shooting()
{
    if (Mouse::isButtonPressed(Mouse::Left) && fire_rate_counter >= current_fire_rate)
    {
        for (int i = 0; i < current_bullets_per_shot; i++)
        {
            bullet newbullet;
            newbullet.shape.setFillColor(Color::Magenta);
            newbullet.shape.setRadius(10.f);
            newbullet.shape.setPosition(Gun.getPosition());
            Vector2f Offset(rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX));
            newbullet.currentvelocity = newbullet.maxvelocity * (Norm_dir_vector + (Offset * 0.2f * current_spread));
            newbullet.damage = current_damage;
            bullets.push_back(newbullet);
        }
        fire_rate_counter = 0;
    }
}
void Player_Movement(float dt)
{
    x = 0;
    y = 0;
    if (Keyboard::isKeyPressed(Keyboard::A))
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
}
void init_walls()
{
    wall1.setFillColor(Color::Black);
    wall1.setPosition(0, 0);
    Wall_Bounds.push_back(wall1.getGlobalBounds());
    wall2.setFillColor(Color::Black);
    wall2.setPosition(10000, 0);
    Wall_Bounds.push_back(wall2.getGlobalBounds());
    wall3.setFillColor(Color::Black);
    wall3.setPosition(0, 0);
    Wall_Bounds.push_back(wall3.getGlobalBounds());
    wall4.setFillColor(Color::Black);
    wall4.setPosition(0, 10000);
    Wall_Bounds.push_back(wall4.getGlobalBounds());

}
void select_guns()
{
    if (Keyboard :: isKeyPressed(Keyboard::Num1))
    {
        current_fire_rate = pistolfirerate;
        current_damage = pistoldamage;
        current_bullets_per_shot = pistolbulletpershot;
        current_spread = pistolspread;

    }
    if (Keyboard::isKeyPressed(Keyboard::Num2))
    {
        current_fire_rate = riflefirerate;
        current_damage = rifledamage;
        current_bullets_per_shot = riflebulletpershot;
        current_spread = riflespread;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3))
    {
        current_fire_rate = shotgunfirerate;
        current_damage = shotgundamage;
        current_bullets_per_shot = shotgunbulletpershot;
        current_spread = shotgunspread;
    }
}
void calculate_shoot_dir()
{
    dir_vector = MousePos - Vector2f(window.getSize().x / 2 + 30, window.getSize().y / 2 + 30);
    Norm_dir_vector = dir_vector / (sqrt(dir_vector.x * dir_vector.x + dir_vector.y * dir_vector.y));

    float rotation = atan2( - 1 *Norm_dir_vector.y ,  - 1 *Norm_dir_vector.x) * 180 / pi + 180;
    Gun.setRotation(rotation);
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
void Draw(state& curr_state)
{
    window.clear(Color::White);
    window.draw(Player);
    window.draw(Gun);
    for (int i = 0; i < bullets.size(); i++)
    {
        window.draw(bullets[i].shape);
    }
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