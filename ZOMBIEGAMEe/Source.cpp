#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <SFML/Audio.hpp>
#include "Candle/LightSource.hpp"


using namespace std;
//to be displayed on ui
int Player_Health = 10000;
int Score= 0;
int Money= 0;
int const speedmoney = 2000;
int const reloadmoney = 2000;
float speedmulti = 1;
float reloadmulti = 1;

int current_level = 1;

//buying weapon
bool pistol_buy = false, smg_buy = false, shotgun_buy = false,sniper_buy = false,speed_pow=false,reload_pow=false;
int const money_pistol = 500, money_smg = 1000, money_shotgun = 1500;


#define pi 3.14159265
#define Level1NumWaves 3

#define pistolfirerate 0.2f
#define pistoldamage 6
#define pistolspread 0
#define pistolbulletpershot 1
#define pistolclipsize 9
#define pistolreloadtime 1
#define Pistol_bullets_loaded_per_reload pistolclipsize
int pistolammostock = 240;
int pistolbulletsloaded = pistolclipsize;

#define riflefirerate 0.08f
#define rifledamage 4.5
#define riflespread 0
#define riflebulletpershot 1
#define rifleclipsize 30
#define riflereloadtime 1
#define Rifle_bullets_loaded_per_reload rifleclipsize
int rifleammostock = 250;
int riflebulletsloaded = rifleclipsize;

#define shotgunfirerate 0.7f
#define shotgundamage 1.5
#define shotgunspread 1
#define shotgunbulletpershot 10
#define shotgunclipsize 8
#define shotgunreloadtime 1
#define ShotGun_bullets_loaded_per_reload 1
int shotgunammostock = 250;
int shotgunbulletsloaded = shotgunclipsize;

#define sniperfirerate 1.6f
#define sniperdamage 25
#define sniperspread 0
#define sniperbulletpershot 1
#define sniperclipsize 5
#define sniperreloadtime 2
#define sniper_bullets_loaded_per_reload sniperclipsize
int sniperammostock = 250;
int sniperbulletsloaded = sniperclipsize;


using namespace sf;

enum state
{
    idle, walk, hit, death
};
state curr_state = state::idle;
enum Gun_State
{
    Pistol, Smg, Shotgun,Sniper
};
Gun_State Curr_Gun_state = Gun_State::Pistol;
struct bullet
{
    Sprite shape;
    Vector2f currentvelocity;
    float maxvelocity = 5000.0f;
    float damage;
    int id;
    float animation_counter = 0;
    float animation_switch_time = 0.1f;
    int bullet_image_counter = 0;
    void animation(float dt,Texture bulletanimation[])
    {
        animation_counter += dt;
        if (animation_counter >= animation_switch_time)
        {
            bullet_image_counter++;
            animation_counter = 0;
            if (bullet_image_counter > 5)
            {
                bullet_image_counter = 0;
            }
        }
        shape.setTexture(bulletanimation[bullet_image_counter]);
    }
};
bool ishit = false;
struct Zombie
{
    Sprite shape;
    Sprite SpawnEffect;
    bool atkready = true;
    Vector2f currentvelocity;
    int last_hit_bullet_id = -1;
    float maxvelocity = 175;
    float damage = 10;
    float attack_fire_rate = 1;
    float fire_rate_counter = 0;
    float animation_counter = 0;
    float animation_duration = 0.20f;
    int imagecounter = 0;

    float Death_animation_counter = 0;
    float Death_animation_duration = 0.25f;
    int Death_imagecounter = 0;

    float Hit_animation_counter = 0;
    float Hit_animation_duration = 0.20f;
    int Hit_imagecounter = 0;

    float Spawn_animation_counter = 0;
    float Spawn_animation_duration = 0.15f;
    int Spawn_imagecounter = 0;

    int numberofframes = 8;
    int health = 75;
    int current_frames = 7;
    float distance_from_player;
    bool isdeath = false;
    bool iszombiehit = false;
    bool remove_zombie = false;
    void setspawnlocation()
    {
        SpawnEffect.setPosition(shape.getPosition());
        SpawnEffect.setScale(2, 2);
    }
    void Zombie_Behaviour1(Vector2f player_pos,float dt,Texture walk_anim[], Texture atk_anim[], Texture hit_anim[], Texture death_anim[],Texture Spawn_anim[])
    {
        if (Spawn_imagecounter <= 6)
        {
            Spawn_animation_counter += dt;
            SpawnEffect.setTexture(Spawn_anim[Spawn_imagecounter]);
            if (Spawn_animation_counter >= Spawn_animation_duration)
            {
                Spawn_animation_counter = 0;
                Spawn_imagecounter++;
            }
        }      
        if (iszombiehit && !isdeath)
        {
            Hit_animation_counter += dt;         
            if (Hit_animation_counter >= Hit_animation_duration)
            {
                Hit_animation_counter = 0;
                Hit_imagecounter++;
                if (Hit_imagecounter >= 2)
                {
                    iszombiehit = false;
                    Hit_imagecounter = 0;
                }
            }
            shape.setTexture(hit_anim[Hit_imagecounter]);
        }
        else if (!isdeath && !iszombiehit)
        {
            animation_counter += dt;
            if (animation_counter >= animation_duration)
            {
                animation_counter = 0;
                imagecounter++;
                if (imagecounter >= current_frames)
                {
                    imagecounter = 0;
                }
            }
            if (!atkready)
            {
                fire_rate_counter += dt;
                if (fire_rate_counter >= attack_fire_rate)
                {
                    atkready = true;
                    fire_rate_counter = 0;
                }
            }
            Vector2f Direction = player_pos - shape.getPosition();
            float magnitude = sqrt(Direction.x * Direction.x + Direction.y * Direction.y);
            Vector2f norm_Direction = Direction / magnitude;
            distance_from_player = magnitude;
            if (distance_from_player <= 75.0f)
            {
                shape.setTexture(atk_anim[imagecounter]);
                current_frames = 7;
                if (atkready)
                {
                    Player_Health -= damage;
                    ishit = true;
                    atkready = false;
                }
            }
            else
            {
                current_frames = 8;
                shape.setTexture(walk_anim[imagecounter]);
                currentvelocity = norm_Direction * maxvelocity;
                shape.move(currentvelocity * dt);
            }
        }
        else if (isdeath)
        {
            Death_animation_counter += dt;
            if (Death_animation_counter >= Death_animation_duration)
            {

                if (Death_imagecounter < 5)
                {
                    Death_imagecounter++;
                }
                if (Death_animation_counter >= 5)
                {
                    remove_zombie = true;
                }
            }
            shape.setTexture(death_anim[Death_imagecounter]);
        }
    }
};
struct BloodEffect
{
    Sprite BloodShape;
    float Hit_Blood_animation_counter = 0;
    float Hit_Blood_animation_duration = 0.20f;
    int Hit_Blood_imagecounter = 0;
    int randomnumber;
    bool DeleteMe = false;
    void assign_random_number()
    {
        randomnumber = rand() % 9;
    }
    void HandleBloodEffect(Texture blood_anim[], float dt)
    {
        Hit_Blood_animation_counter += dt;
        if (Hit_Blood_imagecounter <= 8)
        {
            Hit_Blood_animation_counter += dt;
            if (Hit_Blood_animation_counter >= 0.1f)
            {
                Hit_Blood_animation_counter = 0;
                Hit_Blood_imagecounter++;
            }
            BloodShape.setTexture(blood_anim[Hit_Blood_imagecounter]);
        }
        else
        {
            if (Hit_Blood_animation_counter >= 5.8)
            {
                DeleteMe = true;
            }
            BloodShape.setTexture(blood_anim[randomnumber]);
        }
    }
};
void init_walls(); // function to initilize walls at the start of the game
void GetTextures(); //function that gets the used textures by the game at the start of the program

void Update(float dt); // the main update function where all game logic updates every frame

void Player_Movement(float dt); // function responsible for the player movement
void Player_Collision(float dt); // function that calculates and manages player collision with anything specified
void Switch_States(float dt); // function that manages the current state of the player, i.e: player state right now is dashing, player state right now is dying, etc
void UpdateAnimationCounter(float dt, int maximagecounter, bool isonce); //function that handles the changing of the animation frame with the specified number of frames as argument
void UpdatePortalAnimation(float dt);

void Dashing(float dt); //function that calculates dash direction and does the dashing
void AddDashLineVertexes(); // function that calculates the verticies of the dash effect
void draw_dash_line(); // function to draw the dash effect behind the player

void calculate_shoot_dir(); // function that calculates the normal direction between the player and the current mouse position
void buying_weapons(); //function that  open guns for player 
void select_guns(); // function that switches between guns
void Switch_Current_Gun_Attributes(float dt); //function that manages the current held gun attributes, i.e: the current gun fire rate is 1, current gun spread is 0,etc
void Shooting(float dt); //function that handles shooting and spawning new bullets
void Bullet_Movement_Collision(float dt); //function that handles bullet collision
void Guns_Animation_Handling(float dt);
void Gun_switch_cooldown(float dt);
void Gun_UpdateAnimationCounter(float dt, int maximagecounter, double switchtime); // function that handles gun animations
void camera_shake(float dt);

void SpawnZombiesWaves(float dt);
void HandleZombieBehaviour(float dt);

void SwtichCurrentWallBounds();

void background();
void wall();
void block();

//level 2 functions

void background2();
void wall2();
void block2();

//level 3 functions

void background3();
void wall3();
void block3();

void Draw(); // the main drawing function where everything gets drawn on screen


//levels


//door 

Texture TDoor;
Sprite Door[19];


//level 3 variables

Texture Tbackground3;
Sprite Background3[4][7];

Texture HwallT3, VwallT3;
Sprite Hwall3[14], Vwall3[18];


//level 2 variables

Texture Tbackground2;
Sprite Background2[4][7];

Texture HwallT, VwallT;
Sprite Hwall[15], Vwall[15];

Texture TME2;
Sprite ME2[8];


//level 1 variables

Texture Tbackground;
Sprite background1[7][10];

Texture VwallT1, HwallT1;
Sprite Vwall1[15], Hwall1[15];

Texture ME;
Sprite ME1[7];




CircleShape test(10);
Vector2i center;
Vector2f globalcenter;
Vector2f globalorigin;

unsigned long long numberoftotalbulletsshot = 0;
//all font declear
Font normal_font;
Font blood_font;
// declear all text ui 
Text precent_sign;  
Text score_text;
Text money_text;
Text health_precent_text;
Text current_wave;
Text current_ammo_text;
Text ammo_stock_text;
//all sprites in the game
Sprite Player;
RectangleShape Gun(Vector2f(1.f,1.f));
RectangleShape BlackScreen(Vector2f(4000, 4000));
Sprite Pistol_S;
Sprite SMG_S;
Sprite ShotGun_S;
Sprite Sniper_S;
Sprite pistol_buying;
Sprite smg_buying;
Sprite shotgun_buying;
Sprite sniper_buying;
Sprite full_heart;
Sprite money;
Sprite ammo_shotgun;
Sprite ammo_pistol;
Sprite ammo_smg;
Sprite ammo_box;
Sprite speedmachine;
Sprite reloadmachine;
Sprite Crosshair;
Sprite Portal_S;
Sprite void1[4];
RectangleShape DashOrigin(Vector2f(50.0f, 50.0f));
RenderWindow window(VideoMode(1920, 1080), "ZombieGame",Style::Fullscreen);


Vector2f casingposition;

bool finishedanimationonce = false;
bool PortalOpen = false;

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

bool SpawnedAZombie = false;
//wall bounds vector to detect collision
vector <FloatRect> Wall_Bounds;
vector <FloatRect> Wall_Bounds1;
vector <FloatRect> Wall_Bounds2;
vector <FloatRect> Wall_Bounds3;

//animation textures
Texture PortalAnimation[15];

Texture WalkAnimation[8];
Texture IdleAnimation[6];
Texture HitAnimation[4];
Texture DeathAnimation[9];

Texture sniper_shoot_animations[9];
Texture sniper_reload_animation[40];

Texture pistol_shoot_animations[12];
Texture pistol_reload_animation[26];

Texture SMG_Shoot_Animations[24];
Texture SMG_Reload_Animations[16];

Texture shotgun_shoot_animation[14];
Texture shotgun_reload_animation[14];

Texture bullet_animation[5];

Texture zombie_walk_animation[8];
Texture zombie_atk_animation[7];
Texture zombie_hit_animation[2];
Texture zombie_death_animation[6];

Texture zombie2_walk_animation[8];
Texture zombie2_atk_animation[7];
Texture zombie2_hit_animation[2];
Texture zombie2_death_animation[6];

Texture Zombie_spawn_animation[7];
Texture zombie_hit_blood_effect[10];

Texture CrossHair_Texture;

Texture Health_Texture;
Texture pistol_photo;
Texture smg_photo;
Texture shotgun_photo;
Texture sniper_photo;
Texture full_heart_photo;
Texture money_photo;
Texture ammo_shotgun_photo;
Texture ammo_pistol_photo;
Texture ammo_smg_photo;

Texture speedmachine_photo;
Texture reloadmachine_photo;

Texture Void;

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
float Wave_Cooldown_counter = 0;
float Wave_Cooldown_duration = 2.0f;
float hit_counter = 0;
float hit_duration = 0.5f;
float SpawningZombieCounter = 0;
float SpawningZombieDuration = 3;
float Portal_animation_counter = 0;
float Portal_animation_duration = 0.1f;
int Portal_imagecounter = 0;
//firerate counter
float fire_rate_counter;
//player speed and direction(x,y)
float x, y, playerspeed = 175;

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

//zombie variables
int Current_Wave1 = 1;
int TotalSpawnedZombies = 0;
bool canspawn = true;

VertexArray dasheline(Quads);

SoundBuffer footstepsBuffer;
Sound footstepsSound;

vector<bullet> bullets;
vector <Zombie> zombies;
vector <BloodEffect> bloodeffects;
vector<Sprite> HealthPacks;
vector<Sprite> AmmoPacks;

View view(Vector2f(0, 0), Vector2f(window.getSize().x, window.getSize().y));


int main()
{
    init_walls();
    GetTextures(); 
    footstepsSound.setLoop(true);
    footstepsSound.setVolume(100);
    Player.setTexture(WalkAnimation[0]);
    Player.setOrigin(Vector2f(Player.getGlobalBounds().width /2 , Player.getGlobalBounds().height / 2));
    Player.setPosition(Vector2f(500, 500));
    Gun.setOrigin(Player.getOrigin());
    Clock clock;
    Event event;
    window.setMouseCursorVisible(false);
    view.zoom(0.65);

    ifstream inf("savegame.txt");
    inf >>current_level;
    inf >> Player_Health;
    inf >> Score;
    inf.close();

    SwtichCurrentWallBounds();
    while (window.isOpen()) {
        float elapsed = clock.restart().asSeconds();
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)) {
                ofstream outf("savegame.txt");
                outf << current_level << endl;
                outf << Player_Health << endl;
                outf << Score << endl;
                outf.close();
                window.close();
            }
        }
        window.setView(view);
        Update(elapsed);
        center = Vector2i(window.getSize().x / 2, window.getSize().y / 2);
        globalcenter = window.mapPixelToCoords(center);
        Draw();
    }

    return 0;
}

//initial functions
void init_walls()
{
    //level 1
    background();
    wall();
    block();
    //level 2
    background2();
    wall2();
    block2();
    //level 3
    background3();
    wall3();
    block3();
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
    for (int i = 0; i < 4; i++)
    {
        HitAnimation[i].loadFromFile("anim by rows/hit/tile (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 9; i++)
    {
        DeathAnimation[i].loadFromFile("anim by rows/death/tile (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 5; i++)
    {
        bullet_animation[i].loadFromFile("guns/bullet animation/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 12; i++)
    {
        pistol_shoot_animations[i].loadFromFile("guns/Sprite-sheets/Pistol_V1.00/Weapon/shooting/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 26; i++)
    {
        pistol_reload_animation[i].loadFromFile("guns/Sprite-sheets/Pistol_V1.00/Weapon/reload/tile" + std::to_string(i) + ".png");
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
    for (int i = 0; i < 9; i++)
    {
        sniper_shoot_animations[i].loadFromFile("guns/Sprite-sheets/SNIPER_RIFLE_V1.00 [KAR98]/Shooting/frame (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 40; i++)
    {
        sniper_reload_animation[i].loadFromFile("guns/Sprite-sheets/SNIPER_RIFLE_V1.00 [KAR98]/Reloading/frame (" + std::to_string(i) + ").png");
    }
    for (int i = 0; i < 8; i++)
    {
        zombie_walk_animation[i].loadFromFile("Zombies Animation/walk/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 7; i++)
    {
        zombie_atk_animation[i].loadFromFile("Zombies Animation/attack/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 2; i++)
    {
        zombie_hit_animation[i].loadFromFile("Zombies Animation/Hit/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 6; i++)
    {
        zombie_death_animation[i].loadFromFile("Zombies Animation/Death/tile" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 7; i++)
    {
        Zombie_spawn_animation[i].loadFromFile("FX/ZombieSpawnEffect/frame" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 10; i++)
    {
        zombie_hit_blood_effect[i].loadFromFile("FX/blood/frame" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < 15; i++)
    {
        PortalAnimation[i].loadFromFile("FX/WarpGate/tile" + std::to_string(i) + ".png");
    }
    CrossHair_Texture.loadFromFile("weapons/crosshair.png");
    Crosshair.setTexture(CrossHair_Texture);
    Crosshair.setOrigin(Crosshair.getLocalBounds().width / 2 + 50, Crosshair.getLocalBounds().height / 2);
    Crosshair.setScale(0.3, 0.3);
    SMG_S.setOrigin(SMG_S.getLocalBounds().width / 2 + 25, SMG_S.getLocalBounds().height / 2 + 15);
    ShotGun_S.setOrigin(ShotGun_S.getLocalBounds().width / 2 + 25, ShotGun_S.getLocalBounds().height / 2 + 15);
    Pistol_S.setOrigin(Pistol_S.getLocalBounds().width / 2 +10, Pistol_S.getLocalBounds().height / 2+20 );
    Sniper_S.setOrigin(Sniper_S.getLocalBounds().width / 2, Sniper_S.getLocalBounds().height / 2);
    Health_Texture.loadFromFile("health-red_32px.png");

    speedmachine_photo.loadFromFile("speedvanding.png");
    reloadmachine_photo.loadFromFile("ReloadVanding.png");
    speedmachine.setTexture(speedmachine_photo);
    reloadmachine.setTexture(reloadmachine_photo);
    //ui
    normal_font.loadFromFile("font of score and money.ttf");
    blood_font.loadFromFile("font of current wave.ttf"); // to load font files 
    ammo_shotgun_photo.loadFromFile("ammo_shotgun.png");
    ammo_smg_photo.loadFromFile("ammo_smg.png");
    ammo_pistol_photo.loadFromFile("ammo_pistol.png");
    full_heart_photo.loadFromFile("full_heart.png");
    money_photo.loadFromFile("money.png");
    pistol_photo.loadFromFile("pistol.png");
    smg_photo.loadFromFile("smg.png");
    shotgun_photo.loadFromFile("shotgun.png");
    sniper_photo.loadFromFile("sniper.png");
    sniper_buying.setTexture(sniper_photo);
    Void.loadFromFile("void.jpg");
    for (int i = 0; i < 4; i++)
    {
        void1[i].setTexture(Void);
    }
}
//update function
void Update(float dt)
{

    Switch_States(dt);
    
    if (curr_state != state::death)
    {  

        if (Keyboard::isKeyPressed(Keyboard::O));
        {
            cout << Player.getPosition().x <<"\t" << Player.getPosition().y << endl;
        }

        current_player_pos = DashOrigin.getPosition();
        Vector2i pixelpos = Mouse::getPosition(window);
        MousePos = window.mapPixelToCoords(pixelpos);
        globalorigin = window.mapPixelToCoords(Vector2i(0,0));
        Player_Movement(dt);
        Player_Collision(dt);

        Dashing(dt);
        //zombies
        SpawnZombiesWaves(dt);
        HandleZombieBehaviour(dt);

        calculate_shoot_dir();
        buying_weapons();
        select_guns();
        Switch_Current_Gun_Attributes(dt);
        Shooting(dt);
        Bullet_Movement_Collision(dt);
        Gun_switch_cooldown(dt);
        if (delayfinished) { Guns_Animation_Handling(dt); }
        camera_shake(dt);

        previous_player_pos = current_player_pos;
        //vanding
    }

}

//Player-related functions
void Player_Movement(float dt)
{
    x = 0;
    y = 0;
    if (Keyboard::isKeyPressed(Keyboard::A))
    {
        x = -1; 
        DashOrigin.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 10);
    }
    if (Keyboard::isKeyPressed(Keyboard::D))
    {
        x = 1;
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
    if (Player_Health < 100)
    {
        for (int i = 0; i < HealthPacks.size(); i++)
        {
            if (Player.getGlobalBounds().intersects(HealthPacks[i].getGlobalBounds()))
            {
                Player_Health += 20;
                HealthPacks.erase(HealthPacks.begin() + i);
            }
        }
    }
    else if (Player_Health > 100)
    {
        Player_Health = 100;
    }
    for (int i = 0; i < AmmoPacks.size(); i++)
    {
        if (Player.getGlobalBounds().intersects(AmmoPacks[i].getGlobalBounds()))
        {
            pistolammostock += 36;
            rifleammostock += 120 ;
            shotgunammostock += 16;
            sniperammostock += 15;
            AmmoPacks.erase(AmmoPacks.begin() + i);
        }
    }
    //vandingmachine
    if (Player.getGlobalBounds().intersects(speedmachine.getGlobalBounds()) && Money >= speedmoney && speed_pow == false && Keyboard::isKeyPressed(Keyboard::Key::E)) {
        playerspeed *= 2;
        speedmulti = 2;
        Money -= speedmoney;
        speed_pow = true;
    }

    if (Player.getGlobalBounds().intersects(reloadmachine.getGlobalBounds()) && Money >= reloadmoney && reload_pow == false && Keyboard::isKeyPressed(Keyboard::Key::E)) {
        reloadmulti = 0.5f;
        Money -= reloadmoney;
        reload_pow = true;
    }
    if (Player.getGlobalBounds().intersects(Portal_S.getGlobalBounds()) && PortalOpen && Keyboard::isKeyPressed(Keyboard::E))
    {
        PortalOpen = false;
        current_level++;
        pistol_buy = false;
        smg_buy = false;
        shotgun_buy = false;
        speed_pow = false;
        reload_pow = false;
        speedmulti = 1;
        reloadmulti = 1;
        Money = 0;
        Wall_Bounds.clear();
        HealthPacks.clear();
        Curr_Gun_state = Pistol;
        playerspeed = 175;
        SwtichCurrentWallBounds();
        Current_Wave1 = 0;
    }
}
void Switch_States(float dt)
{
    if (x == 1 || x == -1 || y == 1 || y == -1)
    {
        curr_state = state::walk;
    }
    else
    {
        curr_state = state::idle;
    }
    if (Player_Health <= 0)
    {
        curr_state = state::death;
    }
    if (ishit)
    {
        curr_state = state::hit;
    }
    switch (curr_state)
    {
    case state::walk: UpdateAnimationCounter(dt, 8, false); Player.setTexture(WalkAnimation[ImageCounter]); break;
    case state::idle: UpdateAnimationCounter(dt, 6, false); Player.setTexture(IdleAnimation[ImageCounter]); break;
    case state::death: UpdateAnimationCounter(dt, 6, true); Player.setTexture(DeathAnimation[ImageCounter]); break;
    case state::hit: UpdateAnimationCounter(dt, 4, true); Player.setTexture(HitAnimation[ImageCounter]); break;
    }
}
void UpdateAnimationCounter(float dt,int maximagecounter,bool isonce)
{
    if (isonce && finishedanimationonce)
    {
        if (curr_state == state::hit)
        {
            ishit = false;
        }
        curr_state = state::idle;
        finishedanimationonce = false;
        return;
    }
    AnimationCounter += dt;
    if (AnimationCounter >= AnimationSwitchTime)
    {
        AnimationCounter = 0;
        ImageCounter++;
        if (ImageCounter >= maximagecounter)
        {
            finishedanimationonce = true;
            ImageCounter = 0;
        }
    }
}
void UpdatePortalAnimation(float dt)
{
    Portal_animation_counter += dt;
    if (Portal_animation_counter >= Portal_animation_duration)
    {
        Portal_animation_counter = 0;
        Portal_imagecounter++;
        if (Portal_imagecounter >=15)
        {
            Portal_imagecounter = 0;
        }
    }
    Portal_S.setTexture(PortalAnimation[Portal_imagecounter]);
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
            playerspeed = 175 * speedmulti;
            timesincedash = 0;
        }
    }
    if (!isdashing && !dashready)
    {
        timesincedash += dt;
        if (timesincedash > 2.0)
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
    if (Keyboard::isKeyPressed(Keyboard::Num1) && pistol_buy)   
    {
        Curr_Gun_state = Gun_State::Pistol;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num2) && smg_buy)
    {
        Curr_Gun_state = Gun_State::Smg;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3) && shotgun_buy)
    {
        Curr_Gun_state = Gun_State::Shotgun;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num4) && sniper_buy)
    {
        Curr_Gun_state = Gun_State::Sniper;
        gun_switch_delay_counter = current_fire_rate;
        fire_rate_counter = 100;
        trigger = true;
    }
}
void Switch_Current_Gun_Attributes(float dt)
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
        current_reload_time = pistolreloadtime * reloadmulti;
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
        current_reload_time = riflereloadtime * reloadmulti;
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
        current_reload_time = shotgunreloadtime * reloadmulti;
        current_bullets_loaded_per_reload = ShotGun_bullets_loaded_per_reload;
        camera_shake_magnitude = 7;
        break;
    case Sniper:
        current_fire_rate = sniperfirerate;
        current_damage = sniperdamage;
        current_bullets_per_shot = sniperbulletpershot;
        current_spread = sniperspread;
        current_ammo = &sniperbulletsloaded;
        current_ammo_stock = &sniperammostock;
        current_clip_size = sniperclipsize;
        current_reload_time = sniperreloadtime * reloadmulti;
        current_bullets_loaded_per_reload = sniper_bullets_loaded_per_reload;
        camera_shake_magnitude = 12;
        break;
    }
}
void Shooting(float dt)
{
    fire_rate_counter += dt;
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
            switch (Curr_Gun_state)
            {
            case Pistol: newbullet.shape.setPosition(test.getPosition());
                break;
            case Smg:newbullet.shape.setPosition(test.getPosition());
                break;
            case Shotgun:newbullet.shape.setPosition(test.getPosition());
                break;
            case Sniper:newbullet.shape.setPosition(test.getPosition());
                break;
            }
            newbullet.id = numberoftotalbulletsshot;
            Vector2f Offset(rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX));
            newbullet.currentvelocity = newbullet.maxvelocity * (Norm_dir_vector + (Offset * 0.2f * current_spread));
            newbullet.damage = current_damage;
            bullets.push_back(newbullet);
            numberoftotalbulletsshot++;
        }
        *current_ammo -= 1;
        fire_rate_counter = 0;
    }
    if ((*current_ammo <= 0 || (Keyboard::isKeyPressed(Keyboard::R)) && (*current_ammo_stock >= 0 && *current_ammo != current_clip_size)) || isreloading)
    {
        std::cout << gun_image_counter << std::endl;
        if (!isreloading)
        {
            gun_image_counter = 0;
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
    //std::cout << *current_ammo << " " << *current_ammo_stock << " " << current_clip_size << std::endl;
}
void Bullet_Movement_Collision(float dt)
{
    for (int i = 0; i < bullets.size(); i++)
    {
        bullets[i].shape.move(bullets[i].currentvelocity * dt);
        bullets[i].animation(dt, bullet_animation);
        for (int k = 0; k < Wall_Bounds.size(); k++)
        {
            if (bullets[i].shape.getGlobalBounds().intersects(Wall_Bounds[k]))
            {
                bullets.erase(bullets.begin() + i);
                break;
            }
        }
    }
    for (int i = 0; i < bullets.size(); i++)
    {
        for (int j = 0; j < zombies.size(); j++)
        {
            if (bullets[i].shape.getGlobalBounds().intersects(zombies[j].shape.getGlobalBounds()) && zombies[j].last_hit_bullet_id != bullets[i].id && !zombies[j].isdeath)
            {
                zombies[j].last_hit_bullet_id = bullets[i].id;
                zombies[j].health -= bullets[i].damage;
                if (Curr_Gun_state == Shotgun)
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt * 1 / 16, bullets[i].currentvelocity.y * dt * 1 / 16);
                }
                else if(Curr_Gun_state == Sniper)
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt * 10, bullets[i].currentvelocity.y * dt * 10);
                }
                else
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt, bullets[i].currentvelocity.y * dt);
                }
                if (zombies[j].health <= 0)
                {
                    int random_num = rand() % 100;
                    if (random_num > 50 && random_num < 74)
                    {

                        Sprite newhealthpack;
                        newhealthpack.setTexture(Health_Texture);
                        newhealthpack.setPosition(zombies[j].shape.getPosition());
                        HealthPacks.push_back(newhealthpack);
                        break;

                    }
                    else if (random_num > 75 && random_num < 100)
                    {
                        Sprite newammostock;
                        newammostock.setTexture(ammo_smg_photo);
                        newammostock.setPosition(zombies[j].shape.getPosition());
                        AmmoPacks.push_back(newammostock);
                        break;
                    }
                    zombies[j].isdeath = true;
                    Score += 10;
                    Money += 175;
                }
                else if (zombies[j].health > 0 && !zombies[j].isdeath)
                {
                    zombies[j].iszombiehit = true;
                    BloodEffect newbloodeffect;
                    newbloodeffect.BloodShape.setPosition(zombies[j].shape.getPosition());
                    newbloodeffect.BloodShape.setScale(zombies[j].shape.getScale().x * -0.5f, zombies[j].shape.getScale().y * 0.5f);
                    newbloodeffect.assign_random_number();
                    bloodeffects.push_back(newbloodeffect);
                }
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
        case Sniper:
            Sniper_S.setTexture(sniper_shoot_animations[gun_image_counter]);
            gun_frames = 9;
        }
    }
    else if (Keyboard::isKeyPressed(Keyboard::R))
    {
        trigger = true;
    }
    else if (isreloading && *current_ammo_stock > 0 && Curr_Gun_state)
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
        case Sniper:
            Sniper_S.setTexture(sniper_reload_animation[gun_image_counter]);
            gun_frames = 40;
            break;
        }
    }
    else
    {
        SMG_S.setTexture(SMG_Shoot_Animations[0]);
        Pistol_S.setTexture(pistol_shoot_animations[0]);
        ShotGun_S.setTexture(shotgun_shoot_animation[0]);
        Sniper_S.setTexture(sniper_shoot_animations[0]);
        gun_switch_delay_counter = current_fire_rate;
        trigger = true;
        isshooting = false;
    }
    if (isreloading)
    {
        Gun_UpdateAnimationCounter(dt, gun_frames, current_reload_time / (double)gun_frames);
    }
    else
    {
        Gun_UpdateAnimationCounter(dt, gun_frames, current_fire_rate / (double)gun_frames );
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

//zombie functions

void SpawnZombiesWaves(float dt)
{
    float multiplier = Current_Wave1 / (float)Level1NumWaves;
    SpawningZombieCounter += dt;
    if (canspawn && SpawningZombieCounter >= 1)
    {
        Zombie newzombie;
        newzombie.shape.setPosition( 50 +rand() % 1770, 50 +rand() % 930);
        newzombie.damage *= multiplier;
        newzombie.attack_fire_rate *= (1 / multiplier);
        newzombie.maxvelocity *= multiplier;
        newzombie.setspawnlocation();
        zombies.push_back(newzombie);
        TotalSpawnedZombies++;
        SpawningZombieCounter = 0;
    }
    if (TotalSpawnedZombies >= 50 * Current_Wave1)
    {
        canspawn = false;
    }
    if (zombies.size()== 0 && !canspawn && !PortalOpen )
    {
        Wave_Cooldown_counter += dt;
        if (Wave_Cooldown_counter >= Wave_Cooldown_duration)
        {
            Current_Wave1++;
            TotalSpawnedZombies = 0;
            canspawn = true;
            Wave_Cooldown_counter = 0;
        }
    }
    if (Current_Wave1 > 3 && current_level < 3)
    {
        canspawn = false;
        PortalOpen = true;
        UpdatePortalAnimation(dt);
    }
}
void HandleZombieBehaviour(float dt)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        if (zombies[i].remove_zombie)
        {
            zombies.erase(zombies.begin() + i);
            break;
        }
        zombies[i].Zombie_Behaviour1(Player.getPosition(), dt,zombie_walk_animation,zombie_atk_animation,zombie_hit_animation,zombie_death_animation,Zombie_spawn_animation);
    }
    for (int i = 0; i < bloodeffects.size(); i++)
    {
        if (bloodeffects[i].DeleteMe)
        {
            bloodeffects.erase(bloodeffects.begin() + i);
            break;
        }
        bloodeffects[i].HandleBloodEffect(zombie_hit_blood_effect, dt);
    }
    for (int i = 0; i < zombies.size(); i++)
    {
        if (zombies[i].isdeath)
        {
            continue;
        }
        for (int j = 0; j < Wall_Bounds.size(); j++)
        {
            FloatRect current_zombie_Bound = zombies[i].shape.getGlobalBounds();
            FloatRect intersection;
            FloatRect Wall_bound = Wall_Bounds[j];
            if (current_zombie_Bound.intersects(Wall_bound))
            {
                current_zombie_Bound.intersects(Wall_bound, intersection);
                if (intersection.width < intersection.height)
                {
                    if (current_zombie_Bound.left < Wall_bound.left)
                    {
                        zombies[i].shape.move(-1500 * dt, 0);
                    }
                    else
                    {
                        zombies[i].shape.move(1500 * dt, 0);
                    }
                }
                else
                {
                    if (current_zombie_Bound.top < Wall_bound.top)
                    {
                        zombies[i].shape.move(0, -1500 * dt);
                    }
                    else
                    {
                        zombies[i].shape.move(0, 1500 * dt);
                    }
                }
            }
        }
        for (int j = 0; j < zombies.size(); j++)
        {
            if (i == j)
            {
                continue;
            }
            FloatRect Current_Zombie_Bound = zombies[i].shape.getGlobalBounds();
            FloatRect intersection;
            FloatRect Current_other_zombie_bound = zombies[j].shape.getGlobalBounds();
            if (Current_Zombie_Bound.intersects(Current_other_zombie_bound))
            {

                Current_Zombie_Bound.intersects(Current_other_zombie_bound, intersection);
                if (intersection.width < intersection.height)
                {
                    if (Current_Zombie_Bound.left < Current_other_zombie_bound.left)
                    {
                        zombies[i].shape.move(-50 * dt, 0);
                    }
                    else
                    {
                        zombies[i].shape.move(50 * dt, 0);
                    }
                }
                else
                {
                    if (Current_Zombie_Bound.top < Current_other_zombie_bound.top)
                    {
                        zombies[i].shape.move(0, -50 * dt);
                    }
                    else
                    {
                        zombies[i].shape.move(0, 50 * dt);
                    }
                }
            }
        }
    }
  
}

void SwtichCurrentWallBounds()
{
    switch (current_level)
    {
    case 1:
        for (int i = 0; i < Wall_Bounds1.size(); i++)
        {
            Wall_Bounds.push_back(Wall_Bounds1[i]);
        }
        break;
    case 2:
        for (int i = 0; i < Wall_Bounds2.size(); i++)
        {
            Wall_Bounds.push_back(Wall_Bounds2[i]);
        }
        break;
    case 3:
        for (int i = 0; i < Wall_Bounds3.size(); i++)
        {
            Wall_Bounds.push_back(Wall_Bounds3[i]);
        }
        break;
    }
}


void background()
{
    Tbackground.loadFromFile("LevelAssets/Ground_1.jpg");

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            background1[j][i].setTexture(Tbackground);
            background1[j][i].setPosition(i * 200, j * 200);
        }
    }
}

void wall()
{
    VwallT1.loadFromFile("LevelAssets/Vwall1.png");
    HwallT1.loadFromFile("LevelAssets/Hwall1.png");
    for (int i = 0; i < 15; i++)
    {
        Vwall1[i].setTexture(VwallT1);
    }
    for (int i = 0; i < 15; i++)
    {
        Hwall1[i].setTexture(HwallT1);
    }

    //upper wall

    for (int i = 0; i < 5; i++)
    {
        Hwall1[i].setPosition(405 * i, 0);
        Wall_Bounds1.push_back(Hwall1[i].getGlobalBounds());
    }
    //lower wall

    for (int i = 5; i < 10; i++)
    {
        Hwall1[i].setPosition(405 * (i - 5), 1020);
        Wall_Bounds1.push_back(Hwall1[i].getGlobalBounds());
    }
    //left wall

    for (int i = 0; i < 4; i++)
    {
        Vwall1[i].setPosition(0, (405 * i) + 40);
        Wall_Bounds1.push_back(Vwall1[i].getGlobalBounds());
    }
    //Right wall

    for (int i = 4; i < 8; i++)
    {
        Vwall1[i].setPosition(1890, (405 * (i - 4)) + 40);
        Wall_Bounds1.push_back(Vwall1[i].getGlobalBounds());
    }

    //mid walls
    for (int i = 10; i < 14; i++)
    {
        Hwall1[i].setPosition((405 * (i - 10)) + 30, 550);
        Wall_Bounds1.push_back(Hwall1[i].getGlobalBounds());
    }
    Vwall1[8].setTextureRect(IntRect(0, 0, 30, 275));
    Vwall1[8].setPosition(1050, 575);
    Vwall1[9].setTextureRect(IntRect(0, 0, 30, 305));
    Vwall1[9].setPosition(800, 245);
    Wall_Bounds1.push_back(Vwall1[8].getGlobalBounds());
    Wall_Bounds1.push_back(Vwall1[9].getGlobalBounds());
}

void block()
{
    ME.loadFromFile("LevelAssets/ME1.png");
    for (int i = 0; i < 7; i++)
    {
        ME1[i].setTexture(ME);
    }
    ME1[0].setPosition(1878, 990);
    ME1[1].setPosition(0, 990);
    ME1[2].setPosition(0, 515);
    ME1[3].setPosition(792, 515);
    ME1[4].setPosition(1042, 518);//
    ME1[5].setPosition(0, -20);//
    ME1[6].setPosition(1878, -20);//

    //door
    TDoor.loadFromFile("LevelAssets/Box.png");
    for (int i = 1; i < 17; i++)
    {
        Door[i].setTexture(TDoor);
    }
    for (int i = 0; i < 6; i++)
    {
        Door[i].setPosition(760, -30 + (35 * i));
    }
    for (int i = 6; i < 11; i++)
    {
        Door[i].setPosition(1630 + (46 * (i - 6)), 500);
    }
    for (int i = 11; i < 17; i++)
    {
        Door[i].setPosition(1020, 775 + (35 * (i - 11)));
    }
}

//level 2 functions

void background2()
{
    Tbackground2.loadFromFile("LevelAssets/Ground_2.png");
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Background2[j][i].setTexture(Tbackground2);
            Background2[j][i].setColor(Color(237, 174, 161));
            Background2[j][i].setPosition(i * 298, j * 298);
        }
    }
}

void wall2()
{
    HwallT.loadFromFile("LevelAssets/Hwall2.png");
    VwallT.loadFromFile("LevelAssets/Vwall2.png");
    for (int i = 0; i < 15; i++)
    {
        Hwall[i].setTexture(HwallT);
    }
    for (int i = 0; i < 15; i++)
    {
        Vwall[i].setTexture(VwallT);
    }
    //upper wall
    for (int i = 0; i < 5; i++)
    {
        Hwall[i].setPosition(i * 410, 0);
        Wall_Bounds2.push_back(Hwall[i].getGlobalBounds());
    };

    //lowwer wall
    for (int i = 5; i < 10; i++)
    {
        Hwall[i].setPosition((i - 5) * 410, 1023);
        Wall_Bounds2.push_back(Hwall[i].getGlobalBounds());
    };

    //left wall
    for (int i = 0; i < 3; i++)
    {
        Vwall[i].setPosition(0, i * 347);
        Wall_Bounds2.push_back(Vwall[i].getGlobalBounds());
    };

    //right wall
    for (int i = 3; i < 6; i++)
    {
        Vwall[i].setPosition(1892, (i - 3) * 347);
        Wall_Bounds2.push_back(Vwall[i].getGlobalBounds());
    };

    //mid walls

    //up
    Vwall[6].setPosition(615, 55);
    Vwall[7].setPosition(615, 35);
    Vwall[8].setPosition(1300, 55);
    Vwall[9].setPosition(1300, 35);
    Wall_Bounds2.push_back(Vwall[6].getGlobalBounds());
    Wall_Bounds2.push_back(Vwall[7].getGlobalBounds());
    Wall_Bounds2.push_back(Vwall[8].getGlobalBounds());
    Wall_Bounds2.push_back(Vwall[9].getGlobalBounds());

    //down
    Vwall[10].setPosition(615, 680);
    Vwall[11].setPosition(1300, 680);
    Wall_Bounds2.push_back(Vwall[10].getGlobalBounds());
    Wall_Bounds2.push_back(Vwall[11].getGlobalBounds());

}

void block2()
{
    TME2.loadFromFile("LevelAssets/ME2.png");
    for (int i = 0; i < 8; i++)
    {
        ME2[i].setTexture(TME2);
    }
    ME2[0].setPosition(-10, -30);//
    ME2[1].setPosition(-10, 980);//
    ME2[2].setPosition(1880, -30);//
    ME2[3].setPosition(1880, 980);//
    ME2[4].setPosition(605, 980);
    ME2[5].setPosition(1290, 980);
    ME2[6].setPosition(605, -30);
    ME2[7].setPosition(1290, -30);



    TDoor.loadFromFile("LevelAssets/Box.png");
    for (int i = 1; i < 17; i++)
    {
        Door[i].setTexture(TDoor);
    }

    //left door
    for (int i = 0; i < 8; i++)
    {
        Door[i].setPosition(590, 330 + (35 * i));
    }

    //right door
    for (int i = 8; i < 15; i++)
    {
        Door[i].setPosition(1273, 80 + (35 * i));
    }
}



//level 3 functions
void background3()
{
    Tbackground3.loadFromFile("LevelAssets/Ground_3.png");
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Background3[j][i].setTexture(Tbackground3);
        }
    }

    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Background3[j][i].setPosition(i * 314, j * 314);
        }

    }
}

void wall3()
{
    HwallT3.loadFromFile("LevelAssets/Hwall3.png");
    VwallT3.loadFromFile("LevelAssets/Vwall3.png");
    // set texture
    for (int i = 0; i < 14; i++)
    {
        Hwall3[i].setTexture(HwallT3);
    }
    for (int i = 0; i < 18; i++)
    {
        Vwall3[i].setTexture(VwallT3);
    }

    //upper wall
    for (int i = 0; i < 5; i++)
    {
        Hwall3[i].setPosition(i * 410, -30);
        Wall_Bounds3.push_back(Hwall3[i].getGlobalBounds());
    };

    //lowwer wall
    for (int i = 5; i < 10; i++)
    {
        Hwall3[i].setPosition((i - 5) * 410, 990);
        Wall_Bounds3.push_back(Hwall3[i].getGlobalBounds());
    };

    //Right wall
    for (int i = 0; i < 7; i++)
    {
        Vwall3[i].setPosition(0, i * 150);
        Wall_Bounds3.push_back(Vwall3[i].getGlobalBounds());
    };

    //left wall
    for (int i = 7; i < 14; i++)
    {
        Vwall3[i].setPosition(1887, (i - 7) * 150);
        Wall_Bounds3.push_back(Vwall3[i].getGlobalBounds());
    }

    //mid walls
    Hwall3[10].setPosition(0, 500);
    Hwall3[11].setPosition(410, 500);
    Hwall3[12].setPosition(1090, 500);
    Hwall3[13].setPosition(1500, 500);

    Wall_Bounds3.push_back(Hwall3[10].getGlobalBounds());
    Wall_Bounds3.push_back(Hwall3[11].getGlobalBounds());
    Wall_Bounds3.push_back(Hwall3[12].getGlobalBounds());
    Wall_Bounds3.push_back(Hwall3[13].getGlobalBounds());

    Vwall3[14].setPosition(1090, 500);
    Vwall3[15].setPosition(1090, 650);
    Vwall3[16].setPosition(783, 0);
    Vwall3[17].setPosition(783, 150);

    Wall_Bounds3.push_back(Vwall3[14].getGlobalBounds());
    Wall_Bounds3.push_back(Vwall3[15].getGlobalBounds());
    Wall_Bounds3.push_back(Vwall3[16].getGlobalBounds());
    Wall_Bounds3.push_back(Vwall3[17].getGlobalBounds());
}

void block3()
{
    TDoor.loadFromFile("LevelAssets/Box.png");

    for (int i = 1; i < 17; i++)
    {
        Door[i].setTexture(TDoor);
    }
    //upper door
    for (int i = 1; i < 6; i++)
    {
        Door[i].setPosition(750, 265 + (35 * i));
    };
    //mid door
    for (int i = 6; i < 12; i++)
    {
        Door[i].setPosition(515 + (46 * i), 485);
    };
    //lowwer door
    for (int i = 12; i < 17; i++)
    {
        Door[i].setPosition(1060, 360 + (35 * i));
    };

}


//drawing function
void Draw()
{
    window.clear();

    if (Norm_dir_vector.x > 0)
    {
        Player.setScale(2, 2);
        Gun.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 20.f);
        Gun.setScale(0.5, 0.5);
        Crosshair.setPosition(Gun.getPosition().x  + cos(Gun.getRotation() / 180 * pi) * 75, Gun.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 75);        
    }
    else
    {
        Player.setScale(-2, 2);
        Gun.setPosition(Player.getPosition().x - 15, Player.getPosition().y + 20.f);
        Gun.setScale(0.5, -0.5);
        Crosshair.setPosition(Gun.getPosition().x + 50 + cos(Gun.getRotation() / 180 * pi) * 75, Gun.getPosition().y+ sin(Gun.getRotation() / 180 * pi) * 75);
    }
    test.setPosition(Gun.getPosition().x - 20 + cos(Gun.getRotation() / 180 * pi) * 75, Gun.getPosition().y - 2 + sin(Gun.getRotation() / 180 * pi) * 75);
    switch (current_level)
    {
    case 1:
        Portal_S.setPosition(580, 790);
        speedmachine.setPosition(Vector2f(35, 25));
        reloadmachine.setPosition(Vector2f(1080, 580));
        pistol_buying.setPosition(Vector2f(150, 95)); 
        smg_buying.setPosition(Vector2f(860, 415)); 
        shotgun_buying.setPosition(Vector2f(1850, 790));
        sniper_buying.setPosition(Vector2f(500, 500));
        //upper wall

        for (int i = 0; i < 5; i++)
        {
            Hwall1[i].setPosition(405 * i, 0);
        }
        //lower wall

        for (int i = 5; i < 10; i++)
        {
            Hwall1[i].setPosition(405 * (i - 5), 1020);
        }
        //left wall

        for (int i = 0; i < 4; i++)
        {
            Vwall1[i].setPosition(0, (405 * i) + 40);
        }
        //Right wall

        for (int i = 4; i < 8; i++)
        {
            Vwall1[i].setPosition(1890, (405 * (i - 4)) + 40);
        }

        //mid walls
        for (int i = 10; i < 14; i++)
        {
            Hwall1[i].setPosition((405 * (i - 10)) + 30, 550);
        }
        Vwall1[8].setPosition(1050, 575);
        Vwall1[9].setPosition(800, 245);

        ME1[0].setPosition(1878, 990);
        ME1[1].setPosition(0, 990);
        ME1[2].setPosition(0, 515);
        ME1[3].setPosition(792, 515);
        ME1[4].setPosition(1042, 518);//
        ME1[5].setPosition(0, -20);//
        ME1[6].setPosition(1878, -20);//

        for (int i = 0; i < 6; i++)
        {
            Door[i].setPosition(760, -30 + (35 * i));
        }
        for (int i = 6; i < 11; i++)
        {
            Door[i].setPosition(1630 + (46 * (i - 6)), 500);
        }
        for (int i = 11; i < 17; i++)
        {
            Door[i].setPosition(1020, 775 + (35 * (i - 11)));
        }

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 7; j++)
            {
                window.draw(background1[j][i]);
            }
        }
        //upper wall
        for (int i = 0; i < 5; i++)
        {
            window.draw(Hwall1[i]);
        }
        //mid wall
        for (int i = 10; i < 14; i++)
        {
            window.draw(Hwall1[i]);
        }
        for (int i = 4; i < 7; i++)
        {
            window.draw(ME1[i]);
        }
        for (int i = 0; i < 10; i++)
        {
            window.draw(Vwall1[i]);
        }
        for (int i = 11; i < 17; i++)
        {
            window.draw(Door[i]);
        }
        //lower wall
        for (int i = 5; i < 10; i++)
        {
            window.draw(Hwall1[i]);
        }
        for (int i = 0; i < 11; i++)
        {
            window.draw(Door[i]);
        }
        window.draw(Vwall1[9]);
        for (int i = 0; i < 4; i++)
        {
            window.draw(ME1[i]);
        }
        break;
    case 2:
        Portal_S.setPosition(50, 50);
        //upper wall
        for (int i = 0; i < 5; i++)
        {
            Hwall[i].setPosition(i * 410, 0);
        };

        //lowwer wall
        for (int i = 5; i < 10; i++)
        {
            Hwall[i].setPosition((i - 5) * 410, 1023);
        };

        //left wall
        for (int i = 0; i < 3; i++)
        {
            Vwall[i].setPosition(0, i * 347);
        };

        //right wall
        for (int i = 3; i < 6; i++)
        {
            Vwall[i].setPosition(1892, (i - 3) * 347);
        };

        //mid walls

        //up
        Vwall[6].setPosition(615, 55);
        Vwall[7].setPosition(615, 35);
        Vwall[8].setPosition(1300, 55);
        Vwall[9].setPosition(1300, 35);
        //down
        Vwall[10].setPosition(615, 680);
        Vwall[11].setPosition(1300, 680);

        ME2[0].setPosition(-10, -30);//
        ME2[1].setPosition(-10, 980);//
        ME2[2].setPosition(1880, -30);//
        ME2[3].setPosition(1880, 980);//
        ME2[4].setPosition(605, 980);
        ME2[5].setPosition(1290, 980);
        ME2[6].setPosition(605, -30);
        ME2[7].setPosition(1290, -30);

        //left door
        for (int i = 0; i < 8; i++)
        {
            Door[i].setPosition(590, 330 + (35 * i));
        }

        //right door
        for (int i = 8; i < 15; i++)
        {
            Door[i].setPosition(1273, 80 + (35 * i));
        }

        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                window.draw(Background2[j][i]);
            }
        }
        for (int i = 0; i < 15; i++)
        {
            window.draw(Hwall[i]);
        }
        window.draw(ME2[6]);
        window.draw(ME2[7]);
        for (int i = 0; i < 10; i++)
        {
            window.draw(Vwall[i]);
        }
        for (int i = 0; i < 15; i++)
        {
            window.draw(Door[i]);
        }
        window.draw(Vwall[10]);
        window.draw(Vwall[11]);
        for (int i = 0; i < 6; i++)
        {
            window.draw(ME2[i]);
        }
        break;
    case 3:
        Portal_S.setPosition(50, 50);
        //upper wall
        for (int i = 0; i < 5; i++)
        {
            Hwall3[i].setPosition(i * 410, -30);
        };

        //lowwer wall
        for (int i = 5; i < 10; i++)
        {
            Hwall3[i].setPosition((i - 5) * 410, 990);
        };

        //Right wall
        for (int i = 0; i < 7; i++)
        {
            Vwall3[i].setPosition(0, i * 150);
        };

        //left wall
        for (int i = 7; i < 14; i++)
        {
            Vwall3[i].setPosition(1887, (i - 7) * 150);
        }

        //mid walls
        Hwall3[10].setPosition(0, 500);
        Hwall3[11].setPosition(410, 500);
        Hwall3[12].setPosition(1090, 500);
        Hwall3[13].setPosition(1500, 500);
        Vwall3[14].setPosition(1090, 500);
        Vwall3[15].setPosition(1090, 650);
        Vwall3[16].setPosition(783, 0);
        Vwall3[17].setPosition(783, 150);


        //upper door
        for (int i = 1; i < 6; i++)
        {
            Door[i].setPosition(750, 265 + (35 * i));
        };
        //mid door
        for (int i = 6; i < 12; i++)
        {
            Door[i].setPosition(515 + (46 * i), 485);
        };
        //lowwer door
        for (int i = 12; i < 17; i++)
        {
            Door[i].setPosition(1060, 360 + (35 * i));
        };


        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                window.draw(Background3[j][i]);
            }
        }
        for (int i = 0; i < 18; i++)
        {
            window.draw(Vwall3[i]);
        }
        for (int i = 0; i < 17; i++)
        {
            window.draw(Door[i]);
        }
        for (int i = 0; i < 14; i++)
        {
            window.draw(Hwall3[i]);
        }
        break;
    }
    void1[0].setPosition(0, 1080);
    void1[1].setPosition(0, -2000);
    void1[2].setPosition(1920, 0);
    void1[3].setPosition(-2864, 0);
    for (int i = 0; i < 4; i++)
    {
        window.draw(void1[i]);
    }
    window.draw(speedmachine);
    window.draw(reloadmachine);
    Portal_S.setOrigin(Portal_S.getLocalBounds().width / 2, Portal_S.getLocalBounds().height / 2);
    if (PortalOpen)
    {
        window.draw(Portal_S);
    }
    for (int i = 0; i < bloodeffects.size(); i++)
    {
        bloodeffects[i].BloodShape.setOrigin(bloodeffects[i].BloodShape.getLocalBounds().width / 2, bloodeffects[i].BloodShape.getLocalBounds().height / 2);
        window.draw(bloodeffects[i].BloodShape);
    }
    window.draw(Player);
    for (int i = 0; i < bullets.size(); i++)
    {
        window.draw(bullets[i].shape);
    }
    for (int i = 0; i < zombies.size(); i++)
    {
        zombies[i].SpawnEffect.setOrigin(zombies[i].SpawnEffect.getLocalBounds().width / 2, zombies[i].SpawnEffect.getLocalBounds().height / 2);      
        window.draw(zombies[i].SpawnEffect);
        zombies[i].shape.setScale(2,2);
        zombies[i].shape.setOrigin(zombies[i].shape.getLocalBounds().width / 2, zombies[i].shape.getLocalBounds().height / 2);
        if(zombies[i].currentvelocity.x < 0)
        {
            zombies[i].shape.setScale(zombies[i].shape.getScale().x * -1, zombies[i].shape.getScale().y);
        }
        else
        {
            zombies[i].shape.setScale(zombies[i].shape.getScale().x, zombies[i].shape.getScale().y);
        }
        window.draw(zombies[i].shape);
    }
    if (isdashing)
    {
        draw_dash_line();
    }
    else
    {
        dasheline.clear();
    }
    if (curr_state == state::death)
    {
        Player.setScale(6, 6);
    }
    
    switch (Curr_Gun_state)
    {
    case Pistol:
        Pistol_S.setScale(Gun.getScale().x * 3/4, Gun.getScale().y * 3 / 4);
        Pistol_S.setPosition(Gun.getPosition());
        Pistol_S.setRotation(Gun.getRotation());
        window.draw(Pistol_S);
        break;
    case Smg:
        SMG_S.setScale(Gun.getScale().x * 3 / 4, Gun.getScale().y * 3 / 4);
        SMG_S.setPosition(Gun.getPosition());
        SMG_S.setRotation(Gun.getRotation());
        window.draw(SMG_S);
        break;
    case Shotgun:
        ShotGun_S.setScale(Gun.getScale().x * 3 / 4, Gun.getScale().y * 3 / 4);
        ShotGun_S.setPosition(Gun.getPosition());
        ShotGun_S.setRotation(Gun.getRotation());
        window.draw(ShotGun_S);
        break;
    case Sniper:
        Sniper_S.setScale(Gun.getScale().x * 3 / 4, Gun.getScale().y * 3 / 4);
        Sniper_S.setPosition(Gun.getPosition());
        Sniper_S.setRotation(Gun.getRotation());
        window.draw(Sniper_S);
        break;
    }
    Crosshair.setPosition(MousePos);
    for (int i = 0; i < HealthPacks.size(); i++)
    {
        window.draw(HealthPacks[i]);
    }
    for (int i = 0; i < AmmoPacks.size(); i++)
    {
        window.draw(AmmoPacks[i]);
    }
    // tamer 
    /*{  to draw score and coins title }*/
    /*{end   to draw score and coins title }*/
    /* to draw guns */
    /*pistol*/
    pistol_photo.loadFromFile("pistol.png");
    // tamer
    //{ health bar } 


    RectangleShape health_bar(Vector2f(140, 15));
    health_bar.setScale(Vector2f((Player_Health / 100.0) * 2, 2));
    health_bar.setPosition(window.mapPixelToCoords(Vector2i(60, 20)));
    RectangleShape missing_health_bar(Vector2f(140, 15));
    missing_health_bar.setFillColor(Color::White);
    missing_health_bar.setScale(Vector2f(2, 2));
    missing_health_bar.setPosition(window.mapPixelToCoords(Vector2i(60, 20)));
    RectangleShape health_bar_background(Vector2f(145 * 2, 20 * 2));
    health_bar_background.setFillColor(Color::Black);
    health_bar_background.setPosition(health_bar.getPosition().x - 5 , health_bar.getPosition().y - 5);
    window.draw(missing_health_bar);
    if (Player_Health > 0)
    {
        window.draw(health_bar_background);
        window.draw(missing_health_bar);
    }
    if (Player_Health >= 30)
    {
        health_bar.setFillColor(Color(136, 8, 8));
    }
    else
    {
        health_bar.setFillColor(Color(136, 8, 8));
    }
    if (Player_Health > 0)
    window.draw(health_bar);
    //  to  draw heart next to health bar 
    full_heart.setTexture(full_heart_photo);
    full_heart.setScale(Vector2f(0.15, 0.15));
    full_heart.setPosition(window.mapPixelToCoords(Vector2i(0, 0)));
    if (Player_Health > 0)
    //window.draw(full_heart);
    // to print health percent text 
    health_precent_text.setFont(normal_font); 
    health_precent_text.setScale(2, 2);
    health_precent_text.setString(to_string(Player_Health));
    health_precent_text.setCharacterSize(12);
    health_precent_text.setFillColor(Color(128,128,128));
    health_precent_text.setPosition(health_bar.getPosition().x + 120, health_bar.getPosition().y);
    if (Player_Health >= 0)
        window.draw(health_precent_text);
    // to print  { % }
   
    precent_sign.setFont(normal_font); 
    precent_sign.setScale(2, 2);
    precent_sign.setString(" % ");
    precent_sign.setCharacterSize(12);
    precent_sign.setFillColor(Color(128, 128, 128));
    precent_sign.setPosition(health_bar.getPosition().x + 150, health_bar.getPosition().y);
    if (Player_Health >= 0)
        window.draw(precent_sign);
    //to draw money photo
    money.setTexture(money_photo);
    money.setScale(Vector2f(0.07, 0.07));
    money.setPosition(window.mapPixelToCoords(Vector2i(0, 885)));
    window.draw(money);
    //{  to draw score and coins title and health percent text  }

    //score

    score_text.setFont(blood_font); // select the font 
    score_text.setString("Score : " + to_string(Score));
    score_text.setCharacterSize(36);
    score_text.setFillColor(Color(136, 8, 8));
    score_text.setPosition(window.mapPixelToCoords(Vector2i(0,980 )));
    window.draw(score_text);

    // to print money number

    money_text.setFont(normal_font); // select the font 
    money_text.setString(" : " + to_string(Money));
    money_text.setCharacterSize(36);
    money_text.setFillColor(Color(255, 215, 0));
    money_text.setPosition(window.mapPixelToCoords(Vector2i(80, 890)));
    window.draw(money_text);
    // to print  current wave  

    current_wave.setFont(blood_font);
    current_wave.setScale(1.5, 1.5);
    current_wave.setString(" Current wave \n\t\t    " + to_string(Current_Wave1));
    current_wave.setCharacterSize(19);
    current_wave.setFillColor(Color(136, 8, 8));
    current_wave.setPosition(window.mapPixelToCoords(Vector2i(800, 0)));
    window.draw(current_wave);
    // amo and amo stack

    current_ammo_text.setFont(normal_font);
    current_ammo_text.setString(to_string(*current_ammo));
    current_ammo_text.setCharacterSize(22);
    current_ammo_text.setFillColor(Color::White);
    if (Curr_Gun_state == Gun_State::Smg)
    {
        current_ammo_text.setPosition(window.mapPixelToCoords(Vector2i(1233, 577)));
    }
    else

    {
        current_ammo_text.setPosition(window.mapPixelToCoords(Vector2i(1250, 577)));
    }
    window.draw(current_ammo_text);

    //{to print  amo stock text }

    ammo_stock_text.setFont(normal_font);
    ammo_stock_text.setString(" /" + to_string(*current_ammo_stock));
    ammo_stock_text.setPosition(window.mapPixelToCoords(Vector2i(1270, 587)));
    ammo_stock_text.setCharacterSize(12);
    ammo_stock_text.setFillColor(Color::White);

    window.draw(ammo_stock_text);
    // to draw ammo next to text 
    if (Curr_Gun_state == Gun_State::Pistol)
    {     
        ammo_pistol.setTexture(ammo_pistol_photo);
        ammo_pistol.setPosition(window.mapPixelToCoords(Vector2i(1320, 577)));
        window.draw(ammo_pistol);
    }
    else if (Curr_Gun_state == Gun_State::Smg)
    {
        
        ammo_smg.setTexture(ammo_smg_photo);
        ammo_smg.setPosition(window.mapPixelToCoords(Vector2i(1320, 577)));
        window.draw(ammo_smg);
    }
    else if (Curr_Gun_state == Gun_State::Shotgun)
    {
        
        ammo_shotgun.setTexture(ammo_shotgun_photo);
        ammo_shotgun.setPosition(window.mapPixelToCoords(Vector2i(1320, 577)));
        window.draw(ammo_shotgun);
    }
    /* {end drawing ui }
     to draw guns 
     pistol*/
    pistol_buying.setTexture(pistol_photo);
    if (!sniper_buy)
    {
        window.draw(sniper_buying);
    }
    if (!pistol_buy)
    window.draw(pistol_buying);
    //smg
    smg_buying.setTexture(smg_photo);
    
    if (!smg_buy)
        window.draw(smg_buying);
    // shotgun 
    shotgun_buying.setTexture(shotgun_photo);
    
    if (!shotgun_buy)
        window.draw(shotgun_buying);
    //end draw guns 

    //vandingmachine
    speedmachine.setScale(0.5, 0.5);
    reloadmachine.setScale(0.5, 0.5);
    window.draw(Crosshair);



    window.display();
}

void buying_weapons()
{
    if (Player.getGlobalBounds().intersects(pistol_buying.getGlobalBounds()) && Money >= money_pistol && !pistol_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_pistol;
        pistol_buy = true;
        Score += 100;
    }
    if (Player.getGlobalBounds().intersects(smg_buying.getGlobalBounds()) && Money >= money_smg && !smg_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_smg;
        smg_buy = true;
        Score += 100;
    }
    if (Player.getGlobalBounds().intersects(shotgun_buying.getGlobalBounds()) && Money >= money_shotgun && !shotgun_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_shotgun;
        shotgun_buy = true;
        Score += 100;
    }
    if (Player.getGlobalBounds().intersects(sniper_buying.getGlobalBounds()) && Money >= 2000 && !sniper_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= 2000;
        sniper_buy = true;
        Score += 100;
    }
}