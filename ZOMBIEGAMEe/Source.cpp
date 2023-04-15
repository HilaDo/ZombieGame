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

#define swordfirerate 0.3f
#define sworddamage 5



#define pistolfirerate 0.2f
#define pistoldamage 5
#define pistolspread 0
#define pistolbulletpershot 1
#define pistolclipsize 9
#define pistolreloadtime 1
#define Pistol_bullets_loaded_per_reload pistolclipsize
int pistolammostock = 240;
int pistolbulletsloaded = pistolclipsize;

#define riflefirerate 0.2f
#define rifledamage 3
#define riflespread 0
#define riflebulletpershot 1
#define rifleclipsize 30
#define riflereloadtime 1
#define Rifle_bullets_loaded_per_reload rifleclipsize
int rifleammostock = 999999999;
int riflebulletsloaded = rifleclipsize;

#define shotgunfirerate 0.7f
#define shotgundamage 1
#define shotgunspread 1
#define shotgunbulletpershot 10
#define shotgunclipsize 8
#define shotgunreloadtime 1
#define ShotGun_bullets_loaded_per_reload 1
int shotgunammostock = 10000;
int shotgunbulletsloaded = shotgunclipsize;

using namespace sf;

enum state
{
    idle, walk, dashing, hit, death
};
enum Gun_State
{
    Sword,Pistol, Smg, Shotgun
};
Gun_State Curr_Gun_state = Gun_State::Sword;
struct bullet
{
    CircleShape shape;
    Vector2f currentvelocity;
    float maxvelocity = 5000.0f;
    float damage;
};
void init_walls(); // function to initilize walls at the start of the game
void GetTextures(); //function that gets the used textures by the game at the start of the program

void Update(float dt, state& curr_state); // the main update function where all game logic updates every frame

void Player_Movement(float dt); // function responsible for the player movement
void Player_Collision(float dt); // function that calculates and manages player collision with anything specified
void Switch_States(state& curr_state, float dt); // function that manages the current state of the player, i.e: player state right now is dashing, player state right now is dying, etc
void UpdateAnimationCounter(float dt, int maximagecounter); //function that handles the changing of the animation frame with the specified number of frames as argument

void Dashing(float dt); //function that calculates dash direction and does the dashing
void AddDashLineVertexes(); // function that calculates the verticies of the dash effect
void draw_dash_line(); // function to draw the dash effect behind the player

void calculate_shoot_dir(); // function that calculates the normal direction between the player and the current mouse position
void select_guns(); // function that switches between guns
void knife(float dt);
void Switch_Current_Gun_Attributes(float dt); //function that manages the current held gun attributes, i.e: the current gun fire rate is 1, current gun spread is 0,etc
void Shooting(float dt); //function that handles shooting and spawning new bullets
void Bullet_Movement_Collision(float dt); //function that handles bullet collision
void Guns_Animation_Handling(float dt);
void Gun_switch_cooldown(float dt);
void Gun_UpdateAnimationCounter(float dt, int maximagecounter, double switchtime); // function that handles gun animations
void camera_shake(float dt);

void Draw(); // the main drawing function where everything gets drawn on screen

CircleShape test(15);
Vector2i center;
Vector2f globalcenter;

//all sprites in the game
Sprite Player;
RectangleShape Gun(Vector2f(1.f,1.f));
Sprite Pistol_S;
Sprite SMG_S;
Sprite ShotGun_S;
Sprite Sword_S;
RectangleShape DashOrigin(Vector2f(50.0f, 50.0f));
RenderWindow window(VideoMode(1920, 1080), "ZombieGame",Style::Fullscreen);
RectangleShape wall1(Vector2f(50.0, 10000.0));
RectangleShape wall2(Vector2f(50.0, 10000.0));
RectangleShape wall3(Vector2f(10000.0, 50.0));
RectangleShape wall4(Vector2f(10000.0, 50.0));

Vector2f casingposition;

//dash booleans
bool isdashing = false;
bool dashready = true;
//reload booleans
bool isreloading = false;
//shoot boolean
bool isshooting = false;
bool trigger = true;

bool knifed = false;
bool iscamerashake = false;
bool delayfinished = false;
//wall bounds vector to detect collision
std::vector <FloatRect> Wall_Bounds;

//animation textures
Texture WalkAnimation[8];
Texture IdleAnimation[6];

Texture Sword_Atk_animation[8];

Texture pistol_shoot_animations[12];
Texture pistol_reload_animation[26];

Texture SMG_Shoot_Animations[24];
Texture SMG_Reload_Animations[16];

Texture shotgun_shoot_animation[14];
Texture shotgun_reload_animation[14];

//reload counter and current time that takes the reload to finish
float current_reload_time;
float reload_time_counter = 0;
//dash counter
float timesincedash = 0;
//animation counter and time between each texture takes
float AnimationCounter = 0;
float AnimationSwitchTime = 0.1f;
int ImageCounter = 0;
int gun_image_counter = 0;
float gun_Animation_Counter = 0;
float camera_shake_counter = 0;
int camera_shake_magnitude = 10;
float camera_shake_duration = 1;
float gun_switch_delay_counter = 0;
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
int current_bullets_loaded_per_reload;
//player positions
Vector2f current_player_pos;
Vector2f previous_player_pos;
Vector2f center_of_player_pos;
//normal direction variables
Vector2f MousePos;
Vector2f dir_vector;
Vector2f Norm_dir_vector;
Vector2f cameraoffset_shake;
//level array
int level1[16][16] =
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1},
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

View view(Vector2f(0, 0), Vector2f(window.getSize().x, window.getSize().y));

int main()
{
    init_walls();
    GetTextures(); 
    state state = state::idle;
    Player.setTexture(WalkAnimation[0]);
    Player.setOrigin(Vector2f(Player.getGlobalBounds().width /2 , Player.getGlobalBounds().height / 2));
    Player.setPosition(Vector2f(100, 100));
    Gun.setOrigin(Player.getOrigin());
    Clock clock;
    Event event;
    while (window.isOpen()) {
        float elapsed = clock.restart().asSeconds();
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)) {

                window.close();
            }
        }
        //view.setCenter(Player.getPosition());
        window.setView(view);
        Update(elapsed, state);
        center = Vector2i(window.getSize().x / 2, window.getSize().y / 2);
        globalcenter = window.mapPixelToCoords(center);
        Draw();
    }

    return 0;
}

//initial functions
void init_walls()
{
    RectangleShape wall(Vector2f(128, 128));
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
void GetTextures()
{
    for (int i = 0; i < 8; i++)
    {
        WalkAnimation[i].loadFromFile("anim by rows/walk/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 6; i++)
    {
        IdleAnimation[i].loadFromFile("anim by rows/idle/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 12; i++)
    {
        pistol_shoot_animations[i].loadFromFile("guns/Sprite-sheets/Pistol_V1.00/Weapon/shooting/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 26; i++)
    {
        pistol_reload_animation[i].loadFromFile("guns/Sprite-sheets/Pistol_V1.00/Weapon/reload/frame (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 16; i++)
    {
        SMG_Shoot_Animations[i].loadFromFile("guns/Sprite-sheets/Assault_rifle_V1.00/test/frame_" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 16; i++)
    {
        SMG_Reload_Animations[i].loadFromFile("guns/Sprite-sheets/Assault_rifle_V1.00/WEAPON/reloading/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 14; i++)
    {
        shotgun_shoot_animation[i].loadFromFile("guns/Sprite-sheets/Shotgun_V1.00/shooting/frame (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 14; i++)
    {
        shotgun_reload_animation[i].loadFromFile("guns/Sprite-sheets/Shotgun_V1.00/reloading/frame (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 8; i++)
    {
        Sword_Atk_animation[i].loadFromFile("knife/tile (" + std::to_string(i) + ").png");
    }
    SMG_S.setOrigin(SMG_S.getLocalBounds().width / 2 + 25, SMG_S.getLocalBounds().height / 2 + 15);
    ShotGun_S.setOrigin(ShotGun_S.getLocalBounds().width / 2 + 25, ShotGun_S.getLocalBounds().height / 2 + 15);
    Pistol_S.setTexture(pistol_shoot_animations[0]);
    Pistol_S.setOrigin(Pistol_S.getLocalBounds().width / 2 - 17, Pistol_S.getLocalBounds().height / 2);
}
//update function
void Update(float dt, state& curr_state)
{
    current_player_pos = DashOrigin.getPosition();
    Vector2i pixelpos = Mouse::getPosition(window);
    MousePos = window.mapPixelToCoords(pixelpos);
    Player_Movement(dt);
    Player_Collision(dt);
    Switch_States(curr_state, dt);

    Dashing(dt);

    calculate_shoot_dir();
    select_guns();
    Switch_Current_Gun_Attributes(dt);
    knife(dt);
    Shooting(dt);
    Bullet_Movement_Collision(dt);
    Gun_switch_cooldown(dt);
    if (delayfinished){ Guns_Animation_Handling(dt); }
    camera_shake(dt);

    previous_player_pos = current_player_pos;
}

//Player-related functions
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
    Gun.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 20.f);
    Player.move(x * dt * playerspeed, y * dt * playerspeed);
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
    case state::walk: UpdateAnimationCounter(dt, 4); Player.setTexture(WalkAnimation[ImageCounter]);break;
    case state::idle: UpdateAnimationCounter(dt, 4); Player.setTexture(IdleAnimation[ImageCounter]); break;
    }
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

//dashing functions
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
            timesincedash = 0;
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
void draw_dash_line()
{
    AddDashLineVertexes();
    for (int i = 0; i < dasheline.getVertexCount(); i++)
    {
        dasheline[i].color = Color(107, 107, 107);
    }
    window.draw(dasheline);
}

//shooting functions
void calculate_shoot_dir()
{
    dir_vector = MousePos - Gun.getPosition();
    Norm_dir_vector = dir_vector / (sqrt(dir_vector.x * dir_vector.x + dir_vector.y * dir_vector.y));

    float rotation = atan2(Norm_dir_vector.y,Norm_dir_vector.x) * 180 / pi;
    Gun.setRotation(rotation);
}
void select_guns()
{
    if (Keyboard ::isKeyPressed(Keyboard::Num1))
    {
        Curr_Gun_state = Gun_State::Sword;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num2))
    {
        Curr_Gun_state = Gun_State::Pistol;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3))
    {
        Curr_Gun_state = Gun_State::Smg;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num4))
    {
        Curr_Gun_state = Gun_State::Shotgun;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
}
void knife(float dt)
{
    if (Mouse::isButtonPressed(Mouse::Left) && fire_rate_counter >= current_fire_rate)
    {
        fire_rate_counter = 0;
        knifed = true;
    }
    else
    {
        knifed = false;
    }
}
void Switch_Current_Gun_Attributes(float dt)
{
    switch (Curr_Gun_state)
    {
    case Sword:
        current_fire_rate = swordfirerate;
        current_damage = sworddamage;
        break;
    case Pistol:
        current_fire_rate = pistolfirerate;
        current_damage = pistoldamage;
        current_bullets_per_shot = pistolbulletpershot;
        current_spread = pistolspread;
        current_ammo = &pistolbulletsloaded;
        current_ammo_stock = &pistolammostock;
        current_clip_size = pistolclipsize;
        current_reload_time = pistolreloadtime;
        current_bullets_loaded_per_reload = Pistol_bullets_loaded_per_reload;
        camera_shake_magnitude = 3;
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
        current_bullets_loaded_per_reload = Rifle_bullets_loaded_per_reload;
        camera_shake_magnitude = 3;
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
        current_bullets_loaded_per_reload = ShotGun_bullets_loaded_per_reload;
        camera_shake_magnitude = 5;
        break;
    }
}
void Shooting(float dt)
{
    fire_rate_counter += dt;
    if (Curr_Gun_state != Gun_State ::Sword)
    {
        if (Mouse::isButtonPressed(Mouse::Left) && fire_rate_counter >= current_fire_rate && *current_ammo > 0 && !isreloading)
        {
            camera_shake_counter += 1;
            if (camera_shake_counter > camera_shake_duration)
            {
                camera_shake_counter = camera_shake_duration;
            }
            for (int i = 0; i < current_bullets_per_shot; i++)
            {
                bullet newbullet;
                newbullet.shape.setFillColor(Color::Magenta);
                newbullet.shape.setRadius(10.f);
                switch (Curr_Gun_state)
                {
                case Pistol: newbullet.shape.setPosition(test.getPosition());
                    break;
                case Smg:newbullet.shape.setPosition(test.getPosition());
                    break;
                case Shotgun:newbullet.shape.setPosition(test.getPosition());
                    break;
                }
                Vector2f Offset(rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX));
                newbullet.currentvelocity = newbullet.maxvelocity * (Norm_dir_vector + (Offset * 0.2f * current_spread));
                newbullet.damage = current_damage;
                bullets.push_back(newbullet);
            }
            *current_ammo -= 1;
            fire_rate_counter = 0;
        }
        if ((Keyboard::isKeyPressed(Keyboard::R) && (*current_ammo_stock >= 0 && *current_ammo != current_clip_size)) || isreloading)
        {
            if (!isreloading)
            {
                if (Curr_Gun_state != Gun_State::Shotgun)
                {
                    *current_ammo_stock += *current_ammo;
                    *current_ammo = 0;
                }
            }
            isreloading = true;
            reload_time_counter += dt;
            if (reload_time_counter > current_reload_time)
            {
                if (*current_ammo_stock >= current_clip_size || (Curr_Gun_state == Gun_State::Shotgun && *current_ammo_stock > 0))
                {
                    *current_ammo += current_bullets_loaded_per_reload;
                    *current_ammo_stock -= current_bullets_loaded_per_reload;
                }
                else
                {
                    *current_ammo = *current_ammo_stock;
                    *current_ammo_stock = 0;
                }
                reload_time_counter = 0;
            }
            if ((Curr_Gun_state == Gun_State::Shotgun && Mouse::isButtonPressed(Mouse::Left)) || *current_ammo >= current_clip_size)
            {
                isreloading = false;
            }

        }
        std::cout << *current_ammo << " " << *current_ammo_stock << " " << current_clip_size << std::endl;
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
void Guns_Animation_Handling(float dt)
{
    int gun_frames = 24;
    if (Mouse::isButtonPressed(Mouse::Left) && !isreloading && *current_ammo > 0)
    {
        isshooting = true;
        
        if (trigger == true)
        {
            gun_image_counter = 0;
            trigger = false;
        }
        switch (Curr_Gun_state)
        {
        case Sword:
            Sword_S.setTexture(Sword_Atk_animation[ImageCounter]);
            gun_frames = 8;
            break;
        case Pistol:
            Pistol_S.setTexture(pistol_shoot_animations[gun_image_counter]);
            gun_frames = 12;
            break;
        case Smg:
            SMG_S.setTexture(SMG_Shoot_Animations[gun_image_counter]);
            gun_frames = 16;
            break;
        case Shotgun:
            ShotGun_S.setTexture(shotgun_shoot_animation[gun_image_counter]);
            gun_frames = 14;
            break;
        }
    }
    else if (Keyboard::isKeyPressed(Keyboard::R) && Curr_Gun_state != Gun_State::Sword)
    {
        trigger = true;
    }
    else if (isreloading && *current_ammo_stock > 0 && Curr_Gun_state != Gun_State::Sword)
    {
        if (trigger)
        {
            gun_image_counter = 0;
            reload_time_counter = 0;
            trigger = false;
        }
        switch (Curr_Gun_state)
        {
        case Pistol:
            Pistol_S.setTexture(pistol_reload_animation[gun_image_counter]);
            gun_frames = 26;
            break;
        case Smg:
            SMG_S.setTexture(SMG_Reload_Animations[gun_image_counter]);
            gun_frames = 16;
            break;
        case Shotgun:
            ShotGun_S.setTexture(shotgun_reload_animation[gun_image_counter]);
            gun_frames = 14;
            break;
        }
    }
    else
    {
        SMG_S.setTexture(SMG_Shoot_Animations[0]);
        Pistol_S.setTexture(pistol_shoot_animations[0]);
        ShotGun_S.setTexture(shotgun_shoot_animation[0]);
        Sword_S.setTexture(Sword_Atk_animation[0]);
        gun_switch_delay_counter = current_fire_rate;
        trigger = true;
        isshooting = false;
    }
    if (isreloading)
    {
        Gun_UpdateAnimationCounter(dt, gun_frames, current_reload_time / (double)gun_frames);
    }
    else if(!isreloading || Curr_Gun_state == Gun_State::Sword)
    {
        Gun_UpdateAnimationCounter(dt, gun_frames, current_fire_rate /(double) gun_frames );
    }
}
void Gun_switch_cooldown(float dt)
{
    if (gun_switch_delay_counter > 0)
    {
        gun_switch_delay_counter -= dt;
        delayfinished = false;
    }
    else
    {
        delayfinished = true;
    }
}
void Gun_UpdateAnimationCounter(float dt, int maximagecounter,double switchtime)
{
    gun_Animation_Counter += dt;
    if (gun_Animation_Counter >= switchtime)
    {
        gun_Animation_Counter = 0;
        gun_image_counter++;
        if (gun_image_counter >= maximagecounter)
        {
            gun_image_counter = 0;
        }
    }
}
void camera_shake(float dt)
{
    if (camera_shake_counter > 0)
    {
        float magnitudereduction = camera_shake_magnitude * (camera_shake_counter / camera_shake_duration);
        cameraoffset_shake = Vector2f((rand() /static_cast<float>( RAND_MAX )) * magnitudereduction - magnitudereduction / 2, (rand() / static_cast<float>(RAND_MAX)) * magnitudereduction - magnitudereduction / 2);
        view.setCenter(Player.getPosition() + cameraoffset_shake);
        camera_shake_counter -= dt;
    }
    else
    {
        view.setCenter(Player.getPosition());
    }
}

//drawing function
void Draw()
{
    window.clear(Color::White);

    if (Norm_dir_vector.x > 0)
    {
        Player.setScale(3, 3);
        Gun.setScale(0.2f, 0.2f);
    }
    else
    {
        Player.setScale(-3, 3);
        Gun.setScale(0.2f, -0.2f);
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
    case Sword:
        Sword_S.setScale(Gun.getScale().x * 6, Gun.getScale().y * 6);
        Sword_S.setPosition(Gun.getPosition());
        Sword_S.setRotation(Gun.getRotation());
        window.draw(Sword_S);
        break;
    case Pistol:
        Pistol_S.setScale(Gun.getScale().x * 6, Gun.getScale().y * 6);
        Pistol_S.setPosition(Gun.getPosition());
        Pistol_S.setRotation(Gun.getRotation());
        window.draw(Pistol_S);
        break;
    case Smg:
        SMG_S.setScale(Gun.getScale().x * 6, Gun.getScale().y * 6);
        SMG_S.setPosition(Gun.getPosition());
        SMG_S.setRotation(Gun.getRotation());
        window.draw(SMG_S);
        break;
    case Shotgun:
        ShotGun_S.setScale(Gun.getScale().x * 6, Gun.getScale().y * 6);
        ShotGun_S.setPosition(Gun.getPosition());
        ShotGun_S.setRotation(Gun.getRotation());
        window.draw(ShotGun_S);
        break;
    }
    test.setFillColor(Color::Green);
    test.setPosition(Gun.getPosition().x-20 + cos(Gun.getRotation()/180 * pi) * 75, Gun.getPosition().y-10+ sin(Gun.getRotation()/180 * pi) * 75);
    if (knifed)
    {
        window.draw(test);
    }
    window.display();
}







