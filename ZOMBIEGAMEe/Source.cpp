#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

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

#define sniperfirerate 3
#define sniperdamage 10
#define sniperspread 0
#define sniperbulletpershot 1

using namespace sf;

enum state
{
    idle,walk,dashing,hit,death
};
struct bullet
{
    CircleShape shape;
    Vector2f currentvelocity;
    float maxvelocity = 5000.0f;
    float damage;
};

void Update(float dt);
void Draw();
void draw_dash_line();
void AddDashLineVertexes();
void calculate_shoot_dir();
void select_guns();
void init_walls();
void Player_Movement(float dt);
void Shooting();
void Bullet_Movement_Collision(float dt);
void Dashing(float dt);
void Player_Collision();
void UpdateAnimationCounter(float dt,int maxanimationcounter);
void InitTextures();
void switch_player_states(state& curr_state,float dt);

bool isdashing = false;
bool dashready = true;

Sprite Player;
RectangleShape Gun(Vector2f(35.f, 10.f));
RenderWindow window(VideoMode(1920, 1080), "ZombieGame");
RectangleShape wall1(Vector2f(50.0, 10000.0));
RectangleShape wall2(Vector2f(50.0, 10000.0));
RectangleShape wall3(Vector2f(10000.0, 50.0));
RectangleShape wall4(Vector2f(10000.0, 50.0));

Texture WalkAnimation[8];
Texture IdleAnimation[6];
Texture HitAnimation[3];
Texture RollAnimation[5];
Texture DeathAnimation[10];

std::vector <FloatRect> Wall_Bounds;

float x, y,playerspeed = 500.0;
float maxcooldown = 10;
float timesincedash = 0;
float fire_rate_counter;
float AnimationCounter = 0;
float AnimationSwitchTime = 0.1f;

int ImageCounter = 0;

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
    InitTextures();
    Gun.setFillColor(Color::Green);
    Player.setPosition(Vector2f(50,0));
    Player.setScale(1, 1);
    state current_state_of_player = state::idle;
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
        Update(elapsed);
        switch_player_states(current_state_of_player,elapsed);
        Draw();
    }

    return 0;
}
void Update(float dt)
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
    Dashing(dt);
    previous_player_pos = current_player_pos;

    //check collision
    Player_Collision();
}
void switch_player_states(state& curr_state,float dt)
{
    if (x == 1 || x == -1 || y == 1 || y == -1)
    {
        curr_state = state::walk;
    }
    else
    {
        curr_state = state::idle;
    }
    if (isdashing)
    {
        curr_state = state::dashing;
    }
    switch (curr_state)
    {
    case state::idle: Player.setTexture(IdleAnimation[ImageCounter]); UpdateAnimationCounter(dt, sizeof(IdleAnimation) / sizeof(int) / 10); break;
    case state::walk: Player.setTexture(WalkAnimation[ImageCounter]);  UpdateAnimationCounter(dt, sizeof(WalkAnimation) / sizeof(int) / 10); break;
    case state::dashing: Player.setTexture(RollAnimation[ImageCounter]);  UpdateAnimationCounter(dt, sizeof(RollAnimation) / sizeof(int) / 10); break;
    case state::hit:break;
    case state::death:break;
    }
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
void InitTextures()
{
    for (int i = 0; i < sizeof(WalkAnimation) / sizeof(int)/10; i++)
    {
        WalkAnimation[i].loadFromFile("E:/Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 2/with hands/Walk - Copy (2)/walk_" +std::to_string(i)+ ".png");
    }
    for (int i = 0; i < sizeof(IdleAnimation) / sizeof(int) / 10; i++)
    {
        IdleAnimation[i].loadFromFile("E:/Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 2/with hands/Idle/idle_" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < sizeof(RollAnimation) / sizeof(int) / 10; i++)
    {
        RollAnimation[i].loadFromFile("E:/Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 2/with hands/Roll/roll_" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < sizeof(HitAnimation) / sizeof(int) / 10; i++)
    {
        HitAnimation[i].loadFromFile("E:/Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 2/with hands/Hit/hit_" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < sizeof(DeathAnimation) / sizeof(int) / 10; i++)
    {
        DeathAnimation[i].loadFromFile("E:/Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 2/with hands/Death/death_" + std::to_string(i) + ".png");
    }
}
void UpdateAnimationCounter(float dt,int maximagecounter)
{
    AnimationCounter += dt;
    if (AnimationCounter >= AnimationSwitchTime)
    {
        AnimationCounter -= AnimationSwitchTime;
        ImageCounter++;
        if (ImageCounter >= maximagecounter)
        {
            ImageCounter = 0;
        }
    }
}
void Dashing(float dt)
{
    if (Keyboard::isKeyPressed(Keyboard::LShift) && dashready)
    {
        isdashing = true;
        dashready = false;
        playerspeed = 4000.0f;
    }
    if (isdashing && !dashready)
    {
        timesincedash += dt;

        if (timesincedash > 0.05f)
        {
            isdashing = false;
            playerspeed = 500;
        }
    }
    if (!isdashing && !dashready)
    {
        timesincedash += dt;
        if (timesincedash > 3.0)
        {
            dashready = true;
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
    if (Keyboard::isKeyPressed(Keyboard::Num4))
    {
        current_fire_rate = sniperfirerate;
        current_damage = sniperdamage;
        current_bullets_per_shot = sniperbulletpershot;
        current_spread = sniperspread;
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
void Draw()
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
    if (isdashing)
    {
        draw_dash_line();
    }
    else
    {

        dasheline.clear();
    }
    window.display();
}