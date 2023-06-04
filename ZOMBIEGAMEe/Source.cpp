#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <SFML/Audio.hpp>
#include "Candle/LightSource.hpp"
#include "Candle/RadialLight.hpp"
#include "Candle/LightingArea.hpp"


using namespace std;
//to be displayed on ui
int Player_Health = 100;
int Score= 0;
int highest_score = 0;
int Money= 0;
int const speedmoney = 2000;
int const reloadmoney = 2000;
float speedmulti = 1;
float reloadmulti = 1;

int current_level = 1;

//buying weapon
bool  smg_buy = false, shotgun_buy = false, sniper_buy = false, speed_pow = false, reload_pow = false;
int const  money_smg = 1000, money_shotgun = 1500, money_sniper = 2000;
//to cheak player intersects
bool smg_player_intersects = false, sniper_player_intersects = false;
bool shotgun_player_intersects = false;

#define pi 3.14159265
#define Level1NumWaves 3

#define pistolfirerate 0.2f
#define pistoldamage 6
#define pistolspread 0
#define pistolbulletpershot 1
#define pistolclipsize 9
#define pistolreloadtime 1
#define Pistol_bullets_loaded_per_reload pistolclipsize
int pistolammostock = 36;
int pistolbulletsloaded = pistolclipsize;

#define riflefirerate 0.08f
#define rifledamage 4.5
#define riflespread 0
#define riflebulletpershot 1
#define rifleclipsize 30
#define riflereloadtime 1
#define Rifle_bullets_loaded_per_reload rifleclipsize
int rifleammostock = 90;
int riflebulletsloaded = rifleclipsize;

#define shotgunfirerate 1
#define shotgundamage 1.5
#define shotgunspread 1
#define shotgunbulletpershot 10
#define shotgunclipsize 8
#define shotgunreloadtime 1
#define ShotGun_bullets_loaded_per_reload 1
int shotgunammostock = 24;
int shotgunbulletsloaded = shotgunclipsize;

#define sniperfirerate 1.6f
#define sniperdamage 250
#define sniperspread 0
#define sniperbulletpershot 1
#define sniperclipsize 5
#define sniperreloadtime 2
#define sniper_bullets_loaded_per_reload sniperclipsize
int sniperammostock = 15;
int sniperbulletsloaded = sniperclipsize;

#define minigunfirerate 0.05
#define minigundamage 3
#define minigunspread 2
#define minigunbulletpershot 3
#define minigunclipsize 2000
#define minigunreloadtime 0
#define minigun_bullets_loaded_per_reload minigunclipsize
int minigunammostock = 99999999;
int minigunbulletsloaded = minigunclipsize;

using namespace sf;
using namespace candle;

enum state
{
    idle, walk, hit, death
};
state curr_state = state::idle;
enum Gun_State
{
    Pistol, Smg, Shotgun,Sniper,MiniGun
};
Gun_State Curr_Gun_state = Gun_State::Pistol;
struct Trail
{
    vector<Vector2f> positions;
    int maxlength;
    float thickness;
    Color color;
    void addTrailSegment(Vector2f position) {
        positions.push_back(position);
        while (positions.size() > maxlength) {
            positions.erase(positions.begin());
        }
    }
    void drawTrail(sf::RenderWindow& window) {
        for (int i = 1; i < positions.size(); i++) {
            sf::Vector2f segment = positions[i] - positions[i - 1];
            float segmentLength = std::sqrt(segment.x * segment.x + segment.y * segment.y);
            sf::RectangleShape trailSegment(sf::Vector2f(segmentLength, thickness));
            trailSegment.setPosition(positions[i - 1]);
            trailSegment.setRotation(std::atan2(segment.y, segment.x) * 180 / 3.14159265f);
            trailSegment.setFillColor(sf::Color(color.r, color.g, color.b, color.a * (1.0f - i / (float)positions.size())));
            window.draw(trailSegment);
        }
    }
};
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
    RadialLight lighteffect;
    Gun_State curr_gun_state;
    Trail guntrail;
    int zombieshit = 0;
    void animation(float dt,Texture bulletanimation[])
    {
        guntrail.addTrailSegment(shape.getPosition());
        if (curr_gun_state == Shotgun || curr_gun_state == MiniGun)
        {
            lighteffect.setIntensity(500);
            lighteffect.setRange(200);
        }
        else
        {
            lighteffect.setIntensity(200);
            lighteffect.setRange(200);
        }
        lighteffect.setColor(Color::Yellow);
        lighteffect.setPosition(shape.getPosition());
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
struct Enemy
{
    Sprite shape;
    Sprite SpawnEffect;
    RadialLight SpawnLight;
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
    int Main_max_images;

    float Death_animation_counter = 0;
    float Death_animation_duration = 0.25f;
    int Death_imagecounter = 0;
    int Death_max_images;

    float Hit_animation_counter = 0;
    float Hit_animation_duration = 0.20f;
    int Hit_imagecounter = 0;
    int Hit_max_images;

    float Spawn_animation_counter = 0;
    float Spawn_animation_duration = 0.15f;
    int Spawn_imagecounter = 0;

    int numberofframes = 8;
    int health = 75;
    int current_frames = 7;
    float distance_from_player;
    bool isdeath = false;
    bool isEnemyhit = false;
    bool remove_Enemy = false;
    bool explode = false;

    int Enemytype = 0;

    Vector2f norm_Direction;
    void setspawnlocation()
    {
        SpawnEffect.setPosition(shape.getPosition());
        SpawnEffect.setScale(2, 2);
        SpawnLight.setPosition(shape.getPosition());
        SpawnLight.setRange(200);
        SpawnLight.setIntensity(150);
        SpawnLight.setColor(Color(252, 159, 159));
        switch (Enemytype)
        {
        case 0: Main_max_images = 8; Hit_max_images = 2; Death_max_images = 5; break;
        case 1:Main_max_images = 8; break;
        }
    }
    void EnemyUpdate(Vector2f player_pos, float dt, Texture walk_anim[], Texture atk_anim[], Texture hit_anim[], Texture death_anim[], Texture Spawn_anim[], bool dashing)
    {
        if (shape.getPosition().x > 1920)
        {
            shape.setPosition(1900, shape.getPosition().y);
        }
        if (shape.getPosition().y > 1080)
        {
            shape.setPosition(shape.getPosition().x, 1000);
        }
        if (shape.getPosition().x < 0)
        {
            shape.setPosition(50, shape.getPosition().y);
        }
        if (shape.getPosition().y < 0)
        {
            shape.setPosition(shape.getPosition().x, 50);
        }
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
        if (isEnemyhit && !isdeath)
        {
            Hit_animation_counter += dt;
            if (Hit_animation_counter >= Hit_animation_duration)
            {
                Hit_animation_counter = 0;
                Hit_imagecounter++;
                if (Hit_imagecounter >= Hit_max_images)
                {
                    isEnemyhit = false;
                    Hit_imagecounter = 0;
                }
            }
        }
        else if (!isdeath && !isEnemyhit)
        {
            animation_counter += dt;
            if (animation_counter >= animation_duration)
            {
                animation_counter = 0;
                imagecounter++;
                if (imagecounter >= Main_max_images)
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
            norm_Direction = Direction / magnitude;
            distance_from_player = magnitude;          
        }
        else if (isdeath)
        {
            Death_animation_counter += dt;
            if (Death_animation_counter >= Death_animation_duration)
            {
                if (Death_imagecounter < Death_max_images)
                {
                    Death_imagecounter++;
                }
                if (Death_animation_counter >= Death_max_images)
                {
                    remove_Enemy = true;
                }
            }
        }
        switch (Enemytype)
        {
        case 0: Zombie_Behaviour1(dt, walk_anim, atk_anim, hit_anim, death_anim, Spawn_anim, dashing); break;
        case 1: SkullBehaviour(player_pos, dt, walk_anim); break;
        }
    }
    void Zombie_Behaviour1(float dt,Texture walk_anim[], Texture atk_anim[], Texture hit_anim[], Texture death_anim[],Texture Spawn_anim[],bool dashing)
    {      
        if (isEnemyhit && !isdeath)
        {
            shape.setTexture(hit_anim[Hit_imagecounter]);
        }
        else if (!isdeath && !isEnemyhit)
        {
            if (distance_from_player <= 75.0f)
            {
                Main_max_images = 7;
                shape.setTexture(atk_anim[imagecounter]);
                if (atkready && !dashing)
                {
                    Player_Health -= damage;
                    ishit = true;
                    atkready = false;
                }
            }
            else
            {
                Main_max_images = 8;
                shape.setTexture(walk_anim[imagecounter]);
                currentvelocity = norm_Direction * maxvelocity;
                shape.move(currentvelocity * dt);
            }
        }
        else if (isdeath)
        {           
            shape.setTexture(death_anim[Death_imagecounter]);
        }
    }
    void SkullBehaviour(Vector2f player_pos, float dt, Texture walk_anim[])
    {
        Main_max_images = 8;
        shape.setTexture(walk_anim[imagecounter]);
        currentvelocity = norm_Direction * maxvelocity;
        shape.move(currentvelocity * dt);
        if (distance_from_player <= 75)
        {
            explode = true;
            remove_Enemy = true;
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
struct MuzzleFlashEffect
{
    RadialLight MuzzleEffect;
    float counter = 1;
    bool deleteme = false;
    float muzzlerange = 50;
    void handlemuzzleeffect(float dt,Vector2f pos)
    {
        MuzzleEffect.setColor(Color::Yellow);
        MuzzleEffect.setPosition(pos);
        counter += dt;
        MuzzleEffect.setIntensity(50);
        muzzlerange /= counter;
        MuzzleEffect.setRange(muzzlerange);
        if (muzzlerange<= 2)
        {
            deleteme = true;
        }
    }
};

void init_walls(); // function to initilize walls at the start of the game
void GetTextures(); //function that gets the used textures by the game at the start of the program

void MusicHandler();

void Update(float dt); // the main update function where all game logic updates every frame

void Player_Movement(); // function responsible for the player movement
void Player_Collision(); // function that calculates and manages player collision with anything specified
void Switch_States(); // function that manages the current state of the player, i.e: player state right now is dashing, player state right now is dying, etc
void UpdateAnimationCounter(int maximagecounter, bool isonce); //function that handles the changing of the animation frame with the specified number of frames as argument
void UpdatePortalAnimation();

void Dashing(); //function that calculates dash direction and does the dashing
void AddDashLineVertexes(); // function that calculates the verticies of the dash effect
void draw_dash_line(); // function to draw the dash effect behind the player

void TimeSlow();
void MiniGunAbility();

void calculate_shoot_dir(); // function that calculates the normal direction between the player and the current mouse position
void KnockBack(float magnitude, Vector2f dir, float dt);
void explode();
void buying_weapons(); //function that  open guns for player 
void select_guns(); // function that switches between guns
void Switch_Current_Gun_Attributes(); //function that manages the current held gun attributes, i.e: the current gun fire rate is 1, current gun spread is 0,etc
void Shooting(); //function that handles shooting and spawning new bullets
void Bullet_Movement_Collision(float dt); //function that handles bullet collision
void Guns_Animation_Handling();
void Gun_switch_cooldown();
void Gun_UpdateAnimationCounter(float dt, int maximagecounter, double switchtime); // function that handles gun animations
void camera_shake();

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

void UI();

//menu
void game_openning_menu(Font font);
void Start(Font font);
void Credits(Font font);
void Exit(Font font);
void Pause(Font font);
void open_menu(Font font);
void switch_menu();
void Menu_Background(Font font);
void Game_over(Font font);
void Controls(Font font);


//levels


//door 




//level 3 variables

Texture Tbackground3;
Sprite Background3[4][7];

Texture HwallT3, VwallT3;
Sprite Hwall3[14], Vwall3[18];

Texture Tlantren_Lwall3;
Texture Tlantren_Rwall3;
Texture Tlantren_Mwall3;
Sprite lantren3[22];
//level 2 variables

Texture Tbackground2;
Sprite Background2[4][7];

Texture HwallT, VwallT;
Sprite Hwall[15], Vwall[15];

Texture TME2;
Sprite ME2[8];

Texture Tlantren_Lwall2;
Texture Tlantren_Rwall2;
Texture Tlantren_Mwall2;
Sprite lantren2[22];

//level 1 variables

Texture Tbackground;
Sprite background1[7][10];

Texture VwallT1, HwallT1;
Sprite Vwall1[15], Hwall1[15];

Texture lantren;
Sprite lantren1[17];

Texture ME;
Sprite ME1[7];




CircleShape test(5);
Vector2i center;
Vector2f globalcenter;
Vector2f globalorigin;

unsigned long long numberoftotalbulletsshot = 0;
//all font declear
Font normal_font;
// declear all text ui 
Text precent_sign;  
Text score_text;
Text money_text;
Text health_precent_text;
Text current_wave;
Text current_ammo_text;
Text ammo_stock_text;
Text money_smg_text;
Text money_sniper_text;
Text money_shotgun_text;
Text speedmachine_text;
Text reloadmachine_text;
Text Gun_controls;
//all sprites in the game
Sprite Player;
RectangleShape Gun(Vector2f(1.f,1.f));
RectangleShape BlackScreen(Vector2f(4000, 4000));
Sprite Pistol_S;
Sprite SMG_S;
Sprite ShotGun_S;
Sprite Sniper_S;
Sprite MiniGun_S;
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
Sprite slow_ability;
Sprite MiniGun_ability;
Sprite SpeedCola_S;
Sprite StaminaUp_S;
RectangleShape DashOrigin(Vector2f(50.0f, 50.0f));
RenderWindow window(VideoMode(1920, 1080), "ZombieGame",Style::Fullscreen);


Vector2f casingposition;

bool finishedanimationonce = false;
bool PortalOpen = false;

//dash booleans
bool isdashing = false;
bool dashready = true;
//time slow booleans
bool isSlowing = false;
bool Slowready = true;
//mini gun booleans
bool isMinigunActive = false;
bool isMinigunReady = true;
//reload booleans
bool isreloading = false;
//shoot boolean
bool isshooting = false;
bool trigger = true;
bool reload_trigger = false;//for sound effect

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

Texture MiniGun_Image;

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

Texture slow_ability_photo;
Texture minigun_ability_photo;

Texture SpeedCola_T;
Texture StaminaUp_T;

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
float slow_counter = 0;
float slow_multi = 1;
float minigun_counter = 0;
float knockbackmag = 0;
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

//sounds
SoundBuffer Pistol_shoot_Sound;
SoundBuffer Pistol_reload_Sound;

SoundBuffer rifle_shoot_Sound;
SoundBuffer rifle_reload_Sound;
SoundBuffer rifle_pickup_sound;

SoundBuffer shotgun_shoot_Buffer;
SoundBuffer shotgun_reload_Buffer;
SoundBuffer shotgun_pickup_Buffer;

SoundBuffer sniper_shoot_Sound;
SoundBuffer sniper_reload_Sound;
SoundBuffer sniper_pickup_Sound;

SoundBuffer minigun_shoot_Sound;

Sound ReloadSound;
Sound ShootSound;
SoundBuffer* Current_shoot_Buffer;
SoundBuffer* Current_reload_Buffer;

SoundBuffer music[3];
Sound MusicPlayer;
bool music_trigger = false;

int current_song = 0;
//zombie variables
int Current_Wave1 = 1;
int TotalSpawnedZombies = 0;
bool canspawn = true;

VertexArray dasheline(Quads);

vector<bullet> bullets;
vector <Enemy> zombies;
vector <BloodEffect> bloodeffects;
vector<Sprite> HealthPacks;
vector<Sprite> AmmoPacks;
vector<MuzzleFlashEffect> muzzleEffects;

//  menu
int menu_num = 1;
Texture Menu_background;
Sprite menu_background;

Texture Pause_menu;
Sprite pause_menu;

Texture End_game;
Sprite end_game;

Texture Control;
Sprite control;

View view(Vector2f(0, 0), Vector2f(window.getSize().x, window.getSize().y));

LightingArea ambientlight(candle::LightingArea::FOG,Vector2f(0,0),Vector2f(1920, 1080));
float playerdeltatime;
int main()
{
    init_walls();
    GetTextures();
    MusicPlayer.setVolume(64);
    ReloadSound.setVolume(48);
    ShootSound.setVolume(48);
    ambientlight.setAreaOpacity(150);
    ambientlight.setAreaColor(Color::Black);
    Player.setTexture(WalkAnimation[0]);
    Player.setPosition(Vector2f(500, 500));
    Gun.setOrigin(Player.getOrigin());
    Clock clock;
    Event event;
    window.setMouseCursorVisible(false);
    view.zoom(0.65);

    SwtichCurrentWallBounds();
    while (window.isOpen()) {
        float elapsed = clock.restart().asSeconds();
        playerdeltatime = elapsed;
        elapsed *= slow_multi;
        //cout << elapsed << endl;
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed) {
                window.close();
            }
            if (Keyboard::isKeyPressed(Keyboard::Escape) || curr_state == death)
            {
                switch_menu();
            }
        }
        window.setView(view);
        center = Vector2i(window.getSize().x / 2, window.getSize().y / 2);
        globalcenter = window.mapPixelToCoords(center);
        if (menu_num != 0)
        {
            window.clear();
            Menu_Background(normal_font);
            open_menu(normal_font);
            window.display();
            window.setMouseCursorVisible(true);
        }
        else
        {

            Update(elapsed);
            Draw();
            window.setMouseCursorVisible(false);

        }
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
}
void GetTextures()
{
    lantren.loadFromFile("lantren 1.png");
    Tlantren_Lwall2.loadFromFile("lantren_2_Lwall.png");
    Tlantren_Rwall2.loadFromFile("lantren_2_Rwall.png");
    Tlantren_Mwall2.loadFromFile("lantren_2_Mwall.png");
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
    MiniGun_Image.loadFromFile("minigun.png");
    CrossHair_Texture.loadFromFile("weapons/crosshair.png");
    Crosshair.setTexture(CrossHair_Texture);
    Crosshair.setOrigin(Crosshair.getLocalBounds().width / 2 + 50, Crosshair.getLocalBounds().height / 2);
    Crosshair.setScale(0.3, 0.3);
    Health_Texture.loadFromFile("health-red_32px.png");

    speedmachine_photo.loadFromFile("speedvanding.png");
    reloadmachine_photo.loadFromFile("ReloadVanding.png");
    speedmachine.setTexture(speedmachine_photo);
    reloadmachine.setTexture(reloadmachine_photo);
    //ui
    normal_font.loadFromFile("Gore Rough.otf");
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

    slow_ability_photo.loadFromFile("timer.png");
    slow_ability.setTexture(slow_ability_photo);
    slow_ability.setScale(0.25, 0.25);

    minigun_ability_photo.loadFromFile("minigun icon.png");
    MiniGun_ability.setTexture(minigun_ability_photo);
    MiniGun_ability.setScale(0.05, 0.05);

    Pistol_shoot_Sound.loadFromFile("sounds to use/Pistol/PistolShootSound.WAV");
    Pistol_reload_Sound.loadFromFile("sounds to use/Pistol/PistolReloadSound.wav");

    rifle_shoot_Sound.loadFromFile("sounds to use/Rifle/RifleShootSound.WAV");
    rifle_reload_Sound.loadFromFile("sounds to use/Rifle/RifleReloadSound.wav");
    rifle_pickup_sound.loadFromFile("sounds to use/Rifle/RiflePickupSound.WAV");

    shotgun_shoot_Buffer.loadFromFile("sounds to use/Shotgun/ShotGunShootSound.WAV");
    shotgun_reload_Buffer.loadFromFile("sounds to use/Shotgun/ShotGunReloadSound.WAV");
    shotgun_pickup_Buffer.loadFromFile("sounds to use/Shotgun/ShotGunPickupSound.WAV");

    sniper_shoot_Sound.loadFromFile("sounds to use/Sniper/SniperShootSound.wav");
    sniper_reload_Sound.loadFromFile("sounds to use/Sniper/SniperReloadSound.wav");
    sniper_pickup_Sound.loadFromFile("sounds to use/Sniper/SniperPickupSound.WAV");

    minigun_shoot_Sound.loadFromFile("sounds to use/MiniGun/MiniGunShootSound.wav");

    SpeedCola_T.loadFromFile("SpeedCola.png");
    StaminaUp_T.loadFromFile("StaminUp.png");

    SpeedCola_S.setTexture(SpeedCola_T);
    SpeedCola_S.setScale(0.05, 0.05);
    StaminaUp_S.setTexture(StaminaUp_T);
    StaminaUp_S.setScale(0.05,0.05);
    //menu
    Menu_background.loadFromFile("menu_background1.png");
    Pause_menu.loadFromFile("pause.png");
    End_game.loadFromFile("gameOver.png");
    Control.loadFromFile("game_controls.png");

    music[0].loadFromFile("Sad But True (Remastered).wav");
    music[1].loadFromFile("Seek & Destroy (Remastered).wav");
    music[2].loadFromFile("Metallica Shadows Follow.wav");
    for (int i = 0; i < 4; i++)
    {
        void1[i].setTexture(Void);
    }
    for (int i = 0; i < 15; i++)
    {
        lantren1[i].setTexture(lantren);
    }
}
//update function
void Update(float dt)
{

    Switch_States();
    
    if (curr_state != state::death)
    {  

        if (Keyboard::isKeyPressed(Keyboard::O));
        {
            //cout << MousePos.x <<"\t" << MousePos.y << endl;
        }

        current_player_pos = DashOrigin.getPosition();
        Vector2i pixelpos = Mouse::getPosition(window);
        MousePos = window.mapPixelToCoords(pixelpos);
        globalorigin = window.mapPixelToCoords(Vector2i(0,0));

        MusicHandler();

        Player_Movement();
        Player_Collision();

        Dashing();

        //zombies
        SpawnZombiesWaves(dt);
        HandleZombieBehaviour(dt);

        calculate_shoot_dir();
        buying_weapons();
        select_guns();
        Switch_Current_Gun_Attributes();
        Shooting();
        Bullet_Movement_Collision(dt);
        Gun_switch_cooldown();
        switch (current_level)
        {
        case 2:
            TimeSlow();
            break;
        case 3:
            MiniGunAbility();
            break;
        }
        if (delayfinished) { Guns_Animation_Handling(); }
        camera_shake();

        previous_player_pos = current_player_pos;
        //vanding
    }

}

//music function
void MusicHandler()
{
    if (isSlowing)
    {
        MusicPlayer.setPitch(0.5);
    }
    else
    {
        MusicPlayer.setPitch(1);
    }
    if (MusicPlayer.getStatus() != Sound::Playing && !music_trigger)
    {
        MusicPlayer.setBuffer(music[current_song]);
        MusicPlayer.play();
        music_trigger = true;
    }
    else if(MusicPlayer.getStatus() != Sound::Playing && music_trigger)
    {
        music_trigger = false;
        current_song++;
        if (current_song >= 3)
        {
            current_song = 0;
        }
    }
}

//Player-related functions
void Player_Movement()
{
    x = 0;
    y = 0;
    if (playerspeed > 0)
    {
        playerspeed -= 25;
    }
    if (Keyboard::isKeyPressed(Keyboard::A))
    {
        x = -1; 
        DashOrigin.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 10);
        if (playerspeed < 175)
        {
            playerspeed += 35;
        }
    }
    if (Keyboard::isKeyPressed(Keyboard::D))
    {
        x = 1;
        DashOrigin.setPosition(Player.getPosition().x - 50, Player.getPosition().y + 10);
        if (playerspeed < 175)
        {
            playerspeed += 35;
        }
    }
    if (Keyboard::isKeyPressed(Keyboard::W))
    {
        y = -1;
        if (playerspeed < 175)
        {
            playerspeed += 35;
        }
    }
    if (Keyboard::isKeyPressed(Keyboard::S))
    {
        y = 1;
        if (playerspeed < 175)
        {
            playerspeed += 35;
        }
    }
    DashOrigin.setPosition(DashOrigin.getPosition().x, Player.getPosition().y);
    Player.move(x * playerdeltatime * playerspeed, y * playerdeltatime * playerspeed);
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
                    Player.move(-playerspeed * playerdeltatime, 0);
                }
                else
                {
                    Player.move(playerspeed * playerdeltatime, 0);
                }
            }
            else
            {
                if (Player_Bounds.top < Wall_bound.top)
                {
                    Player.move(0, -playerspeed * playerdeltatime);
                }
                else
                {
                    Player.move(0, playerspeed * playerdeltatime);
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
            rifleammostock += 30;
            shotgunammostock += 4;
            sniperammostock += 10;
            AmmoPacks.erase(AmmoPacks.begin() + i);
        }
    }
    if (Player.getGlobalBounds().intersects(smg_buying.getGlobalBounds()))
    {
        smg_player_intersects = true;
    }
    else
        smg_player_intersects = false;
    if (Player.getGlobalBounds().intersects(sniper_buying.getGlobalBounds()))
    {
        sniper_player_intersects = true;
    }
    else
        sniper_player_intersects = false;
    if (Player.getGlobalBounds().intersects(shotgun_buying.getGlobalBounds()))
    {
        shotgun_player_intersects = true;
    }
    else
        shotgun_player_intersects = false;
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
        smg_buy = false;
        shotgun_buy = false;
        speed_pow = false;
        reload_pow = false;
        sniper_buy = false;
        speedmulti = 1;
        reloadmulti = 1;
        Money = 0;
        Wall_Bounds.clear();
        HealthPacks.clear();
        Curr_Gun_state = Pistol;
        playerspeed = 175;
        SwtichCurrentWallBounds();
        Current_Wave1 = 0;
        pistolbulletsloaded = 9;
        riflebulletsloaded = 30;
        shotgunbulletsloaded = 8;
        sniperbulletsloaded = 5;
    }
}
void Switch_States()
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
        menu_num = 6;
        if (highest_score < Score)
        {
            highest_score = Score;
        }
        curr_state = state::death;
    }
    if (ishit)
    {
        curr_state = state::hit;
    }
    switch (curr_state)
    {
    case state::walk: UpdateAnimationCounter(8, false); Player.setTexture(WalkAnimation[ImageCounter]); break;
    case state::idle: UpdateAnimationCounter(6, false); Player.setTexture(IdleAnimation[ImageCounter]); break;
    case state::death: UpdateAnimationCounter(6, true); Player.setTexture(DeathAnimation[ImageCounter]); break;
    case state::hit: UpdateAnimationCounter(4, true); Player.setTexture(HitAnimation[ImageCounter]); break;
    }
}
void UpdateAnimationCounter(int maximagecounter,bool isonce)
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
    AnimationCounter += playerdeltatime;
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
void UpdatePortalAnimation()
{
    Portal_animation_counter += playerdeltatime;
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
void Dashing()
{
    if (Keyboard::isKeyPressed(Keyboard::LShift) && dashready)
    {
        isdashing = true;
        dashready = false;
        playerspeed = 4000.0f;
    }
    if (isdashing && !dashready)
    {
        timesincedash += playerdeltatime;

        if (timesincedash > 0.05f)
        {
            isdashing = false;
            playerspeed = 175 * speedmulti;
            timesincedash = 0;
        }
    }
    if (!isdashing && !dashready)
    {
        timesincedash += playerdeltatime;
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

//abilties
void TimeSlow()
{
    if (Keyboard::isKeyPressed(Keyboard::F) && Slowready)
    {
        isSlowing = true;
        Slowready = false;
    }
    if (isSlowing && !Slowready)
    {
        slow_counter += playerdeltatime;
        slow_multi = 0.0001;
        if (slow_counter > 5)
        {
            isSlowing = false;
            slow_counter = 0;
            slow_multi = 1;
        }
    }
    if (!isSlowing && !Slowready)
    {
        slow_counter += playerdeltatime;
        if (slow_counter > 10.0)
        {
            Slowready = true;
            slow_counter = 0;
        }
    }
}

void MiniGunAbility()
{
    if (Keyboard::isKeyPressed(Keyboard::F) && isMinigunReady)
    {
        isMinigunActive = true;
        isMinigunReady = false;
        Curr_Gun_state = MiniGun;
        *current_ammo += 100;
    }
    if (isMinigunActive && !isMinigunReady)
    {
        minigun_counter += playerdeltatime;
        if (minigun_counter >= 3)
        {
            Curr_Gun_state = Pistol;
            isMinigunActive = false;           
            minigun_counter = 0;
        }
    }
    if (!isMinigunActive && !isMinigunReady)
    {
        minigun_counter += playerdeltatime;
        if (minigun_counter >= 6.0)
        {
            isMinigunReady = true;
            minigun_counter = 0;
        }
    }
}

//shooting functions
void calculate_shoot_dir()
{
    dir_vector = MousePos - Gun.getPosition();
    Norm_dir_vector = dir_vector / (sqrt(dir_vector.x * dir_vector.x + dir_vector.y * dir_vector.y));

    float rotation = atan2(Norm_dir_vector.y,Norm_dir_vector.x) * 180 / pi;
    
    Gun.setRotation(rotation);
}
void KnockBack(float magnitude, Vector2f dir,float dt)
{
    while (magnitude > 0)
    {
        float multi = 10;
        Player.move(dir  * multi * magnitude * dt);
        magnitude -= dt;
    }
}
void explode(float radius,float power,Vector2f Expo_Point)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        Vector2f Direction = Expo_Point - zombies[i].shape.getPosition();
        float magnitude = sqrt(Direction.x * Direction.x + Direction.y * Direction.y);
        if (magnitude <= radius)
        {
            zombies[i].health -= power * (radius / magnitude);
        }
    }
}
void select_guns()
{
    if (!isMinigunActive)
    {
        if (Keyboard::isKeyPressed(Keyboard::Num1))
        {
            Curr_Gun_state = Gun_State::Pistol;
            gun_switch_delay_counter = current_fire_rate;
            trigger = true;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num2) && smg_buy)
        {
            Curr_Gun_state = Gun_State::Smg;
            gun_switch_delay_counter = current_fire_rate;
            trigger = true;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num3) && shotgun_buy)
        {
            Curr_Gun_state = Gun_State::Shotgun;
            gun_switch_delay_counter = current_fire_rate;
            trigger = true;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num4) && sniper_buy)
        {
            Curr_Gun_state = Gun_State::Sniper;
            gun_switch_delay_counter = current_fire_rate;
            trigger = true;
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
        current_reload_time = pistolreloadtime * reloadmulti;
        current_bullets_loaded_per_reload = Pistol_bullets_loaded_per_reload;
        camera_shake_magnitude = 3;
        Current_shoot_Buffer = &Pistol_shoot_Sound;
        Current_reload_Buffer = &Pistol_reload_Sound;
        if (pistolbulletsloaded > 9)
        {
            pistolbulletsloaded = 9;
        }
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
        Current_shoot_Buffer = &rifle_shoot_Sound;
        Current_reload_Buffer = &rifle_reload_Sound;
        if (riflebulletsloaded > 30)
        {
            riflebulletsloaded = 30;
        }
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
        Current_shoot_Buffer = &shotgun_shoot_Buffer;
        Current_reload_Buffer = &shotgun_reload_Buffer;
        if (shotgunbulletsloaded > 8)
        {
            shotgunbulletsloaded = 8;
        }
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
        Current_shoot_Buffer = &sniper_shoot_Sound;
        Current_reload_Buffer = &sniper_reload_Sound;
        if (sniperbulletsloaded > 5)
        {
            sniperbulletsloaded = 5;
        }
        break;
    case MiniGun:
        current_fire_rate = minigunfirerate;
        current_damage = minigundamage;
        current_bullets_per_shot = minigunbulletpershot;
        current_spread = minigunspread;
        current_ammo = &minigunbulletsloaded;
        current_ammo_stock = &minigunammostock;
        current_clip_size = minigunclipsize;
        current_reload_time = minigunreloadtime * reloadmulti;
        current_bullets_loaded_per_reload = minigun_bullets_loaded_per_reload;
        Current_shoot_Buffer = &minigun_shoot_Sound;
        camera_shake_magnitude = 20;
        if (*current_ammo_stock <= 0)
        {
            *current_ammo_stock = 99999;
        }
    }
}
void Shooting()
{
    fire_rate_counter += playerdeltatime;
    if (Mouse::isButtonPressed(Mouse::Left) && fire_rate_counter >= current_fire_rate && *current_ammo > 0 && !isreloading)
    {
        fire_rate_counter = 0;
        camera_shake_counter += 1;
        if (camera_shake_counter > camera_shake_duration)
        {
            camera_shake_counter = camera_shake_duration;
        }
        KnockBack(1, -Norm_dir_vector,playerdeltatime);
        MuzzleFlashEffect newmuzzleeffect;
        newmuzzleeffect.MuzzleEffect.setPosition(Player.getPosition());
        muzzleEffects.push_back(newmuzzleeffect);
        ShootSound.setBuffer(*Current_shoot_Buffer);
        ShootSound.play();
        for (int i = 0; i < current_bullets_per_shot; i++)
        {
            bullet newbullet;
            newbullet.shape.setPosition(test.getPosition());
            newbullet.shape.setScale(0.5f, 0.5f);
            newbullet.id = numberoftotalbulletsshot;
            newbullet.curr_gun_state = Curr_Gun_state;
            newbullet.guntrail.color = Color::Yellow;
            newbullet.guntrail.thickness = 5;
            newbullet.guntrail.maxlength = 7;
            Vector2f Offset(rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX));
            newbullet.currentvelocity = newbullet.maxvelocity * (Norm_dir_vector + (Offset * 0.2f * current_spread));
            newbullet.damage = current_damage;
            bullets.push_back(newbullet);
            numberoftotalbulletsshot++;
        }
        if (Curr_Gun_state == MiniGun)
        {
            *current_ammo -= 3;
        }
        else
        {
            *current_ammo -= 1;
        }
    }
    //reloading
    if (((Keyboard::isKeyPressed(Keyboard::R) && *current_ammo < current_clip_size && *current_ammo_stock >0) ||(*current_ammo <= 0 && *current_ammo_stock > 0))||isreloading)
    {
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
        reload_time_counter += playerdeltatime;
        //play reload sound
        if (ReloadSound.getStatus() != Sound::Playing && !reload_trigger)
        {
            ReloadSound.setPitch(1 / reloadmulti);
            ReloadSound.setBuffer(*Current_reload_Buffer);
            ReloadSound.play();
            reload_trigger = true;
        }
        if (reload_time_counter > current_reload_time)//finished reloading
        {         
            reload_trigger = false;
            if (*current_ammo_stock >= current_clip_size || (Curr_Gun_state == Gun_State::Shotgun && *current_ammo_stock > 0))
            {
                *current_ammo += current_bullets_loaded_per_reload;
                if (Curr_Gun_state != Pistol)
                {
                    *current_ammo_stock -= current_bullets_loaded_per_reload;
                }
            }
            else if(*current_ammo_stock < current_clip_size)
            {
                *current_ammo = *current_ammo_stock;
                *current_ammo_stock = 0;
            }
            if (Curr_Gun_state != Shotgun || (Curr_Gun_state == Shotgun && *current_ammo >= current_clip_size))
            {
                isreloading = false;
            }
            reload_time_counter = 0;
        }
        if (Curr_Gun_state == Gun_State::Shotgun && Mouse::isButtonPressed(Mouse::Left))
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
                bullets[i].zombieshit++;
                zombies[j].health -= bullets[i].damage;
                if (Curr_Gun_state == Shotgun)
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt * 1 / 16, bullets[i].currentvelocity.y * dt * 1 / 16);
                }
                else if(Curr_Gun_state == Sniper)
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt * 10, bullets[i].currentvelocity.y * dt * 10);
                }
                else if (Curr_Gun_state == MiniGun)
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt * 0.0001, bullets[i].currentvelocity.y * dt  *0.0001);
                }
                else
                {
                    zombies[j].shape.move(bullets[i].currentvelocity.x * dt, bullets[i].currentvelocity.y * dt);
                }
                zombies[j].isEnemyhit = true;
                BloodEffect newbloodeffect;
                newbloodeffect.BloodShape.setPosition(zombies[j].shape.getPosition());
                newbloodeffect.BloodShape.setScale(zombies[j].shape.getScale().x * -0.5f, zombies[j].shape.getScale().y * 0.5f);
                newbloodeffect.assign_random_number();
                bloodeffects.push_back(newbloodeffect);
                if(zombies[j].health <= 0)
                {
                    int random_num = rand() % 100;
                    zombies[j].isdeath = true;
                    if (random_num > 70 && random_num < 81)
                    {
                        Sprite newhealthpack;
                        newhealthpack.setTexture(Health_Texture);
                        newhealthpack.setPosition(zombies[j].shape.getPosition());
                        HealthPacks.push_back(newhealthpack);

                    }
                    else if (random_num > 80 && random_num < 90)
                    {
                        Sprite newammostock;
                        newammostock.setTexture(ammo_smg_photo);
                        newammostock.setPosition(zombies[j].shape.getPosition());
                        AmmoPacks.push_back(newammostock);
                    }
                    Score += 10;
                    Money += 75;
                }
            }

        }
    }
    for (int i = 0; i < muzzleEffects.size(); i++)
    {
        if (muzzleEffects[i].deleteme)
        {
            muzzleEffects.erase(muzzleEffects.begin() + i);
            break;
        }
        muzzleEffects[i].handlemuzzleeffect(playerdeltatime,test.getPosition());
    }
}
void Guns_Animation_Handling()
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
        case MiniGun:
            MiniGun_S.setTexture(MiniGun_Image);
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
        case MiniGun:
            MiniGun_S.setTexture(MiniGun_Image);
        }
    }
    else
    {
        SMG_S.setTexture(SMG_Shoot_Animations[0]);
        Pistol_S.setTexture(pistol_shoot_animations[0]);
        ShotGun_S.setTexture(shotgun_shoot_animation[0]);
        Sniper_S.setTexture(sniper_shoot_animations[0]);
        gun_switch_delay_counter = current_fire_rate;
        MiniGun_S.setTexture(MiniGun_Image);
        trigger = true;
        isshooting = false;
    }
    if (isreloading)
    {
        Gun_UpdateAnimationCounter(playerdeltatime, gun_frames, current_reload_time / (double)gun_frames);
    }
    else
    {
        Gun_UpdateAnimationCounter(playerdeltatime, gun_frames, current_fire_rate / (double)gun_frames );
    }
}
void Gun_switch_cooldown()
{
    if (gun_switch_delay_counter > 0)
    {
        gun_switch_delay_counter -= playerdeltatime;
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
void camera_shake()
{
    if (camera_shake_counter > 0)
    {
        float magnitudereduction = camera_shake_magnitude * (camera_shake_counter / camera_shake_duration);
        cameraoffset_shake = Vector2f((rand() /static_cast<float>( RAND_MAX )) * magnitudereduction - magnitudereduction / 2, (rand() / static_cast<float>(RAND_MAX)) * magnitudereduction - magnitudereduction / 2);
        view.setCenter(Player.getPosition() + cameraoffset_shake);
        camera_shake_counter -= playerdeltatime;
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
        Enemy newzombie;
        newzombie.shape.setPosition( 50 +rand() % 1770, 50 +rand() % 930);
        newzombie.damage *= multiplier;
        newzombie.attack_fire_rate *=(1 / multiplier) ;
        newzombie.maxvelocity *= multiplier;
        newzombie.setspawnlocation();
        zombies.push_back(newzombie);
        TotalSpawnedZombies++;
        SpawningZombieCounter = 0;
    }
    if (TotalSpawnedZombies >= 35 * Current_Wave1)
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
        UpdatePortalAnimation();
    }
}
void HandleZombieBehaviour(float dt)
{
    for (int i = 0; i < zombies.size(); i++)
    {
        if (zombies[i].remove_Enemy)
        {
            zombies.erase(zombies.begin() + i);
            break;
        }
        zombies[i].EnemyUpdate(Player.getPosition(), dt, zombie_walk_animation, zombie_atk_animation, zombie_hit_animation, zombie_death_animation, Zombie_spawn_animation, isdashing);
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
                        zombies[i].shape.move(-2500 * dt, 0);
                    }
                    else
                    {
                        zombies[i].shape.move(2500 * dt, 0);
                    }
                }
                else
                {
                    if (current_zombie_Bound.top < Wall_bound.top)
                    {
                        zombies[i].shape.move(0, -2500 * dt);
                    }
                    else
                    {
                        zombies[i].shape.move(0, 2500 * dt);
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

//level 1 functions

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


//drawing function
void Draw()
{
    Player.setOrigin(Vector2f(Player.getLocalBounds().width / 2, Player.getLocalBounds().height / 2));
    RadialLight light;
    light.setRange(400);
    light.setIntensity(150);
    light.setColor(Color(229, 238, 141));
    CircleShape testpoint(5);
    testpoint.setOrigin(testpoint.getLocalBounds().width / 2, testpoint.getLocalBounds().height / 2);
    testpoint.setPosition(Player.getPosition());
    if (menu_num != 5 && menu_num != 6) {
        ambientlight.clear();
        window.clear();
    }

    if (Norm_dir_vector.x >= 0)
    {
        Player.setScale(2, 2);
        Gun.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 20.f);
        Gun.setScale(0.5, 0.5);
    }
    else
    {
        Player.setScale(-2, 2);
        Gun.setPosition(Player.getPosition().x - 15, Player.getPosition().y + 20.f);
        Gun.setScale(0.5, -0.5);
    }
    test.setOrigin(test.getLocalBounds().width / 2, test.getLocalBounds().height / 2);
    switch (current_level)
    {
    case 1:
        Portal_S.setPosition(580, 790);
        speedmachine.setPosition(Vector2f(200, 25));
        reloadmachine.setPosition(Vector2f(1080, 580)); 
        smg_buying.setPosition(Vector2f(860, 415)); 
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 7; j++)
            {
                window.draw(background1[j][i]);
            }
        }
        shotgun_buying.setPosition(Vector2f(1700, 790));
        sniper_buying.setPosition(Vector2f(500, 700));
        //upper wall

        for (int i = 0; i < 5; i++)
        {
            Hwall1[i].setPosition(405 * i, 0);
            light.setPosition((405 * i) + 100, 20);
            window.draw(light);
            ambientlight.draw(light);
            lantren1[i].setPosition((405 * i) + 100, 20);
        }
        //lower wall

        for (int i = 5; i < 10; i++)
        {
            Hwall1[i].setPosition(405 * (i - 5), 1020);
            light.setPosition(405 * (i - 5), 1020);
            window.draw(light);
            ambientlight.draw(light);
        }
        //left wall

        for (int i = 0; i < 4; i++)
        {
            Vwall1[i].setPosition(0, (405 * i) + 40);
            lantren1[i + 9].setPosition(20, (405 * i) + 85);
        }
        //Right wall

        for (int i = 4; i < 8; i++)
        {
            Vwall1[i].setPosition(1890, (405 * (i - 4)) + 40);
            lantren1[i + 8].setPosition(1880, (405 * (i - 4)) + 85);
        }

        //mid walls
        for (int i = 10; i < 14; i++)
        {
            Hwall1[i].setPosition((405 * (i - 10)) + 30, 550);
            lantren1[i - 5].setPosition((405 * (i - 10)) + 130, 570);
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
        for (int i = 9; i < 17; i++)
        {
            window.draw(lantren1[i]);
            light.setPosition(lantren1[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        for (int i = 0; i < 10; i++)
        {
            window.draw(Vwall1[i]);
        }     
        //lower wall
        for (int i = 5; i < 10; i++)
        {
            window.draw(Hwall1[i]);
        }      
        window.draw(Vwall1[9]);
        for (int i = 0; i < 4; i++)
        {
            window.draw(ME1[i]);
        }
        for (int i = 0; i < 9; i++)
        {
            window.draw(lantren1[i]);
            light.setPosition(lantren1[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        break;
    case 2:
        light.setColor(Color(255, 221, 82));
        Portal_S.setPosition(960, 540);
        speedmachine.setPosition(Vector2f(400, 25));
        reloadmachine.setPosition(Vector2f(850, 25));
        smg_buying.setPosition(Vector2f(200, 415));
        shotgun_buying.setPosition(Vector2f(1000, 790));
        sniper_buying.setPosition(Vector2f(1700, 700));
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

        lantren2[18].setPosition(590, 360);
        lantren2[19].setPosition(1275, 360);
        lantren2[20].setPosition(590, 630);
        lantren2[21].setPosition(1275, 630);
        //lantren 
        for (int i = 0; i < 22; i++)
        {
            lantren2[i].setColor(Color(237, 174, 161));
        }
        for (int i = 0; i < 4; i++)
        {
            lantren2[i].setTexture(Tlantren_Rwall2);
            lantren2[i].setPosition(1845, 20 + (300 * i));

            lantren2[i + 4].setTexture(Tlantren_Lwall2);
            lantren2[i + 4].setPosition(0, 20 + (300 * i));

        }
        for (int i = 8; i < 13; i++)
        {
            lantren2[i].setTexture(Tlantren_Mwall2);
            lantren2[i].setPosition(200 + (500 * (i - 8)), 10);
        }
        for (int i = 18; i < 22; i++)
        {
            lantren2[i].setTexture(Tlantren_Mwall2);
        }
        for (int i = 13; i < 17; i++)
        {
            lantren2[i].setTexture(Tlantren_Mwall2);
            lantren2[i].setPosition(200 + (500 * (i - 13)), 980);
        }
        for (int i = 8; i < 17; i++)
        {
            lantren2[i].setTexture(Tlantren_Mwall2);
        }
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                window.draw(Background2[j][i]);
            }
        }
        for (int i = 13; i < 17; i++)
        {
            window.draw(lantren2[i]);
            light.setPosition(lantren2[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        window.draw(lantren2[20]);
        light.setPosition(lantren2[20].getPosition().x + 35, lantren2[20].getPosition().y);
        window.draw(light);
        ambientlight.draw(light);
        window.draw(lantren2[21]);
        light.setPosition(lantren2[21].getPosition().x + 35, lantren2[21].getPosition().y);
        window.draw(light);
        ambientlight.draw(light);
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
        window.draw(Vwall[10]);
        window.draw(Vwall[11]);
        for (int i = 0; i < 6; i++)
        {
            window.draw(ME2[i]);
        }
        window.draw(lantren2[19]);
        light.setPosition(lantren2[19].getPosition().x + 35, lantren2[19].getPosition().y);
        window.draw(light);
        ambientlight.draw(light);
        window.draw(lantren2[18]);
        light.setPosition(lantren2[18].getPosition().x + 35, lantren2[18].getPosition().y);
        window.draw(light);
        ambientlight.draw(light);
        for (int i = 0; i < 13; i++)
        {
            window.draw(lantren2[i]);
            light.setPosition(lantren2[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        break;
    case 3:
        light.setColor(Color(229, 238, 141));
        Portal_S.setPosition(960, 540);
        speedmachine.setPosition(Vector2f(35, 25));
        reloadmachine.setPosition(Vector2f(1500, 25));
        smg_buying.setPosition(Vector2f(200, 415));
        shotgun_buying.setPosition(Vector2f(500, 790));
        sniper_buying.setPosition(Vector2f(1700, 700));
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

        Tlantren_Lwall3.loadFromFile("lantren_1_Lwall.png");
        Tlantren_Rwall3.loadFromFile("lantren_1_Rwall.png");
        Tlantren_Mwall3.loadFromFile("lantren_1_Mwall.png");
        //lantren 
        for (int i = 17; i < 22; i++)
        {
            lantren3[i].setTexture(Tlantren_Mwall3);
            lantren3[i].setPosition(200 + (500 * (i - 17)), 530);

        }
        for (int i = 0; i < 4; i++)
        {
            lantren3[i].setTexture(Tlantren_Rwall3);
            lantren3[i].setPosition(1845, 20 + (300 * i));

            lantren3[i + 4].setTexture(Tlantren_Lwall3);
            lantren3[i + 4].setPosition(30, 20 + (300 * i));
        }
        for (int i = 8; i < 13; i++)
        {
            lantren3[i].setTexture(Tlantren_Mwall3);
            lantren3[i].setPosition(200 + (500 * (i - 8)), 10);
        }
        for (int i = 13; i < 17; i++)
        {
            lantren3[i].setTexture(Tlantren_Mwall3);
            lantren3[i].setPosition(200 + (500 * (i - 13)), 980);
        }
        for (int i = 8; i < 17; i++)
        {
            lantren3[i].setTexture(Tlantren_Mwall3);
        }
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                window.draw(Background3[j][i]);
            }
        }
        for (int i = 13; i < 17; i++)
        {
            window.draw(lantren3[i]);
            light.setPosition(lantren3[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        for (int i = 0; i < 18; i++)
        {
            window.draw(Vwall3[i]);
        }
        for (int i = 0; i < 14; i++)
        {
            window.draw(Hwall3[i]);
        }
        for (int i = 0; i < 13; i++)
        {
            window.draw(lantren3[i]);
            light.setPosition(lantren3[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        for (int i = 17; i < 22; i++)
        {
            window.draw(lantren3[i]);
            light.setPosition(lantren3[i].getPosition());
            window.draw(light);
            ambientlight.draw(light);
        }
        break;
    }
   //window.draw(test);
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
        bullets[i].shape.setOrigin(bullets[i].shape.getLocalBounds().width / 2, bullets[i].shape.getLocalBounds().height / 2);
        window.draw(bullets[i].lighteffect);
        bullets[i].guntrail.drawTrail(window);
        if(isSlowing){window.draw(bullets[i].shape); }
        ambientlight.draw(bullets[i].lighteffect);
    }
    for (int i = 0; i < zombies.size(); i++)
    {
        zombies[i].SpawnEffect.setOrigin(zombies[i].SpawnEffect.getLocalBounds().width / 2, zombies[i].SpawnEffect.getLocalBounds().height / 2);      
        window.draw(zombies[i].SpawnEffect);
        if (zombies[i].Spawn_imagecounter <= 6)
        {
            window.draw(zombies[i].SpawnLight);
            ambientlight.draw(zombies[i].SpawnLight);
        }
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
        Pistol_S.setOrigin(Pistol_S.getLocalBounds().width/4, Pistol_S.getLocalBounds().height / 2);
        test.setPosition(Pistol_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 10, Pistol_S.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 10);
        Pistol_S.setScale(Gun.getScale().x, Gun.getScale().y);
        Pistol_S.setPosition(Gun.getPosition().x, Gun.getPosition().y - 10);
        Pistol_S.setRotation(Gun.getRotation());
        window.draw(Pistol_S);
        break;
    case Smg:
        SMG_S.setOrigin(SMG_S.getLocalBounds().width / 4, SMG_S.getLocalBounds().height / 2);
        test.setPosition(SMG_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 25, SMG_S.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 25);
        SMG_S.setScale(Gun.getScale().x * 3 / 4, Gun.getScale().y * 3 / 4);
        SMG_S.setPosition(Gun.getPosition().x - 10, Gun.getPosition().y - 10);
        SMG_S.setRotation(Gun.getRotation());
        window.draw(SMG_S);
        break;
    case Shotgun:
        ShotGun_S.setOrigin(ShotGun_S.getLocalBounds().width / 4, ShotGun_S.getLocalBounds().height / 2);
        test.setPosition(ShotGun_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 30, ShotGun_S.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 30);
        ShotGun_S.setScale(Gun.getScale().x * 3 / 4, Gun.getScale().y * 3 / 4);
        ShotGun_S.setPosition(Gun.getPosition());
        ShotGun_S.setRotation(Gun.getRotation());
        window.draw(ShotGun_S);
        break;
    case Sniper:
        Sniper_S.setOrigin(Sniper_S.getLocalBounds().width / 4, Sniper_S.getLocalBounds().height / 2);
        test.setPosition(Sniper_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 40, Sniper_S.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 40);
        Sniper_S.setScale(Gun.getScale().x * 3 / 4, Gun.getScale().y * 3 / 4);
        Sniper_S.setPosition(Gun.getPosition());
        Sniper_S.setRotation(Gun.getRotation());
        window.draw(Sniper_S);
        break;
    case MiniGun:
        MiniGun_S.setOrigin(MiniGun_S.getLocalBounds().width / 4, MiniGun_S.getLocalBounds().height / 2);
        test.setPosition(MiniGun_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 10, MiniGun_S.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 50);
        MiniGun_S.setScale(Gun.getScale().x * 4, Gun.getScale().y * 4);
        MiniGun_S.setPosition(Gun.getPosition());
        MiniGun_S.setRotation(Gun.getRotation());
        window.draw(MiniGun_S);
        break;
    } 
    for (int i = 0; i < muzzleEffects.size(); i++)
    {
        window.draw(muzzleEffects[i].MuzzleEffect);
        ambientlight.draw(muzzleEffects[i].MuzzleEffect);
    }
    for (int i = 0; i < HealthPacks.size(); i++)
    {
        window.draw(HealthPacks[i]);
    }
    for (int i = 0; i < AmmoPacks.size(); i++)
    {
        window.draw(AmmoPacks[i]);
    }
    window.draw(ambientlight);
    Crosshair.setPosition(MousePos);
    void1[0].setPosition(0, 1080);
    void1[1].setPosition(0, -2000);
    void1[2].setPosition(1920, 0);
    void1[3].setPosition(-2864, -400);
    speedmachine.setScale(0.5, 0.5);
    reloadmachine.setScale(0.5, 0.5);
    UI();
    window.draw(Crosshair);
    if (menu_num != 5 && menu_num != 6)
    {
        window.display();
        ambientlight.display();
    }
}
void UI()
{
    RadialLight light;
    light.setRange(100);
    light.setIntensity(200);
    light.setColor(Color::Yellow);

    //smg price text 
    money_smg_text.setFont(normal_font);
    money_smg_text.setString("$" + to_string(money_smg));
    money_smg_text.setCharacterSize(9);
    money_smg_text.setFillColor(Color::White);
    money_smg_text.setPosition(smg_buying.getPosition().x + 20, smg_buying.getPosition().y);
    if (smg_player_intersects && !smg_buy)
        window.draw(money_smg_text);
    //sniper price text 
    money_sniper_text.setFont(normal_font);
    money_sniper_text.setString("$" + to_string(money_sniper));
    money_sniper_text.setCharacterSize(9);
    money_sniper_text.setFillColor(Color::White);
    money_sniper_text.setPosition(sniper_buying.getPosition().x + 20, sniper_buying.getPosition().y);
    if (sniper_player_intersects && !sniper_buy)
        window.draw(money_sniper_text);
    //shotgun price text 
    money_shotgun_text.setFont(normal_font);
    money_shotgun_text.setString("$" + to_string(money_shotgun));
    money_shotgun_text.setCharacterSize(9);
    money_shotgun_text.setFillColor(Color::White);
    money_shotgun_text.setPosition(shotgun_buying.getPosition().x + 20, shotgun_buying.getPosition().y);
    if (shotgun_player_intersects && !shotgun_buy)
        window.draw(money_shotgun_text);
    //speedmachine price text 
    speedmachine_text.setFont(normal_font);
    speedmachine_text.setString("$" + to_string(speedmoney));
    speedmachine_text.setCharacterSize(9);
    speedmachine_text.setFillColor(Color::White);
    speedmachine_text.setPosition(speedmachine.getPosition().x + 20, speedmachine.getPosition().y - 12);
    if (Player.getGlobalBounds().intersects(speedmachine.getGlobalBounds()))
        window.draw(speedmachine_text);
    //
    reloadmachine_text.setFont(normal_font);
    reloadmachine_text.setString("$" + to_string(reloadmoney));
    reloadmachine_text.setCharacterSize(9);
    reloadmachine_text.setFillColor(Color::White);
    reloadmachine_text.setPosition(reloadmachine.getPosition().x + 20, reloadmachine.getPosition().y - 12);
    if (Player.getGlobalBounds().intersects(reloadmachine.getGlobalBounds()))
        window.draw(reloadmachine_text);
    /* {end drawing ui }
     to draw guns
     */

    if (!sniper_buy)
    {
        window.draw(sniper_buying);
        light.setPosition(sniper_buying.getPosition().x + sniper_buying.getLocalBounds().width / 2, sniper_buying.getPosition().y + sniper_buying.getLocalBounds().height / 2);
        window.draw(light);
        ambientlight.draw(light);
    }

    //smg
    smg_buying.setTexture(smg_photo);

    if (!smg_buy)
    {
        window.draw(smg_buying);
        light.setPosition(smg_buying.getPosition().x + smg_buying.getLocalBounds().width / 2, smg_buying.getPosition().y + smg_buying.getLocalBounds().height / 2);
        window.draw(light);
        ambientlight.draw(light);
    }
    // shotgun 
    shotgun_buying.setTexture(shotgun_photo);

    if (!shotgun_buy)
    {
        window.draw(shotgun_buying);
        light.setPosition(shotgun_buying.getPosition().x + shotgun_buying.getLocalBounds().width / 2, shotgun_buying.getPosition().y + shotgun_buying.getLocalBounds().height / 2);
        window.draw(light);
        ambientlight.draw(light);
    }
    RectangleShape health_bar(Vector2f(140, 15));
    health_bar.setScale(Vector2f((Player_Health / 100.0) * 2, 2));
    health_bar.setPosition(window.mapPixelToCoords(Vector2i(60, 20)));
    RectangleShape missing_health_bar(Vector2f(140, 15));
    missing_health_bar.setFillColor(Color::White);
    missing_health_bar.setScale(Vector2f(2, 2));
    missing_health_bar.setPosition(window.mapPixelToCoords(Vector2i(60, 20)));
    RectangleShape health_bar_background(Vector2f(145 * 2, 20 * 2));
    health_bar_background.setFillColor(Color::Black);
    health_bar_background.setPosition(health_bar.getPosition().x - 5, health_bar.getPosition().y - 5);
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
    health_precent_text.setFillColor(Color(128, 128, 128));
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

    score_text.setFont(normal_font); // select the font 
    score_text.setString("Score : " + to_string(Score));
    score_text.setCharacterSize(36);
    score_text.setFillColor(Color(136, 8, 8));
    score_text.setPosition(window.mapPixelToCoords(Vector2i(0, 980)));
    window.draw(score_text);

    // to print money number

    money_text.setFont(normal_font); // select the font 
    money_text.setString(" : " + to_string(Money));
    money_text.setCharacterSize(36);
    money_text.setFillColor(Color(255, 215, 0));
    money_text.setPosition(window.mapPixelToCoords(Vector2i(80, 890)));
    window.draw(money_text);
    // to print  current wave  

    current_wave.setFont(normal_font);
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

    //{to print  amo stock text }

    ammo_stock_text.setFont(normal_font);
    if (Curr_Gun_state == Pistol)
    {
        ammo_stock_text.setString(" /inf" );
    }
    else
    {
        ammo_stock_text.setString(" /" + to_string(*current_ammo_stock));
    }
    ammo_stock_text.setPosition(window.mapPixelToCoords(Vector2i(1270, 587)));
    ammo_stock_text.setCharacterSize(12);
    ammo_stock_text.setFillColor(Color::White);
    if (Curr_Gun_state != MiniGun)
    {
        window.draw(ammo_stock_text);
        window.draw(current_ammo_text);
    }
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
    //end draw guns 
    slow_ability.setPosition(window.mapPixelToCoords(Vector2i(1200, 50)));
    MiniGun_ability.setPosition(window.mapPixelToCoords(Vector2i(1200, 50)));
    if (Slowready && current_level == 2)
    {
        window.draw(slow_ability);
    }
    if (isMinigunReady && current_level == 3)
    {
        window.draw(MiniGun_ability);
    }
    SpeedCola_S.setPosition(window.mapPixelToCoords(Vector2i(50, 75)));
    StaminaUp_S.setPosition(window.mapPixelToCoords(Vector2i(75+SpeedCola_S.getGlobalBounds().width, 75)));
    if (speed_pow)
    {
        window.draw(StaminaUp_S);
    }
    if (reload_pow)
    {
        window.draw(SpeedCola_S);
    }
    Gun_controls.setFont(normal_font); // select the font 
    Gun_controls.setString("1:Pistol,2:Rifle,3:ShotGun,4:Sniper");
    Gun_controls.setCharacterSize(18);
    Gun_controls.setFillColor(Color::White);
    Gun_controls.setPosition(window.mapPixelToCoords(Vector2i(1400, 0)));
    window.draw(Gun_controls);
}
void buying_weapons()
{
    if (Player.getGlobalBounds().intersects(smg_buying.getGlobalBounds()) && Money >= money_smg && !smg_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_smg;
        smg_buy = true;
        Score += 100;
        ReloadSound.setBuffer(rifle_pickup_sound);
        ReloadSound.play();
    }
    if (Player.getGlobalBounds().intersects(shotgun_buying.getGlobalBounds()) && Money >= money_shotgun && !shotgun_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_shotgun;
        shotgun_buy = true;
        Score += 100;
        ReloadSound.setBuffer(shotgun_pickup_Buffer);
        ReloadSound.play();
    }
    if (Player.getGlobalBounds().intersects(sniper_buying.getGlobalBounds()) && Money >= 2000 && !sniper_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= 2000;
        sniper_buy = true;
        Score += 100;
        ReloadSound.setBuffer(sniper_pickup_Sound);
        ReloadSound.play();
    }
}
void game_openning_menu(Font font)
{
    Text start; start.setFont(font); start.setString("start"); start.setPosition(globalcenter.x - 100, globalcenter.y - 250); FloatRect collesion1 = start.getGlobalBounds();
    if (collesion1.contains(MousePos))
    {
        start.setFillColor(Color(100, 100, 100, 100));  start.setCharacterSize(50); window.draw(start);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 2;
        }
    }
    else
    {
        start.setFillColor(Color(225, 225, 225, 225));  start.setCharacterSize(45); window.draw(start);
    }

    Text credits; credits.setFont(font); credits.setString("credits"); credits.setPosition(globalcenter.x - 100, globalcenter.y - 100); FloatRect collesion2 = credits.getGlobalBounds();
    if (collesion2.contains(MousePos))
    {
        credits.setFillColor(Color(100, 100, 100, 100));  credits.setCharacterSize(50); window.draw(credits);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 3;
        }
    }
    else
    {
        credits.setFillColor(Color(225, 225, 225, 225));  credits.setCharacterSize(45); window.draw(credits);
    }

    Text exit; exit.setFont(font); exit.setString("exit"); exit.setPosition(globalcenter.x - 100, globalcenter.y + 200); FloatRect collesion3 = exit.getGlobalBounds();
    if (collesion3.contains(MousePos))
    {
        exit.setFillColor(Color(100, 100, 100, 100));  exit.setCharacterSize(50); window.draw(exit);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 4;
        }
    }
    else
    {
        exit.setFillColor(Color(225, 225, 225, 225));  exit.setCharacterSize(45); window.draw(exit);
    }


    Text controls; controls.setFont(font); controls.setString("controls"); controls.setPosition(globalcenter.x - 100, globalcenter.y + 50); FloatRect collesion4 = controls.getGlobalBounds();
    if (collesion4.contains(MousePos))
    {
        controls.setFillColor(Color(100, 100, 100, 100));  controls.setCharacterSize(50); window.draw(controls);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 7;
        }
    }
    else
    {
        controls.setFillColor(Color(225, 225, 225, 225));  controls.setCharacterSize(45); window.draw(controls);
    }

}

void Start(Font font)
{
    Text newgame; newgame.setFont(font); newgame.setString("new game"); newgame.setPosition(globalcenter.x - 250, globalcenter.y - 150); FloatRect collesion1 = newgame.getGlobalBounds();
    if (collesion1.contains(MousePos))
    {
        newgame.setFillColor(Color(0, 100, 100, 60));  newgame.setCharacterSize(45); window.draw(newgame);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            current_level = 2;
            Player.setPosition(500, 500);
            Player_Health = 100;
            Score = 0;
            menu_num = 0;
            zombies.clear();
            TotalSpawnedZombies = 0;
            PortalOpen = false;
            smg_buy = true;
            shotgun_buy = true;
            speed_pow = false;
            reload_pow = false;
            sniper_buy = true;
            speedmulti = 1;
            reloadmulti = 1;
            Money = 100000;
            Wall_Bounds.clear();
            HealthPacks.clear();
            bloodeffects.clear();
            Curr_Gun_state = Pistol;
            playerspeed = 175;
            SwtichCurrentWallBounds();
            Current_Wave1 = 0;
            MusicPlayer.stop();
            current_song = 0;
            MusicPlayer.play();
            pistolbulletsloaded = 9;
            riflebulletsloaded = 30;
            shotgunbulletsloaded = 8;
            sniperbulletsloaded = 5;
        }
    }
    else
    {
        newgame.setFillColor(Color(225, 225, 225, 225));  newgame.setCharacterSize(40); window.draw(newgame);
    }

    Text resume; resume.setFont(font); resume.setString("resume");  resume.setPosition(globalcenter.x + 50, globalcenter.y - 150); FloatRect collesion2 = resume.getGlobalBounds();
    if (collesion2.contains(MousePos))
    {
        resume.setFillColor(Color(100, 100, 100, 100));  resume.setCharacterSize(45); window.draw(resume);
        if (Mouse::isButtonPressed(Mouse::Left))
        {           
            if (Player_Health <= 0)
            {
                current_level = 1;
                Player.setPosition(500, 500);
                Player_Health = 100;
                Score = 0;
                menu_num = 0;
                zombies.clear();
                TotalSpawnedZombies = 0;
                PortalOpen = false;
                smg_buy = false;
                shotgun_buy = false;
                speed_pow = false;
                reload_pow = false;
                sniper_buy = false;
                speedmulti = 1;
                reloadmulti = 1;
                Money = 0;
                Wall_Bounds.clear();
                HealthPacks.clear();
                bloodeffects.clear();
                Curr_Gun_state = Pistol;
                playerspeed = 175;
                SwtichCurrentWallBounds();
                Current_Wave1 = 0;
                MusicPlayer.stop();
                current_song = 0;
                MusicPlayer.play();
                pistolbulletsloaded = 9;
                riflebulletsloaded = 30;
                shotgunbulletsloaded = 8;
                sniperbulletsloaded = 5;
            }
            else
            {
                ifstream inf("savegame.txt");
                inf >> current_level;
                inf >> Player_Health;
                inf >> Score;
                inf >> highest_score;
                inf.close();
                zombies.clear();
                TotalSpawnedZombies = 0;
                PortalOpen = false;
                smg_buy = false;
                shotgun_buy = false;
                speed_pow = false;
                reload_pow = false;
                sniper_buy = false;
                speedmulti = 1;
                reloadmulti = 1;
                Money = 0;
                Wall_Bounds.clear();
                HealthPacks.clear();
                bloodeffects.clear();
                Curr_Gun_state = Pistol;
                playerspeed = 175;
                SwtichCurrentWallBounds();
                Current_Wave1 = 0;
                MusicPlayer.stop();
                current_song = 0;
                MusicPlayer.play();
                pistolbulletsloaded = 9;
                riflebulletsloaded = 30;
                shotgunbulletsloaded = 8;
                sniperbulletsloaded = 5;
            }
            menu_num = 0;
        }
    }
    else
    {
        resume.setFillColor(Color(225, 225, 225, 225)); resume.setCharacterSize(40); window.draw(resume);
    }

}

void Credits(Font font)
{
    Text first; first.setFont(font); first.setString("Abdullah Sheriff"); first.setFillColor(Color(225, 225, 225, 225)); first.setPosition(globalcenter.x - 300, globalcenter.y - 300); first.setCharacterSize(32); window.draw(first);
    Text second; second.setFont(font); second.setString("Abdelrahman Ahmed Saber"); second.setFillColor(Color(225, 225, 225, 225)); second.setPosition(globalcenter.x - 300, globalcenter.y - 200); second.setCharacterSize(32); window.draw(second);
    Text third; third.setFont(font); third.setString("Abdelrahman Ahmed Ezzat"); third.setFillColor(Color(225, 225, 225, 225)); third.setPosition(globalcenter.x - 300, globalcenter.y - 100); third.setCharacterSize(32); window.draw(third);
    Text fourth; fourth.setFont(font); fourth.setString("Abdelrahman Tamer Mohamed"); fourth.setFillColor(Color(225, 225, 225, 225)); fourth.setPosition(globalcenter.x - 300, globalcenter.y); fourth.setCharacterSize(32); window.draw(fourth);
    Text fifth; fifth.setFont(font); fifth.setString("Shahd Hani"); fifth.setFillColor(Color(225, 225, 225, 225)); fifth.setPosition(globalcenter.x - 300, globalcenter.y + 100); fifth.setCharacterSize(32); window.draw(fifth);
    Text sixth; sixth.setFont(font); sixth.setString("Mohamed Magdy"); sixth.setFillColor(Color(225, 225, 225, 225)); sixth.setPosition(globalcenter.x - 300, globalcenter.y + 200); sixth.setCharacterSize(32); window.draw(sixth);
}

void Exit(Font font)
{
    Text escape; escape.setFont(font); escape.setString(" do you want to exit "); escape.setFillColor(Color(225, 225, 225, 225)); escape.setPosition(globalcenter.x - 200, globalcenter.y - 250); escape.setCharacterSize(32); window.draw(escape);

    Text no; no.setFont(font); no.setString("no"); no.setPosition(globalcenter.x - 150, globalcenter.y - 150); FloatRect collesion1 = no.getGlobalBounds();
    if (collesion1.contains(MousePos))
    {
        no.setFillColor(Color(100, 100, 100, 100));  no.setCharacterSize(45); window.draw(no);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 1;
        }
    }
    else
    {
        no.setFillColor(Color(225, 225, 225, 225));  no.setCharacterSize(40); window.draw(no);
    }

    Text yes; yes.setFont(font); yes.setString("yes"); yes.setPosition(globalcenter.x + 50, globalcenter.y - 150); FloatRect collesion2 = yes.getGlobalBounds();
    if (collesion2.contains(MousePos))
    {
        yes.setFillColor(Color(100, 100, 100, 100));  yes.setCharacterSize(40); window.draw(yes);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            window.close();
        }
    }
    else
    {
        yes.setFillColor(Color(225, 225, 225, 225));  yes.setCharacterSize(32); window.draw(yes);
    }
}

void Pause(Font font)
{
    pause_menu.setTexture(Pause_menu);
    pause_menu.setPosition(globalcenter.x - 85, globalcenter.y - 150);
    window.draw(pause_menu);
    {
        Text resume; resume.setFont(font); resume.setString("continue"); resume.setPosition(globalcenter.x - 40, globalcenter.y - 120); FloatRect collesion1 = resume.getGlobalBounds();
        if (collesion1.contains(MousePos))
        {
            resume.setFillColor(Color(100, 100, 100, 100));  resume.setCharacterSize(30); window.draw(resume);
            if (Mouse::isButtonPressed(Mouse::Left))
            {
                menu_num = 0;
            }
        }
        else
        {
            resume.setFillColor(Color(225, 225, 225, 225));  resume.setCharacterSize(25); window.draw(resume);
        }


        Text exit; exit.setFont(font); exit.setString("exit");  exit.setPosition(globalcenter.x - 10, globalcenter.y - 30); FloatRect collesion2 = exit.getGlobalBounds();
        if (collesion2.contains(MousePos))
        {
            exit.setFillColor(Color(100, 100, 100, 100));  exit.setCharacterSize(30); window.draw(exit);
            if (Mouse::isButtonPressed(Mouse::Left))
            {
                ofstream outf("savegame.txt");
                outf << current_level << endl;
                outf << Player_Health << endl;
                outf << Score << endl;
                outf.close();
                menu_num = 1;
            }
        }
        else
        {
            exit.setFillColor(Color(225, 225, 225, 225)); exit.setCharacterSize(25); window.draw(exit);
        }
    }
}

void open_menu(Font font)
{
    Vector2i pixelpos = Mouse::getPosition(window);
    MousePos = window.mapPixelToCoords(pixelpos);
    /*if (curr_state==death)
        menu_num = 6;*/
    switch (menu_num)
    {
    case 1:game_openning_menu(font); break;
    case 2:Start(font); break;
    case 3:Credits(font); break;
    case 4:Exit(font); break;
    case 5:Pause(font); break;
    case 6:Game_over(font); break;
    case 7:Controls(font);
    }

}

void switch_menu()
{
    switch (menu_num)
    {
    case 0:menu_num = 5; break;
    case 2:
    case 3:
    case 7:
    case 4:menu_num = 1; break;
    }

}

void Menu_Background(Font font)
{
    menu_background.setTexture(Menu_background);
    if (menu_num == 5 || menu_num == 6)
    {
        Draw();
    }
    else
    {
        menu_background.setScale(Vector2f(0.65, 0.65));
        menu_background.setPosition(globalcenter.x - (960 * 0.65), globalcenter.y - (540 * 0.65));
        window.draw(menu_background);

        Text high_score; high_score.setFont(font); high_score.setString(" high score : " + to_string(highest_score)); high_score.setFillColor(Color(225, 225, 225, 225)); high_score.setPosition(globalcenter.x + 300, globalcenter.y - 310); high_score.setCharacterSize(32); window.draw(high_score);

    }

}

void Game_over(Font font)
{
    Clock clock;
    clock.getElapsedTime().asSeconds();

    end_game.setTexture(End_game);
    end_game.setPosition(globalcenter.x - 200, globalcenter.y - 210);
    window.draw(end_game);
    Text over; over.setFont(font); over.setString(" GAME OVER "); over.setFillColor(Color(225, 225, 225, 225)); over.setPosition(globalcenter.x - 120, globalcenter.y - 160); over.setCharacterSize(32); window.draw(over);
    Text back; back.setFont(font); back.setString("back");  back.setPosition(globalcenter.x - 25, globalcenter.y - 95); FloatRect collesion2 = back.getGlobalBounds();
    if (collesion2.contains(MousePos))
    {
        back.setFillColor(Color(100, 100, 100, 100));  back.setCharacterSize(30); window.draw(back);
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            if (highest_score < Score)
            {
                highest_score = Score;
            }
            ofstream outf("savegame.txt");
            outf << 1 << endl;
            outf << 100 << endl;
            outf << 0 << endl;
            outf << highest_score;
            outf.close();
            menu_num = 1;
        }
    }
    else
    {
        back.setFillColor(Color(225, 225, 225, 225)); back.setCharacterSize(25); window.draw(back);
    }

}

void Controls(Font font)
{
    control.setTexture(Control);

    control.setScale(Vector2f(0.65, 0.65));
    control.setPosition(globalcenter.x - (960 * 0.65), globalcenter.y - (540 * 0.65));
    window.draw(control);
}