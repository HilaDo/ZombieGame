#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>


//to be displayed on ui
int Player_Health;
int Score;
int Wave_Number;
int Money;


#define num_walls_lv1 4
#define pi 3.14159265

#define pistolfirerate 0.2f
#define pistoldamage 5
#define pistolspread 0
#define pistolbulletpershot 1
#define pistolclipsize 9;
#define pistolreloadtime 1;
int pistolammostock = 240;
int pistolbulletsloaded = pistolclipsize;

#define riflefirerate 0.1f
#define rifledamage 3
#define riflespread 0
#define riflebulletpershot 1
#define rifleclipsize 30;
#define riflereloadtime 1;
int rifleammostock = 90;
int riflebulletsloaded = rifleclipsize;

#define shotgunfirerate 2
#define shotgundamage 1
#define shotgunspread 1
#define shotgunbulletpershot 10
#define shotgunclipsize 8
#define shotgunreloadtime 1;
int shotgunammostock = 24;
int shotgunbulletsloaded = shotgunclipsize;

using namespace sf;

enum state
{
    idle, walk, dashing, hit, death
};
enum Gun_State
{
    Pistol, Smg, Shotgun
};
Gun_State Curr_Gun_state = Gun_State::Pistol;
struct bullet
{
    CircleShape shape;
    Vector2f currentvelocity;
    float maxvelocity = 5000.0f;
    float damage;
};

void Update(float dt, state& curr_state); // the main update function where all game logic updates every frame
void Draw(); // the main drawing function where everything gets drawn on screen
void draw_dash_line(); // function to draw the dash effect behind the player
void AddDashLineVertexes(); // function that calculates the verticies of the dash effect
void calculate_shoot_dir(); // function that calculates the normal direction between the player and the current mouse position
void select_guns(); // function that switches between guns
void init_walls(); // function to initilize walls at the start of the game
void Player_Movement(float dt); // function responsible for the player movement
void Shooting(float dt); //function that handles shooting and spawning new bullets
void Bullet_Movement_Collision(float dt); //function that handles bullet collision
void Dashing(float dt); //function that calculates dash direction and does the dashing
void Player_Collision(float dt); // function that calculates and manages player collision with anything specified
void GetTextures(); //function that gets the used textures by the game at the start of the program
void UpdateAnimationCounter(float dt, int maximagecounter); //function that handles the changing of the animation frame with the specified number of frames as argument
void Switch_States(state& curr_state, float dt); // function that manages the current state of the player, i.e: player state right now is dashing, player state right now is dying, etc
void Switch_Current_Gun_Attributes(); //function that manages the current held gun attributes, i.e: the current gun fire rate is 1, current gun spread is 0,etc


//all sprites in the game
Sprite Player;
RectangleShape Gun(Vector2f(1.f,1.f));
Sprite Pistol_S;
Sprite SMG_S;
Sprite ShotGun_S;
RectangleShape DashOrigin(Vector2f(50.0f, 50.0f));
RenderWindow window(VideoMode(800, 600), "ZombieGame");
RectangleShape wall1(Vector2f(50.0, 10000.0));
RectangleShape wall2(Vector2f(50.0, 10000.0));
RectangleShape wall3(Vector2f(10000.0, 50.0));
RectangleShape wall4(Vector2f(10000.0, 50.0));

//dash booleans
bool isdashing = false;
bool dashready = true;
//reload booleans
bool isreloading = false;

//wall bounds vector to detect collision
std::vector <FloatRect> Wall_Bounds;

//animation textures
Texture WalkAnimation[8];
Texture IdleAnimation[6];
//gun textures
Texture Pistol_T;
Texture SMG_T;
Texture ShotGun_T;

//reload counter and current time that takes the reload to finish
float current_reload_time;
float reload_time_counter = 0;
//dash counter
float timesincedash = 0;
//animation counter and time between each texture takes
float AnimationCounter = 0;
float AnimationSwitchTime = 0.2f;
int ImageCounter = 0;
//firerate counter
float fire_rate_counter;
//player speed and direction(x,y)
float x, y, playerspeed = 500.0;

//current gun attributes
int* current_ammo = &pistolbulletsloaded; // ui
int* current_ammo_stock = &pistolammostock; // ui
int current_clip_size;
float current_fire_rate;
float current_damage;
float current_spread;
float current_bullets_per_shot;
//player positions
Vector2f current_player_pos;
Vector2f previous_player_pos;
Vector2f center_of_player_pos;
//normal direction variables
Vector2f MousePos;
Vector2f dir_vector;
Vector2f Norm_dir_vector;
//level array
int level1[16][16] =
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

VertexArray dasheline(Quads);

std::vector<bullet> bullets;

int main()
{
    init_walls();
    GetTextures(); 
    state state = state::dashing;
    Player.setTexture(WalkAnimation[0]);
    Player.setOrigin(Vector2f(Player.getGlobalBounds().width /2 , Player.getGlobalBounds().height / 2));
    Player.setPosition(Vector2f(100, 100));
    Player.setScale(0.2f, 0.2f);
    Gun.setOrigin(Player.getOrigin());
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
        Update(elapsed, state);
        Switch_States(state, elapsed);
        Draw();
    }

    return 0;
}
void Update(float dt, state& curr_state)
{
    fire_rate_counter += dt;
    current_player_pos = DashOrigin.getPosition();
    Vector2i pixelpos = Mouse::getPosition(window);
    MousePos = window.mapPixelToCoords(pixelpos);
    Switch_Current_Gun_Attributes();
    calculate_shoot_dir();
    select_guns();
    // movement
    Player_Movement(dt);
    // shooting
    Shooting(dt);
    Bullet_Movement_Collision(dt);
    //dashing
    Dashing(dt);
    previous_player_pos = current_player_pos;

    //check collision
    Player_Collision(dt);
}
void Switch_States(state& curr_state,float dt)
{
    if (x == 1 || x == -1 || y == 1 || y == -1)
    {
        curr_state = state::walk;
    }
    else
    {
        curr_state = state::idle;
    }
    switch (curr_state)
    {
    case state::walk: UpdateAnimationCounter(dt, 8); Player.setTexture(WalkAnimation[ImageCounter]);break;
    case state::idle: UpdateAnimationCounter(dt, 4); Player.setTexture(IdleAnimation[ImageCounter]); break;
    }
}
void GetTextures()
{
    for (int i = 0; i < 8; i++)
    {
        WalkAnimation[i].loadFromFile("Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 3/with hands/Walk/walk_"+ std::to_string(i)+".png");
    }
    for (int i = 0; i < 5; i++)
    {
        IdleAnimation[i].loadFromFile("Free 2D Animated Vector Game Character Sprites/Full body animated characters/Char 3/with hands/Idle/idle_" + std::to_string(i) + ".png");
    }  
    ShotGun_T.loadFromFile("Free 2D Animated Vector Game Character Sprites/PNG higher resolution (@2x)/shotgun.png"); 
    ShotGun_S.setTexture(ShotGun_T);
    SMG_T.loadFromFile("Free 2D Animated Vector Game Character Sprites/PNG higher resolution (@2x)/assaultrifle.png");
    SMG_S.setTexture(SMG_T);
    Pistol_T.loadFromFile("Free 2D Animated Vector Game Character Sprites/PNG higher resolution (@2x)/pistol.png");
    Pistol_S.setTexture(Pistol_T);    
}
void UpdateAnimationCounter(float dt,int maximagecounter)
{
    AnimationCounter += dt;
    if (AnimationCounter >= AnimationSwitchTime)
    {
        AnimationCounter = 0;
        ImageCounter++;
        if (ImageCounter >= maximagecounter)
        {
            ImageCounter = 0;
        }
    }
}
void Player_Collision(float dt)
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
                    Player.move(-playerspeed * dt, 0);
                }
                else
                {
                    Player.move(playerspeed * dt, 0);
                }
            }
            else
            {
                if (Player_Bounds.top < Wall_bound.top)
                {
                    Player.move(0, -playerspeed * dt);
                }
                else
                {
                    Player.move(0, playerspeed * dt);
                }
            }
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
void Switch_Current_Gun_Attributes()
{
    switch (Curr_Gun_state)
    {
    case Pistol:
        current_fire_rate = pistolfirerate;
        current_damage = pistoldamage;
        current_bullets_per_shot = pistolbulletpershot;
        current_spread = pistolspread;
        current_ammo = &pistolbulletsloaded;
        current_ammo_stock = &pistolammostock;
        current_clip_size = pistolclipsize;
        current_reload_time = pistolreloadtime;
        break;
    case Smg:
        current_fire_rate = riflefirerate;
        current_damage = rifledamage;
        current_bullets_per_shot = riflebulletpershot;
        current_spread = riflespread;
        current_ammo = &riflebulletsloaded;
        current_ammo_stock = &rifleammostock;
        current_clip_size = rifleclipsize;
        current_reload_time = riflereloadtime;
        break;
    case Shotgun:
        current_fire_rate = shotgunfirerate;
        current_damage = shotgundamage;
        current_bullets_per_shot = shotgunbulletpershot;
        current_spread = shotgunspread;
        current_ammo = &shotgunbulletsloaded;
        current_ammo_stock = &shotgunammostock;
        current_clip_size = shotgunclipsize;
        current_reload_time = shotgunreloadtime;
        break;
    }
}
void Shooting(float dt)
{
    if (Mouse::isButtonPressed(Mouse::Left) && fire_rate_counter >= current_fire_rate && *current_ammo > 0 )
    {
        for (int i = 0; i < current_bullets_per_shot; i++)
        {
            bullet newbullet;
            newbullet.shape.setFillColor(Color::Magenta);
            newbullet.shape.setRadius(10.f);
            switch (Curr_Gun_state)
            {
            case Pistol: newbullet.shape.setPosition(Norm_dir_vector.x * 50.f + Player.getPosition().x, Norm_dir_vector.y * 50.f + Player.getPosition().y + 15);
                break;
            case Smg:newbullet.shape.setPosition(Norm_dir_vector.x * 150.f + Player.getPosition().x, Norm_dir_vector.y * 150.f + Player.getPosition().y + 30);
                break;
            case Shotgun:newbullet.shape.setPosition(Norm_dir_vector.x * 200.f + Player.getPosition().x, Norm_dir_vector.y * 200.f + Player.getPosition().y +  25);
                break;
            }
            Vector2f Offset(rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX));
            newbullet.currentvelocity = newbullet.maxvelocity * (Norm_dir_vector + (Offset * 0.2f * current_spread));
            newbullet.damage = current_damage;
            bullets.push_back(newbullet);
        }
        *current_ammo-=1;
        fire_rate_counter = 0;
    }
    if (((*current_ammo <= 0 || Keyboard::isKeyPressed(Keyboard::R)) && *current_ammo_stock >= 0) || isreloading)
    {
        if (!isreloading)
        {
            *current_ammo_stock += *current_ammo;
            *current_ammo = 0;
        }
        isreloading = true;
        reload_time_counter += dt;
        if (reload_time_counter > current_reload_time)
        {
            if (*current_ammo_stock <= current_clip_size)
            {
                *current_ammo = *current_ammo_stock;
                *current_ammo_stock = 0;
            }
            else
            {
                *current_ammo += current_clip_size;
                *current_ammo_stock -= current_clip_size;
            }
            reload_time_counter = 0;
            isreloading = false;
        }
    }
    std::cout << *current_ammo << " " << *current_ammo_stock << " " << current_clip_size << std::endl;
}
void Player_Movement(float dt)
{
    x = 0;
    y = 0;
    if (Keyboard::isKeyPressed(Keyboard::A))
    {
        x = -1;
        //Player.setScale(-0.2f, 0.2f);    
        DashOrigin.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 10);
    }
    if (Keyboard::isKeyPressed(Keyboard::D))
    {
        x = 1;
        //Player.setScale(0.2f, 0.2f);
        DashOrigin.setPosition(Player.getPosition().x - 50, Player.getPosition().y + 10);
    }
    if (Keyboard::isKeyPressed(Keyboard::W))
    {
        y = -1;

    }
    if (Keyboard::isKeyPressed(Keyboard::S))
    {
        y = 1;
    }
    DashOrigin.setPosition(DashOrigin.getPosition().x, Player.getPosition().y);
    Gun.setPosition(Player.getPosition().x, Player.getPosition().y + 25.f);
    Player.move(x * dt * playerspeed, y * dt * playerspeed);
}
void init_walls()
{
    RectangleShape wall(Vector2f(128,128));
    wall.setFillColor(Color::Black);
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if (level1[j][i] == 1)
            {
                wall.setPosition(i * 128, j * 128);
                Wall_Bounds.push_back(wall.getGlobalBounds());
            }
        }
    }
}
void select_guns()
{
    if (Keyboard::isKeyPressed(Keyboard::Num1))
    {
        Curr_Gun_state = Gun_State::Pistol;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num2))
    {
        Curr_Gun_state = Gun_State::Smg;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3))
    {
        Curr_Gun_state = Gun_State::Shotgun;
    }
}
void calculate_shoot_dir()
{
    dir_vector = MousePos - Gun.getPosition();
    Norm_dir_vector = dir_vector / (sqrt(dir_vector.x * dir_vector.x + dir_vector.y * dir_vector.y));

    float rotation = atan2(-1 * Norm_dir_vector.y, -1 * Norm_dir_vector.x) * 180 / pi + 180;
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
    FloatRect playerbounds = DashOrigin.getLocalBounds();
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
    if (Player.getScale().x > 0)
    {
        if (Norm_dir_vector.x > 0)
        {
            Gun.setScale(0.2f, 0.2f);
        }
        else
        {
            Gun.setScale(0.2f, -0.2f);
        }
    }
    else
    {
        if (Norm_dir_vector.x > 0)
        {
            Gun.setScale(0.2f, 0.2f);
        }
        else
        {
            Gun.setScale(0.2f, -0.2f);
        }
    }
    if (Norm_dir_vector.x > 0)
    {
        Player.setScale(0.1f, 0.1f);
    }
    else
    {
        Player.setScale(-0.1f, 0.1f);
    }
    RectangleShape wall(Vector2f(128, 128));
    wall.setFillColor(Color::Black);
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if (level1[j][i] == 1)
            {
                wall.setPosition(i * 128, j * 128);
                window.draw(wall);
            }
        }
    }
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
    window.draw(Player);
    switch (Curr_Gun_state)
    {
    case Pistol:
        Pistol_S.setScale(Gun.getScale());
        Pistol_S.setPosition(Gun.getPosition());
        Pistol_S.setRotation(Gun.getRotation());
        window.draw(Pistol_S);
        break;
    case Smg:
        SMG_S.setScale(Gun.getScale());
        SMG_S.setPosition(Gun.getPosition());
        SMG_S.setRotation(Gun.getRotation());


        window.draw(SMG_S);
        break;
    case Shotgun:
        ShotGun_S.setScale(Gun.getScale());
        ShotGun_S.setPosition(Gun.getPosition());
        ShotGun_S.setRotation(Gun.getRotation());
        window.draw(ShotGun_S);
        break;
    }
    window.display();
}