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
#include <sstream>
#include <iomanip>


using namespace std;
//to be displayed on ui
int Player_Health = 100;
int BossHealth = 100;
int Score= 0;
int highest_score = 0;
int Money= 0;
int const speedmoney = 2000;
int const reloadmoney = 2000;
float speedmulti = 1;
float reloadmulti = 1;
int EnemiesKilledWithoutHit = 0;

int current_level = 0;

//buying weapon
bool  smg_buy = false, shotgun_buy = false, sniper_buy = false, rocket_buy = false, speed_pow = false, reload_pow = false;
int const  money_smg = 1000, money_shotgun = 1500, money_sniper = 2000, money_rocket = 5000;
//to cheak player intersects
bool smg_player_intersects = false, sniper_player_intersects = false;
bool shotgun_player_intersects = false;
bool rocket_player_intersects = false;

#define pi 3.14159265
#define Level1NumWaves 3

#define pistolfirerate 0.2f
#define pistoldamage 16
#define pistolspread 0
#define pistolbulletpershot 1
#define pistolclipsize 9
#define pistolreloadtime 1
#define Pistol_bullets_loaded_per_reload pistolclipsize
int pistolammostock = 36;
int pistolbulletsloaded = pistolclipsize;

#define riflefirerate 0.08f
#define rifledamage 15.2
#define riflespread 0
#define riflebulletpershot 1
#define rifleclipsize 30
#define riflereloadtime 1
#define Rifle_bullets_loaded_per_reload rifleclipsize
int rifleammostock = 90;
int riflebulletsloaded = rifleclipsize;

#define shotgunfirerate 0.7
#define shotgundamage 7
#define shotgunspread 1
#define shotgunbulletpershot 10
#define shotgunclipsize 8
#define shotgunreloadtime 1
#define ShotGun_bullets_loaded_per_reload 1
int shotgunammostock = 24;
int shotgunbulletsloaded = shotgunclipsize;

#define sniperfirerate 1.6f
#define sniperdamage 150
#define sniperspread 0
#define sniperbulletpershot 1
#define sniperclipsize 5
#define sniperreloadtime 2
#define sniper_bullets_loaded_per_reload sniperclipsize
int sniperammostock = 15;
int sniperbulletsloaded = sniperclipsize;

#define minigunfirerate 0.05
#define minigundamage 5
#define minigunspread 2
#define minigunbulletpershot 3
#define minigunclipsize 2000
#define minigunreloadtime 0
#define minigun_bullets_loaded_per_reload minigunclipsize
int minigunammostock = 99999999;
int minigunbulletsloaded = minigunclipsize;

#define rocketfirerate 2
#define rocketdamage 250
#define rocketspread 0
#define rocketbulletpershot 1
#define rocketclipsize 1
#define rocketreloadtime 2
#define rocket_bullets_loaded_per_reload rocketclipsize
#define rocket_radius 250
int rocketammostock = 15;
int rocketbulletsloaded = rocketclipsize;

using namespace sf;
using namespace candle;

enum state
{
    idle, walk, hit, death
};
state curr_state = state::idle;
enum Gun_State
{
    Pistol, Smg, Shotgun,Sniper,MiniGun, RocketLauncher
};
Gun_State Curr_Gun_state = Gun_State::Pistol;
enum Boss_State
{
    Walk, Death, LaserAtk, BlindAtk, BulletAtk
};
Boss_State Curr_Boss_State = Boss_State::Walk;
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
    Sprite effect;
    CircleShape explosionArea;
    Vector2f currentvelocity;
    float maxvelocity = 5000.0f;
    float damage;
    int id;
    float animation_counter = 0;
    float animation_switch_time = 0.03f;
    int bullet_image_counter = 0;

    float Fire_animation_counter = 0;
    float Fire_animation_switch_time = 0.1;
    int Fire_image_counter = 0;
    RadialLight lighteffect;
    Gun_State curr_gun_state;
    Trail guntrail;
    int zombieshit = 0;
    bool isRocket = false;
    bool DrawEffect = false;
    bool deleteme = false;
    bool IsFireBall = false;
    bool Isturret = false;
    void animation(float dt,Texture SpawnEffectanimation[],Vector2f playerpos,bool slowing,Texture FireBallAnim[])
    {
        explosionArea.setRadius(rocket_radius);
        explosionArea.setFillColor(Color(180, 0, 0, 16));
        explosionArea.setOrigin(explosionArea.getLocalBounds().width / 2, explosionArea.getLocalBounds().height / 2);
        explosionArea.setPosition(shape.getPosition());
        guntrail.addTrailSegment(shape.getPosition());
        if (curr_gun_state == Shotgun || curr_gun_state == MiniGun)
        {
            lighteffect.setIntensity(500);
            lighteffect.setRange(100);
        }
        else
        {
            lighteffect.setIntensity(200);
            lighteffect.setRange(100);
        }
        if (slowing)
        {
            lighteffect.setIntensity(1000);
            lighteffect.setRange(10);
        }
        lighteffect.setColor(Color::Yellow);
        lighteffect.setPosition(shape.getPosition());
        effect.setScale(0.3, 0.3);
        if (bullet_image_counter < 60 && DrawEffect)
        {
            effect.setPosition(playerpos.x,playerpos.y - 25);
            effect.setTexture(SpawnEffectanimation[bullet_image_counter]);
            animation_counter += dt;
            if (animation_counter >= animation_switch_time)
            {
                bullet_image_counter++;
                animation_counter = 0;
            }
        }
        else
        {
            DrawEffect = false;
            bullet_image_counter = 0;
            animation_counter = 0;
        }
        if (IsFireBall)
        {
            shape.setTexture(FireBallAnim[Fire_image_counter]);
            Fire_animation_counter += dt;
            if (Fire_animation_counter > Fire_animation_switch_time)
            {
                Fire_animation_counter = 0;
                Fire_image_counter++;
                if (Fire_image_counter >4)
                {
                    Fire_image_counter = 0;
                }
            }
        }
    }
};
bool ishit = false;
SoundBuffer Rocket_Explosion_Sound;
Sound ExplosionSound;
struct Enemy
{
    Sprite shape;
    Sprite SpawnEffect;
    CircleShape explosionArea;
    RadialLight SpawnLight;
    Vector2f currentvelocity;
    int last_hit_bullet_id = -1;
    float maxvelocity = 175;
    bool isdeath = false;
    bool isEnemyhit = false;
    bool remove_Enemy = false;
    int type = 0;
    float Spawn_animation_counter = 0;
    float Spawn_animation_duration = 0.15f;
    int Spawn_imagecounter = 0;
    int health = 75;
    Vector2f norm_Direction;
    virtual void EnemyBehaviour(Texture hit_anim[], Texture atk_anim[], Texture walk_anim[], Texture death_anim[], bool dashing, float dt, Vector2f player_pos, Vector2f Boss_pos) {}
    void SetSpawnLocation()
    {
        SpawnEffect.setPosition(shape.getPosition());
        SpawnEffect.setScale(2, 2);
        SpawnLight.setPosition(shape.getPosition());
        SpawnLight.setRange(200);
        SpawnLight.setIntensity(150);
        SpawnLight.setColor(Color(252, 159, 159));
    }
    void SpawnAnimation(Texture Spawn_anim[],float dt)
    {
        if (current_level != 4)
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
    }
};
struct Zombie : public Enemy
{
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


    float distance_from_player;
    float attackdistance = 75.0f;
    bool atkready = false;
    bool isattacking = false;
    float damage = 6;

    int maxwalkframes = 8;
    int maxhitframes = 2;
    int maxdeathframes = 4;
    void EnemyBehaviour(Texture hit_anim[],Texture atk_anim[],Texture walk_anim[],Texture death_anim[], bool dashing, float dt,Vector2f player_pos, Vector2f Boss_pos) override
    {
        if (isEnemyhit && !isdeath)
        {
            Hit_animation_counter += dt;
            shape.setTexture(hit_anim[Hit_imagecounter]);
            if (Hit_animation_counter >= Hit_animation_duration)
            {
                Hit_animation_counter = 0;
                Hit_imagecounter++;
                if (Hit_imagecounter >= maxhitframes)
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
                atkready = false;
                if (imagecounter >= maxwalkframes)
                {
                    if (isattacking)
                    {
                        atkready = true;
                    }
                    imagecounter = 0;
                }
            }
            Vector2f Direction = player_pos - shape.getPosition();
            float magnitude = sqrt(Direction.x * Direction.x + Direction.y * Direction.y);
            norm_Direction = Direction / magnitude;
            distance_from_player = magnitude;
            if (distance_from_player <= attackdistance)
            {
                isattacking = true;
                shape.setTexture(atk_anim[imagecounter]);
                if (atkready && !dashing)
                {
                    Player_Health -= damage;
                    EnemiesKilledWithoutHit = 0;
                    ishit = true;
                    atkready = false;
                }
                currentvelocity = Vector2f(0, 0);
            }
            else
            {
                shape.setTexture(walk_anim[imagecounter]);
            }
        }
        else if(isdeath)
        {
            Death_animation_counter += dt;
            shape.setTexture(death_anim[Death_imagecounter]);
            if (Death_animation_counter >= Death_animation_duration)
            {               
                if (Death_imagecounter <= maxdeathframes)
                {
                    Death_imagecounter++;
                    Death_animation_counter = 0;
                }
                if (Death_animation_counter > maxdeathframes)
                {
                    remove_Enemy = true;
                }
            }
        }
        if (!isdeath)
        {
            if (sqrt(currentvelocity.x * currentvelocity.x + currentvelocity.y * currentvelocity.y) < maxvelocity)
            {
                currentvelocity = Vector2f(currentvelocity.x += maxvelocity * dt, currentvelocity.y += maxvelocity * dt);
            }
            else
            {
                currentvelocity = Vector2f(currentvelocity.x += -(currentvelocity.x / abs(currentvelocity.x)) * dt * 500, currentvelocity.y += -(currentvelocity.y / abs(currentvelocity.y)) * dt * 500);
            }
            shape.move(Vector2f(currentvelocity.x * norm_Direction.x * dt, currentvelocity.y * norm_Direction.y * dt));
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
vector<shared_ptr<Enemy>> Enemies;
Texture ammo_smg_photo;
Texture Health_Texture;
vector<Sprite> HealthPacks;
vector<Sprite> AmmoPacks;
vector <BloodEffect> bloodeffects;
void SpawnBlood(Enemy& enemy)
{
    BloodEffect newbloodeffect;
    newbloodeffect.BloodShape.setPosition(enemy.shape.getPosition());
    switch (enemy.type)
    {
    case 0:
    case 1:newbloodeffect.BloodShape.setScale(enemy.shape.getScale().x * -0.5f, enemy.shape.getScale().y * 0.5f); break;
    case 2:newbloodeffect.BloodShape.setScale(enemy.shape.getScale().x * -1.0f, enemy.shape.getScale().y * 1.0f); break;
    }
    newbloodeffect.assign_random_number();
    bloodeffects.push_back(newbloodeffect);
    if (enemy.health <= 0)
    {
        EnemiesKilledWithoutHit++;
        int random_num = rand() % 100;
        enemy.isdeath = true;
        if (random_num >= 89 && random_num <94)
        {
            Sprite newhealthpack;
            newhealthpack.setTexture(Health_Texture);
            newhealthpack.setPosition(enemy.shape.getPosition());
            HealthPacks.push_back(newhealthpack);

        }
        else if (random_num >= 94)
        {
            Sprite newammostock;
            newammostock.setTexture(ammo_smg_photo);
            newammostock.setPosition(enemy.shape.getPosition());
            AmmoPacks.push_back(newammostock);
        }
        switch (enemy.type)
        {
        case 0:Score += 25 * (float(1 + floor(EnemiesKilledWithoutHit / 10.0f) / 10)); Money += 75;  break;
        case 1:Score += 50 * (float(1 + floor(EnemiesKilledWithoutHit / 10.0f) / 10)); Money += 100; break;
        case 2:Score += 100 * (float(1 + floor(EnemiesKilledWithoutHit / 10.0f) / 10)); Money += 225; break;
        }
    }
}
float camera_shake_counter = 0;
int camera_shake_magnitude = 10;
float camera_shake_duration = 1;
struct Explosion
{
    int expo_image_counter = 0;
    float expo_animation_counter = 0;
    float expo_animation_switch_time = 0.015;
    Sprite shape;
    bool deleteme = false;
    void ExpoAnimation(Texture ExpoAnimation[], float dt)
    {
        shape.setOrigin(shape.getLocalBounds().width / 2, shape.getLocalBounds().height / 2);
        if (expo_image_counter < 52)
        {
            shape.setTexture(ExpoAnimation[expo_image_counter]);
            expo_animation_counter += dt;
            if (expo_animation_counter >= expo_animation_switch_time)
            {
                expo_image_counter++;
                expo_animation_counter = 0;
            }
        }
        else
        {
            deleteme = true;
        }
    }
    void explosionlogic(Vector2f Expo_Pos, Vector2f Player_pos,Vector2f Boss_pos,float damage,float radius,bool isdashing)
    {
        camera_shake_counter += 1;
        if (camera_shake_counter > camera_shake_duration)
        {
            camera_shake_counter = camera_shake_duration;
        }
        ExplosionSound.setBuffer(Rocket_Explosion_Sound);
        ExplosionSound.play();
        shape.setPosition(Expo_Pos);
        for (int k = 0; k < Enemies.size(); k++)
        {
            if (!Enemies[k]->isdeath)
            {
                float distance = sqrt((Enemies[k]->shape.getPosition().x - Expo_Pos.x) * (Enemies[k]->shape.getPosition().x - Expo_Pos.x) + (Enemies[k]->shape.getPosition().y - Expo_Pos.y) * (Enemies[k]->shape.getPosition().y - Expo_Pos.y));
                if (distance <= radius)
                {
                    Enemies[k]->currentvelocity = -Vector2f(Enemies[k]->currentvelocity.x + 100, Enemies[k]->currentvelocity.y + 100);
                    Enemies[k]->health -= damage;
                    SpawnBlood(*Enemies[k]);
                }
            }
        }
        float distance = sqrt((Player_pos.x - Expo_Pos.x) * (Player_pos.x - Expo_Pos.x) + (Player_pos.y - Expo_Pos.y) * (Player_pos.y - Expo_Pos.y));
        if (distance <= radius && !isdashing)
        {
            Player_Health -= rocketdamage * 0.05;
            EnemiesKilledWithoutHit = 0;
        }
        float Boss_distance = sqrt((Boss_pos.x - Expo_Pos.x) * (Boss_pos.x - Expo_Pos.x) + (Boss_pos.y - Expo_Pos.y) * (Boss_pos.y - Expo_Pos.y));
        if (Boss_distance <= radius)
        {
            BossHealth -= damage * (15 / distance);
        }
    }
};
vector<Explosion> explosions;
struct SkullFire : public Enemy
{
    float animation_counter = 0;
    float atkreadycounter = 0;
    float animation_duration = 0.20f;
    int imagecounter = 0;
    float distance_from_player;
    bool atkready = false;
    void EnemyBehaviour(Texture hit_anim[], Texture atk_anim[], Texture walk_anim[], Texture death_anim[], bool dashing, float dt, Vector2f player_pos,Vector2f Boss_pos) override
    {
        if (!isdeath)
        {          
            animation_counter += dt;
            if (atkreadycounter < 2)
            {
                atkreadycounter += dt;
            }
            else
            {
                atkready = true;
            }
            if (animation_counter >= animation_duration)
            {
                animation_counter = 0;
                imagecounter++;
                if (imagecounter >= 8)
                {
                    imagecounter = 0;
                }
            }
            shape.setTexture(walk_anim[imagecounter]);
            Vector2f Direction = player_pos - shape.getPosition();
            float magnitude = sqrt(Direction.x * Direction.x + Direction.y * Direction.y);
            norm_Direction = Direction / magnitude;
            distance_from_player = magnitude;
            if (sqrt(currentvelocity.x * currentvelocity.x + currentvelocity.y * currentvelocity.y) < maxvelocity)
            {
                currentvelocity = Vector2f(currentvelocity.x += maxvelocity * dt, currentvelocity.y += maxvelocity * dt);
            }
            else
            {
                currentvelocity = Vector2f(currentvelocity.x += -(currentvelocity.x / abs(currentvelocity.x)) * dt * 500, currentvelocity.y += -(currentvelocity.y / abs(currentvelocity.y)) * dt * 500);
            }
            shape.move(Vector2f(currentvelocity.x * norm_Direction.x * dt, currentvelocity.y * norm_Direction.y * dt));
            if (magnitude <= 75 &&atkready)
            {
                isdeath = true;
            }
        }
        else if (isdeath)
        {
            Explosion newexplosion;
            newexplosion.explosionlogic(shape.getPosition(), player_pos, Boss_pos, 250, rocket_radius, dashing);
            explosions.push_back(newexplosion);
            remove_Enemy = true;
        }
    }
};
struct MuzzleFlashEffect
{
    RadialLight MuzzleEffect;
    float counter = 1;
    bool deleteme = false;
    float muzzlerange = 50;
    bool Isturret = false;
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
struct LaserBeam
{
    Sprite laserSprite;
    Vector2f start, end;
    bool secondphase = false;
    bool deleteme = false;
    float counter = 0;
    RadialLight beamLight;
    float angle;
    void LaserBeamLogic(Texture LaserTexture)
    {
        float length = std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2));
        angle = std::atan2(end.y - start.y, end.x - start.x) * 180 / pi;
        laserSprite.setOrigin(0, LaserTexture.getSize().y / 2.f);
        laserSprite.setPosition(start);
        laserSprite.setRotation(angle);
        laserSprite.setScale(length / LaserTexture.getSize().x, 1);
    }
    void LaserBeamAnimation(float dt)
    {
        beamLight.setColor(Color::Red);
        beamLight.setRange(300);
        beamLight.setIntensity(200);
        beamLight.setPosition(end);
        counter += dt;
        if (laserSprite.getScale().y <= 2 && !secondphase)
        {           
            laserSprite.setScale(laserSprite.getScale().x, laserSprite.getScale().y + dt * 2);
        }
        else
        {
            secondphase = true;
        }
        if (laserSprite.getScale().y >= 1 && secondphase)
        {
            laserSprite.setScale(laserSprite.getScale().x, laserSprite.getScale().y - dt * 2);
        }
        if (counter > 1)
        {
            deleteme = true;
        }
    }
};
vector<LaserBeam> LaserBeams;
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
void TurretLogic();
void TurretAbility();

void calculate_shoot_dir(); // function that calculates the normal direction between the player and the current mouse position
void buying_weapons(); //function that  open guns for player 
void select_guns(); // function that switches between guns
void Switch_Current_Gun_Attributes(); //function that manages the current held gun attributes, i.e: the current gun fire rate is 1, current gun spread is 0,etc
void Shooting(); //function that handles shooting and spawning new bullets
void SpawnBlood(Enemy& enemy);
void Bullet_Movement_Collision(float dt); //function that handles bullet collision
void Guns_Animation_Handling();
void Gun_switch_cooldown();
void Gun_UpdateAnimationCounter(float dt, int maximagecounter, double switchtime); // function that handles gun animations
void camera_shake();

void SpawnZombiesWaves(float dt);
void HandleZombieBehaviour(float dt);

//bossfight functions
void BossAnimationCounter(int maximagecounter, bool isonce);
void BossAnimationHandler();
void BossUpdateStuff(float dt);
void BossMovement(float dt);
void BossAbilityRoller();
void BossAbilitySelector(float dt);
void BossLaserBeam(float dt);
void BossBlind(float dt);
void BossBulletHell(float dt);
void BossSpawnEnemies();

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

//level 4 functions
void Wall4();

void Draw(); // the main drawing function where everything gets drawn on screen
void Combo();
void WeaponsBuyDrawing();
void UI();

void Dialogue();

//menu
void StartNewGame();

void game_openning_menu();
void Credits(Font font);
void Exit();
void Pause();
void open_menu();
void switch_menu();
void Menu_Background();
void Game_over();
void Controls();

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
Text money_Rocket_text;
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
Sprite RocketL_S;
Sprite pistol_buying;
Sprite smg_buying;
Sprite shotgun_buying;
Sprite sniper_buying;
Sprite Rocket_buying;
Sprite money;
Sprite speedmachine;
Sprite reloadmachine;
Sprite Crosshair;
Sprite LockinCrosshair;
Sprite Portal_S;
Sprite void1[4];
Sprite slow_ability;
Sprite MiniGun_ability;
Sprite Turret_ability;
Sprite SpeedCola_S;
Sprite StaminaUp_S;
Sprite Turret;
Sprite DialogueBox;
RectangleShape DashOrigin(Vector2f(50.0f, 50.0f));
ContextSettings settings;
RenderWindow window(VideoMode(1920, 1080), "ZombieGame", Style::Fullscreen,settings);


//menu
Texture StartButtonTexture;
Sprite startButton;
Texture CreditsButtonTexture;
Sprite creditsButton;
Texture ExitButtonTexture;
Sprite exitButton;
Texture ControlsButtonTexture;
Sprite controlsButton;
Texture PlayButtonTexture;
Sprite PlayButton;
Texture EscapeButtonTexture;
Sprite escapeButton;
Texture NoButtonTexture;
Sprite noButton;
Texture YesButtonTexture;
Sprite yesButton;
Texture ResumeButtonTextures;
Sprite resumeButton;
Texture GameOverButtonTexture;
Sprite overButton;
Texture BackButtonTexture;
Sprite backButton;
Texture HighScoreTexture;
Sprite HighScoreButton;
//winning screen
Text winningText;
Color WinningColor;
Text WinningBackButton;

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
//turret booleans
bool isTurretActive = false;
bool isTurretReady = true;
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

bool isComboAnimation = false;
bool Isincreasing = true;
//wall bounds vector to detect collision
vector <FloatRect> Wall_Bounds;
vector <FloatRect> Wall_Bounds1;
vector <FloatRect> Wall_Bounds2;
vector <FloatRect> Wall_Bounds3;
vector <FloatRect> Wall_Bounds4;

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

Texture RocketLauncher_Image;

Texture bullet_Texture;

Texture zombie_walk_animation[8];
Texture zombie_atk_animation[7];
Texture zombie_hit_animation[2];
Texture zombie_death_animation[6];

Texture Golem_walk_animation[8];
Texture Golem_atk_animation[8];
Texture Golem_hit_animation[4];
Texture Golem_death_animation[10];

Texture SkullFire_animations[8];

Texture Zombie_spawn_animation[7];
Texture zombie_hit_blood_effect[10];

Texture RocketSpawnAnimation[60];
Texture RocketExplosionAnimation[52];

Texture TurretAnimation[10];

Texture CrossHair_Texture;


Texture pistol_photo;
Texture smg_photo;
Texture shotgun_photo;
Texture sniper_photo;
Texture money_photo;
Texture speedmachine_photo;
Texture reloadmachine_photo;

Texture slow_ability_photo;
Texture minigun_ability_photo;
Texture Turret_ability_photo;

Texture SpeedCola_T;
Texture StaminaUp_T;

Texture Void;

Texture DialogueBox_T;

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
float turret_counter = 0;
float knockbackmag = 0;
float Combo_timer_counter = 0;
float winnerAlpha = 0;
//firerate counter
float fire_rate_counter;
int previouskilledenemiescombo;
//player speed and direction(x,y)
Vector2f MovementDirection(0, 0);
//combo stuff
int ComboIncrease = 0;

//turret stuff

Vector2f TurretShootDir;
Vector2f TurretShootDir_norm;
Vector2f turretShootPoint;
float ClosestEnemyDistance = 100;
float TurretBulletDamage = 20;
float turretFireRateCounter = 0;
float turretFireRate = 0.1;
float turretAnimationCounter = 0;
int turretimagecounter = 0;
bool turretLockedin = false;
shared_ptr<Enemy> currentLockedonEnemey;

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

SoundBuffer Rocket_Shoot_Sound;
SoundBuffer Rocket_Reload_Sound;

SoundBuffer TurretShoot_sound;

Sound ReloadSound;
Sound ShootSound;

SoundBuffer* Current_shoot_Buffer;
SoundBuffer* Current_reload_Buffer;

SoundBuffer GameOver_buffer;
SoundBuffer Combo_buffer;

SoundBuffer BossLaserSound;
SoundBuffer BossFireBallSound;
SoundBuffer BossSpawnEnemiesSound;
SoundBuffer BossBlindSound;

SoundBuffer VictoryTheme;

SoundBuffer AbilityReadSound;

Sound EffectsPlayer;
Sound TurretSoundPlayer;
Sound BossSoundPlayer;

SoundBuffer music[5];
Sound MusicPlayer;
bool music_trigger = false;

int current_song = 0;
//zombie variables
int Current_Wave1 = 1;
int TotalSpawnedZombies = 0;
bool canspawn = true;

VertexArray dasheline(Quads);

vector<bullet> bullets;
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

ostringstream ss;
View view(Vector2f(0, 0), Vector2f(window.getSize().x, window.getSize().y));

//Boss Variables
Texture BossLevel_T;
Sprite BossLevel;
Sprite Boss;
Vector2f Boss_move_dir;
float BossMaxSpeed = 100;
float distancefromplayer;
bool isCasting = false;
int RandomAbility = 5;
Texture laserTex;
int NumberOfLaserShots = 5;
int LaserShotsFired = 0;
float lasercounter = 0;
float laserFreq = 0.3;
bool LockingIn = false;
Vector2f endpoint;
CircleShape explosionArea;
bool IsBlind = false;
float blindcounter = 0;
float blindduration = 6;
float CurrentVolume = 80;
float blindVolumeCounter = 0;
Texture FireBallAnimation[5];
float Boss_FireRate_counter = 0;
float Boss_Fire_Rate = 0.7;
int Boss_NumberofBulletsToShoot = 15;
Vector2f Boss_Player_dir;
Vector2f Boss_Player_Norm_dir;
int numberofbulletsegments = 15;
float RadiusOfBulletRing = 20;
float fireballspeed = 1500;
int Fireball_Damage = 5;
int numberofbulletwaves = 6;
int currentBulletwaves = 0;
float countertest = 0;
float ability_counter = 0;
float Abilities_freq = 2;
bool Ability_Ready = true;
bool IsBossDead = false;
Texture Boss_walk_animation[8];
Texture Boss_LaserAtk_animation[13];
Texture Boss_BulletsAtk_animation[13];
Texture Boss_BlindAtk_animation[17];
Texture Boss_Death_animation[9];
float Boss_animation_counter = 0;
float Boss_animation_switchtime = 0.2;
int Boss_image_counter = 0;
bool Boss_animation_done_once = false;
int SelectedAbility = 3;
int previousSelected = 999;
bool BossDone = false;

//Dialogue Variables

bool IsDialogueActive = false;
bool finishedCurrentDialogue = false;
float DialogueCounter = 0;
float DialogueSpeed = 0.05;
int letterindex = 0;
int current_dialogue = 0;
Text Current_D_Text;

string Current_String;
string DisplayedString;
string Boss_D_Intro = "Well Well Well....You Passed all my trials, Perhaps I underestimated you...";
string Player_D_Intro = "Your reign of terror ends now!";
string Boss_D_Outro = "TRY AND STOP ME!";

LightingArea ambientlight(candle::LightingArea::FOG,Vector2f(0,0),Vector2f(1920, 1920));
float playerdeltatime;
int main()
{
    init_walls();
    GetTextures();
    settings.majorVersion = 3;
    settings.minorVersion = 2;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    ReloadSound.setVolume(CurrentVolume);
    ShootSound.setVolume(CurrentVolume);
    ExplosionSound.setVolume(CurrentVolume);
    TurretSoundPlayer.setVolume(CurrentVolume);
    ambientlight.setAreaOpacity(0.3);
    ambientlight.setAreaColor(Color::Black);
    Player.setTexture(WalkAnimation[0]);
    Player.setPosition(Vector2f(500, 500));
    Boss.setPosition(1000, 1000);
    Gun.setOrigin(Player.getOrigin());
    Clock clock;
    Event event;
    window.setMouseCursorVisible(false);
    view.zoom(0.65);
    WinningColor = Color(136, 8, 8);
    WinningColor.a = 0;
    SwtichCurrentWallBounds();
    ifstream inf("savegame.txt");
    inf >> highest_score;
    inf.close();   
    MusicPlayer.setBuffer(music[3]);
    MusicPlayer.play();
    while (window.isOpen()) {
        float elapsed = clock.restart().asSeconds();
        playerdeltatime = elapsed;
        elapsed *= slow_multi;
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
            Menu_Background();
            open_menu();
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
    //level 4
    Wall4();
}
void GetTextures()
{
    lantren.loadFromFile("lantren 1.png");
    Tlantren_Lwall2.loadFromFile("lantren_2_Lwall.png");
    Tlantren_Rwall2.loadFromFile("lantren_2_Rwall.png");
    Tlantren_Mwall2.loadFromFile("lantren_2_Mwall.png");
    BossLevel_T.loadFromFile("map.png");
    BossLevel.setTexture(BossLevel_T);
    laserTex.loadFromFile("Beam.png");
    DialogueBox_T.loadFromFile("DialogueBox.png");
    DialogueBox.setTexture(DialogueBox_T);
    DialogueBox.setScale(0.9, 0.5);
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
    bullet_Texture.loadFromFile("guns/bullet animation/tile0.png");
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
    for (int i = 0; i < 8; i++)
    {
        Golem_walk_animation[i].loadFromFile("GolemAnimation/walk/tile (" + std::to_string(i+1) + ").png");
    }
    for (int i = 0; i < 8; i++)
    {
        Golem_atk_animation[i].loadFromFile("GolemAnimation/Atk/tile (" + std::to_string(i+1) + ").png");
    }
    for (int i = 0; i < 10; i++)
    {
        Golem_death_animation[i].loadFromFile("GolemAnimation/death/tile (" + std::to_string(i+1) + ").png");
    }
    for (int i = 0; i < 4; i++)
    {
        Golem_hit_animation[i].loadFromFile("GolemAnimation/hit/tile (" + std::to_string(i+1) + ").png");
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
    for (int i = 0; i < 60; i++)
    {
        RocketSpawnAnimation[i].loadFromFile("Effect_Kabooms_1/frame (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 52; i++)
    {
        RocketExplosionAnimation[i].loadFromFile("Effect_Explosion2_1/frame (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 8; i++)
    {
        SkullFire_animations[i].loadFromFile("SkullFace/frame (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 5; i++)
    {
        FireBallAnimation[i].loadFromFile("FireBall/tile (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 8; i++)
    {
        Boss_walk_animation[i].loadFromFile("Boss/walk/tile (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 13; i++)
    {
        Boss_LaserAtk_animation[i].loadFromFile("Boss/laserAtk/tile (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 13; i++)
    {
        Boss_BulletsAtk_animation[i].loadFromFile("Boss/bulletAtk/tile (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 17; i++)
    {
        Boss_BlindAtk_animation[i].loadFromFile("Boss/BlindAtk/tile (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 9; i++)
    {
        Boss_Death_animation[i].loadFromFile("Boss/Death/tile (" + std::to_string(i + 1) + ").png");
    }
    for (int i = 0; i < 10; i++)
    {
        TurretAnimation[i].loadFromFile("Turret/tile (" + std::to_string(i + 1) + ").png");
    }
    MiniGun_Image.loadFromFile("minigun.png");
    RocketLauncher_Image.loadFromFile("RocketLauncher.png");
    Rocket_buying.setTexture(RocketLauncher_Image);
    Rocket_buying.setScale(4, 4);
    CrossHair_Texture.loadFromFile("weapons/crosshair.png");
    Crosshair.setTexture(CrossHair_Texture);
    Crosshair.setScale(0.3, 0.3);
    LockinCrosshair.setTexture(CrossHair_Texture);
    //LockinCrosshair.setScale(0.5, 0.5);
    LockinCrosshair.setColor(Color(170,0,0,128));
    Health_Texture.loadFromFile("health-red_32px.png");

    speedmachine_photo.loadFromFile("speedvanding.png");
    reloadmachine_photo.loadFromFile("ReloadVanding.png");
    speedmachine.setTexture(speedmachine_photo);
    reloadmachine.setTexture(reloadmachine_photo);
    //ui
    normal_font.loadFromFile("Okami.otf");
    ammo_smg_photo.loadFromFile("ammo_smg.png");
    money_photo.loadFromFile("money.png");
    pistol_photo.loadFromFile("pistol.png");
    smg_photo.loadFromFile("smg.png");
    shotgun_photo.loadFromFile("shotgun.png");
    shotgun_buying.setTexture(shotgun_photo);
    sniper_photo.loadFromFile("sniper.png");
    sniper_buying.setTexture(sniper_photo);
    Void.loadFromFile("void.jpg");

    slow_ability_photo.loadFromFile("timer.png");
    slow_ability.setTexture(slow_ability_photo);
    slow_ability.setScale(0.25, 0.25);

    minigun_ability_photo.loadFromFile("minigun icon.png");
    MiniGun_ability.setTexture(minigun_ability_photo);
    MiniGun_ability.setScale(0.05, 0.05);

    Turret_ability_photo.loadFromFile("TurretIcon.png");
    Turret_ability.setTexture(Turret_ability_photo);
    Turret_ability.setScale(0.25, 0.25);

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

    Rocket_Shoot_Sound.loadFromFile("RocketShootSound.wav");
    Rocket_Reload_Sound.loadFromFile("RocketReloadSound.wav");
    Rocket_Explosion_Sound.loadFromFile("RocketExplosionSound.wav");

    TurretShoot_sound.loadFromFile("TurretShoot.wav");

    BossLaserSound.loadFromFile("Boss/LaserShootSound.wav");
    BossBlindSound.loadFromFile("Boss/BlindAttackSound.wav");
    BossFireBallSound.loadFromFile("Boss/FireBallAttack.wav");
    BossSpawnEnemiesSound.loadFromFile("Boss/SpawnEnemiesSound.wav");

    GameOver_buffer.loadFromFile("GameOversoundeffect.wav");
    Combo_buffer.loadFromFile("PowerChord.wav");
    VictoryTheme.loadFromFile("VictoryTheme.wav");

    AbilityReadSound.loadFromFile("AbilityReadySound.wav");

    SpeedCola_T.loadFromFile("SpeedCola.png");
    StaminaUp_T.loadFromFile("StaminUp.png");

    SpeedCola_S.setTexture(SpeedCola_T);
    SpeedCola_S.setScale(0.05, 0.05);
    StaminaUp_S.setTexture(StaminaUp_T);
    StaminaUp_S.setScale(0.05,0.05);
    //menu
    Menu_background.loadFromFile("MenuAndUi/GameBackground.png");
    menu_background.setTexture(Menu_background);
    Pause_menu.loadFromFile("pause.png");
    End_game.loadFromFile("gameOver.png");
    Control.loadFromFile("game_controls.png");

    music[0].loadFromFile("Sad But True (Remastered).wav");
    music[1].loadFromFile("Seek & Destroy (Remastered).wav");
    music[2].loadFromFile("Metallica Shadows Follow.wav");
    music[3].loadFromFile("MainMenuMusic.wav");
    music[4].loadFromFile("Flow.wav");


    Vector2f scalefactor = Vector2f(0.2, 0.2);
    BackButtonTexture.loadFromFile("MenuAndUi/BackButton.png");
    backButton.setTexture(BackButtonTexture);
    backButton.setScale(scalefactor);
    ControlsButtonTexture.loadFromFile("MenuAndUi/ControlButton.png");
    controlsButton.setTexture(ControlsButtonTexture);
    controlsButton.setScale(scalefactor);
    CreditsButtonTexture.loadFromFile("MenuAndUi/CreditButton.png");
    creditsButton.setTexture(CreditsButtonTexture);
    creditsButton.setScale(scalefactor);
    EscapeButtonTexture.loadFromFile("MenuAndUi/ExcapeWarningText.png");
    escapeButton.setTexture(EscapeButtonTexture);
    escapeButton.setScale(0.4, 0.4);
    ExitButtonTexture.loadFromFile("MenuAndUi/ExitButton.png");
    exitButton.setTexture(ExitButtonTexture);
    exitButton.setScale(scalefactor);
    GameOverButtonTexture.loadFromFile("MenuAndUi/GameOverText.png");
    overButton.setTexture(GameOverButtonTexture);
    overButton.setScale(scalefactor);
    NoButtonTexture.loadFromFile("MenuAndUi/NoButton.png");
    noButton.setTexture(NoButtonTexture);
    noButton.setScale(scalefactor);
    PlayButtonTexture.loadFromFile("MenuAndUi/PlayButton.png");
    PlayButton.setTexture(PlayButtonTexture);
    PlayButton.setScale(scalefactor);
    ResumeButtonTextures.loadFromFile("MenuAndUi/ResumeButton.png");
    resumeButton.setTexture(ResumeButtonTextures);
    resumeButton.setScale(scalefactor);
    YesButtonTexture.loadFromFile("MenuAndUi/YesButton.png");
    yesButton.setTexture(YesButtonTexture);
    yesButton.setScale(scalefactor);
    
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
        current_player_pos = DashOrigin.getPosition();
        Vector2i pixelpos = Mouse::getPosition(window);
        MousePos = window.mapPixelToCoords(pixelpos);
        globalorigin = window.mapPixelToCoords(Vector2i(0,0));
        if (menu_num == 0 && !IsBossDead)
        {
            MusicHandler();
        }
        if (!IsDialogueActive)
        {
            Player_Movement();
            Player_Collision();

            Dashing();
        }

        if (current_level != 4)
        {
            SpawnZombiesWaves(dt);
        }
        HandleZombieBehaviour(dt);


        if (current_level == 4)
        {
            BossUpdateStuff(dt);
        }

        if (!IsDialogueActive)
        {
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
            case 4:
                TurretAbility();
                break;
            }
            if (delayfinished) { Guns_Animation_Handling(); }
            camera_shake();
        }


        previous_player_pos = current_player_pos;
        //vanding
    }

}

//music function
void MusicHandler()
{
    MusicPlayer.setVolume(CurrentVolume);
    EffectsPlayer.setVolume(CurrentVolume);
    BossSoundPlayer.setVolume(CurrentVolume);
    TurretSoundPlayer.setVolume(CurrentVolume);
    ExplosionSound.setVolume(CurrentVolume);
    ReloadSound.setVolume(CurrentVolume);
    ShootSound.setVolume(CurrentVolume);
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
        if (current_level == 4)
        {
            current_song = 4;
        }
        MusicPlayer.setBuffer(music[current_song]);
        MusicPlayer.play();
        music_trigger = true;
    }
    else if(MusicPlayer.getStatus() != Sound::Playing && music_trigger)
    {
        music_trigger = false;
        current_song++;
        if (current_song > 2)
        {
            current_song = 0;
        }
    }
}

//Player-related functions
void Player_Movement()
{
    if (Keyboard::isKeyPressed(Keyboard::A))
    {
        if (MovementDirection.x > -175 * speedmulti)
        {
            MovementDirection.x -= 1650 * playerdeltatime * speedmulti;
        }
        DashOrigin.setPosition(Player.getPosition().x + 15, Player.getPosition().y + 10);
    }
    else if (Keyboard::isKeyPressed(Keyboard::D))
    {
        if (MovementDirection.x < 175 * speedmulti)
        {
            MovementDirection.x += 1650 * playerdeltatime * speedmulti;
        }
        DashOrigin.setPosition(Player.getPosition().x - 50, Player.getPosition().y + 10);
    }
    if (Keyboard::isKeyPressed(Keyboard::W))
    {
        if (MovementDirection.y > -175 * speedmulti)
        {
            MovementDirection.y -= 1650 * playerdeltatime * speedmulti;
        }
    }
    else if (Keyboard::isKeyPressed(Keyboard::S))
    {
        if (MovementDirection.y < 175 * speedmulti)
        {
            MovementDirection.y += 1650 * playerdeltatime * speedmulti;
        }
    }
    if (MovementDirection.x < -0.1 || MovementDirection.x > 0.1)
    {
        MovementDirection.x += -(MovementDirection.x / abs(MovementDirection.x)) * playerdeltatime * 700 * speedmulti;
    }
    if (MovementDirection.y < -0.1 || MovementDirection.y > 0.1)
    {
        MovementDirection.y += -(MovementDirection.y / abs(MovementDirection.y)) * playerdeltatime * 700 * speedmulti;
    }
    DashOrigin.setPosition(DashOrigin.getPosition().x, Player.getPosition().y);
    Player.move(MovementDirection * playerdeltatime);
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
                    Player.move(-(MovementDirection.x / abs(MovementDirection.x)) * MovementDirection.x * playerdeltatime, 0);
                }
                else
                {
                    Player.move(-(MovementDirection.x / abs(MovementDirection.x)) * -MovementDirection.x * playerdeltatime, 0);
                }
            }
            else
            {
                if (Player_Bounds.top < Wall_bound.top)
                {
                    Player.move(0, -(MovementDirection.y / abs(MovementDirection.y)) * MovementDirection.y * playerdeltatime);
                }
                else
                {
                    Player.move(0, -(MovementDirection.y / abs(MovementDirection.y)) * -MovementDirection.y * playerdeltatime);
                }
            }
        }
    }
    for (int i = 0; i < bullets.size(); i++)
    {
        if (!isdashing && bullets[i].IsFireBall && bullets[i].shape.getGlobalBounds().intersects(Player.getGlobalBounds()))
        {
            Player_Health -= bullets[i].damage;
            bullets.erase(bullets.begin() + i);
            break;
        }
    }
    if (IsBlind)
    {
        if (blindcounter > blindduration)
        {
            IsBlind = false;
            blindcounter = 0;
        }
        else
        {
            blindcounter += playerdeltatime;
        }
    }
    if (CurrentVolume > 20 && IsBlind)
    {
        CurrentVolume -= playerdeltatime * 50;
    }
    if (CurrentVolume < 80 && !IsBlind)
    {
        CurrentVolume += playerdeltatime * 50;
    }
    if (ambientlight.getAreaOpacity() < 0.98 && IsBlind)
    {
        ambientlight.setAreaOpacity(ambientlight.getAreaOpacity() + playerdeltatime * 0.5);
    }
    if (!IsBlind && ambientlight.getAreaOpacity() > 0.3)
    {
        ambientlight.setAreaOpacity(ambientlight.getAreaOpacity() - playerdeltatime * 0.5);
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
            rifleammostock += 60;
            shotgunammostock += 16;
            sniperammostock += 10;
            rocketammostock += 1;
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
    if (Player.getGlobalBounds().intersects(Rocket_buying.getGlobalBounds()))
    {
        rocket_player_intersects = true;
    }
    else
        rocket_player_intersects = false;

    //vandingmachine
    if (Player.getGlobalBounds().intersects(speedmachine.getGlobalBounds()) && Money >= speedmoney && speed_pow == false && Keyboard::isKeyPressed(Keyboard::Key::E)) {
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
        Money = 0;
        HealthPacks.clear();
        AmmoPacks.clear();
        explosions.clear();
        muzzleEffects.clear();
        bloodeffects.clear();
        Enemies.clear();
        bullets.clear();
        Curr_Gun_state = Pistol;
        Current_Wave1 = 0;
        current_level++;
        SwtichCurrentWallBounds();
        PortalOpen = false;
        Player_Health = 100;
        if (current_level ==4)
        {
            Player.setScale(2,2);
            MovementDirection = Vector2f(0, 0);
            Player.setPosition(760, 960);
            Boss.setPosition(960, 900);
            Boss.setScale(-2, 2);
            IsDialogueActive = true;
            smg_buy = true;
            shotgun_buy = true;
            speed_pow = true;
            reload_pow = true;
            sniper_buy = true;
            rocket_buy = true;
            speedmulti = 2;
            reloadmulti = 0.5;
            BossHealth = 6000;
            pistolbulletsloaded = 9;
            riflebulletsloaded = 30;
            shotgunbulletsloaded = 8;
            sniperbulletsloaded = 5;
            rocketbulletsloaded = 1;

            rifleammostock = 150;
            shotgunammostock = 24;
            sniperammostock = 15;
            rocketammostock = 2;

            MusicPlayer.stop();
            music_trigger = false;
        }
        else
        {
            smg_buy = false;
            shotgun_buy = false;
            speed_pow = false;
            reload_pow = false;
            sniper_buy = false;
            rocket_buy = false;
            speedmulti = 1;
            reloadmulti = 1;
            pistolbulletsloaded = 9;
            riflebulletsloaded = 30;
            shotgunbulletsloaded = 8;
            sniperbulletsloaded = 5;
            rocketbulletsloaded = 1;

            rifleammostock = 120;
            shotgunammostock = 24;
            sniperammostock = 5;
            rocketammostock = 2;
        }
    }
}
void Switch_States()
{
    if (MovementDirection.x < -0.1 || MovementDirection.x > 0.1 || MovementDirection.y < -0.1 || MovementDirection.y > 0.1)
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
        if (curr_state!= death)
        {
            EffectsPlayer.setBuffer(GameOver_buffer);
            EffectsPlayer.play();
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
        MovementDirection.x *= 20.0f;
        MovementDirection.y *= 20.0f;
    }
    if (isdashing && !dashready)
    {
        timesincedash += playerdeltatime;

        if (timesincedash > 0.05f)
        {
            isdashing = false;
            MovementDirection.x /= 20.0f;
            MovementDirection.y /= 20.0f;
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
            EffectsPlayer.setBuffer(AbilityReadSound);
            EffectsPlayer.play();
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
            EffectsPlayer.setBuffer(AbilityReadSound);
            EffectsPlayer.play();
            isMinigunReady = true;
            minigun_counter = 0;
        }
    }
}

void TurretLogic()
{
    if (!turretLockedin)
    {
        for (int i = 0; i < Enemies.size(); i++)
        {
            if (!Enemies[i]->isdeath)
            {
                Vector2f currentshootdir = Enemies[i]->shape.getPosition() - Turret.getPosition();
                float currentlength = sqrt(currentshootdir.x * currentshootdir.x + currentshootdir.y * currentshootdir.y);
                if (currentlength < ClosestEnemyDistance)
                {
                    ClosestEnemyDistance = currentlength;
                    currentLockedonEnemey = Enemies[i];
                }
            }
        }
        turretLockedin = true;
    }
    else
    {
        if (currentLockedonEnemey->isdeath)
        {
            turretLockedin = false;
            ClosestEnemyDistance = 10000;
            return;
        }
        TurretShootDir = currentLockedonEnemey->shape.getPosition() - Turret.getPosition();
        float length = sqrt(TurretShootDir.x * TurretShootDir.x + TurretShootDir.y * TurretShootDir.y);
        TurretShootDir_norm = TurretShootDir / length;
        turretAnimationCounter += playerdeltatime;
        turretFireRateCounter += playerdeltatime;
        Turret.setTexture(TurretAnimation[turretimagecounter]);
        if (turretAnimationCounter > turretFireRate / 10)
        {
            turretAnimationCounter = 0;
            turretimagecounter++;
            if (turretimagecounter > 9)
            {
                turretimagecounter = 0;
            }
        }
        float angle = atan2(TurretShootDir_norm.y, TurretShootDir_norm.x) * 180 / pi;
        turretShootPoint = Vector2f(Turret.getPosition().x + cos(angle) * 20, Turret.getPosition().y + sin(angle) * 20);
        Turret.setRotation(angle);
        if (turretFireRateCounter > turretFireRate)
        {
            TurretSoundPlayer.setBuffer(TurretShoot_sound);
            TurretSoundPlayer.play();
            turretFireRateCounter = 0;
            MuzzleFlashEffect newmuzzleeffect;
            newmuzzleeffect.Isturret = true;
            newmuzzleeffect.MuzzleEffect.setPosition(turretShootPoint.x, turretShootPoint.y);
            muzzleEffects.push_back(newmuzzleeffect);
            bullet newbullet;
            newbullet.Isturret = true;
            newbullet.shape.setTexture(bullet_Texture);
            newbullet.shape.setPosition(turretShootPoint.x, turretShootPoint.y);
            newbullet.shape.setScale(0.5f, 0.5f);
            newbullet.id = numberoftotalbulletsshot;
            newbullet.curr_gun_state = Smg;
            newbullet.guntrail.color = Color(239, 177, 7);
            newbullet.guntrail.thickness = 5;
            newbullet.guntrail.maxlength = 7;
            newbullet.currentvelocity = newbullet.maxvelocity * TurretShootDir_norm;
            newbullet.damage = TurretBulletDamage;
            bullets.push_back(newbullet);
            numberoftotalbulletsshot++;
        }
    }
    

}
void TurretAbility()
{

    if (Keyboard::isKeyPressed(Keyboard::F) && isTurretReady)
    {
        isTurretActive = true;
        isTurretReady = false;
        ClosestEnemyDistance = 10000;
        Turret.setPosition(Player.getPosition());
    }
    if (isTurretActive && !isTurretReady)
    {
        turret_counter += playerdeltatime;
        if (Enemies.size() != 0)
        {
            TurretLogic();
        }
        if (turret_counter > 10)
        {
            isTurretActive = false;
            turret_counter = 0;
        }
    }
    if (!isTurretActive && !isTurretReady)
    {
        turret_counter += playerdeltatime;
        if (turret_counter > 0.1)
        {
            EffectsPlayer.setBuffer(AbilityReadSound);
            EffectsPlayer.play();
            isTurretReady = true;
            turret_counter = 0;
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
void select_guns()
{
    if (!isMinigunActive)
    {
        if (Keyboard::isKeyPressed(Keyboard::Num1))
        {
            Curr_Gun_state = Gun_State::Pistol;
            gun_switch_delay_counter = current_fire_rate;
            if (ReloadSound.getStatus() == Sound::Playing)
            {
                ReloadSound.stop();
            }
            trigger = true;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num2) && smg_buy)
        {
            Curr_Gun_state = Gun_State::Smg;
            gun_switch_delay_counter = current_fire_rate;
            if (ReloadSound.getStatus() == Sound::Playing)
            {
                ReloadSound.stop();
            }
            trigger = true;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num3) && shotgun_buy)
        {
            Curr_Gun_state = Gun_State::Shotgun;
            gun_switch_delay_counter = current_fire_rate;
            if (ReloadSound.getStatus() == Sound::Playing)
            {
                ReloadSound.stop();
            }
            trigger = true;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num4) && sniper_buy)
        {
            Curr_Gun_state = Gun_State::Sniper;
            gun_switch_delay_counter = current_fire_rate;
            if (ReloadSound.getStatus() == Sound::Playing)
            {
                ReloadSound.stop();
            }
            trigger = true;
        }  
        if (Keyboard::isKeyPressed(Keyboard::Num5) && rocket_buy)
        {
            Curr_Gun_state = Gun_State::RocketLauncher;
            gun_switch_delay_counter = current_fire_rate;
            if (ReloadSound.getStatus() == Sound::Playing)
            {
                ReloadSound.stop();
            }
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
        break;
    case RocketLauncher:
        current_fire_rate = rocketfirerate;
        current_damage = rocketdamage;
        current_bullets_per_shot = rocketbulletpershot;
        current_spread = rocketspread;
        current_ammo = &rocketbulletsloaded;
        current_ammo_stock = &rocketammostock;
        current_clip_size = rocketclipsize;
        current_reload_time = rocketreloadtime * reloadmulti;
        current_bullets_loaded_per_reload = rocket_bullets_loaded_per_reload;
        camera_shake_magnitude = 20;
        Current_shoot_Buffer = &Rocket_Shoot_Sound;
        Current_reload_Buffer = &Rocket_Reload_Sound;
        if (rocketbulletsloaded > 30)
        {
            rocketbulletsloaded = 30;
        }
    }
    if (explosions.size() != 0)
    {
        camera_shake_magnitude = 20;
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
        switch (Curr_Gun_state)
        {
        case Pistol:
            MovementDirection = Vector2f(MovementDirection.x += -Norm_dir_vector.x * 100, MovementDirection.y += -Norm_dir_vector.y * 100);
            break;
        case Smg:
            MovementDirection = Vector2f(MovementDirection.x += -Norm_dir_vector.x * 25, MovementDirection.y += -Norm_dir_vector.y * 25);
            break;
        case Shotgun:
            MovementDirection = Vector2f(MovementDirection.x += -Norm_dir_vector.x * 200, MovementDirection.y += -Norm_dir_vector.y * 200);
            break;
        case Sniper:
            MovementDirection = Vector2f(MovementDirection.x += -Norm_dir_vector.x * 200, MovementDirection.y += -Norm_dir_vector.y * 200);
            break;
        case MiniGun:
            MovementDirection = Vector2f(MovementDirection.x += -Norm_dir_vector.x * 25, MovementDirection.y += -Norm_dir_vector.y * 25);
            break;
        case RocketLauncher:
            MovementDirection = Vector2f(MovementDirection.x += -Norm_dir_vector.x * 300, MovementDirection.y += -Norm_dir_vector.y * 300);
            break;
        }
        MuzzleFlashEffect newmuzzleeffect;
        newmuzzleeffect.MuzzleEffect.setPosition(test.getPosition());
        muzzleEffects.push_back(newmuzzleeffect);
        ShootSound.setBuffer(*Current_shoot_Buffer);
        ShootSound.play();
        for (int i = 0; i < current_bullets_per_shot; i++)
        {
            bullet newbullet;
            newbullet.shape.setTexture(bullet_Texture);
            newbullet.shape.setPosition(test.getPosition());
            newbullet.shape.setScale(0.5f, 0.5f);
            newbullet.id = numberoftotalbulletsshot;
            newbullet.curr_gun_state = Curr_Gun_state;
            switch (Curr_Gun_state)
            {
            case Pistol:
                newbullet.guntrail.color = Color(239, 177, 7);
                newbullet.guntrail.thickness = 4;
                newbullet.guntrail.maxlength = 7;
                break;
            case Smg:
                newbullet.guntrail.color = Color(239, 177, 7);
                newbullet.guntrail.thickness = 5;
                newbullet.guntrail.maxlength = 7;
                break;
            case Shotgun:
                newbullet.guntrail.color = Color(239, 177, 7);
                newbullet.guntrail.thickness = 5;
                newbullet.guntrail.maxlength = 3;
                break;
            case Sniper:
                newbullet.guntrail.color = Color(239, 177, 7);
                newbullet.guntrail.thickness = 5;
                newbullet.guntrail.maxlength = 10;
                break;
            case MiniGun:
                newbullet.guntrail.color = Color(239, 177, 7);
                newbullet.guntrail.thickness = 4;
                newbullet.guntrail.maxlength = 3;
                break;
            case RocketLauncher:
                newbullet.guntrail.color = Color::White;
                newbullet.guntrail.thickness = 4;
                newbullet.guntrail.maxlength = 3;
                newbullet.isRocket = true;
                newbullet.maxvelocity = 1000;
                newbullet.DrawEffect = true;
                break;
            }
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
        bullets[i].animation(playerdeltatime, RocketSpawnAnimation,Player.getPosition(),isSlowing,FireBallAnimation);
        for (int k = 0; k < Wall_Bounds.size(); k++)
        {
            if (bullets[i].shape.getGlobalBounds().intersects(Wall_Bounds[k]))
            {
                if (bullets[i].isRocket)
                {
                    Explosion newexplosion;
                    newexplosion.explosionlogic(bullets[i].shape.getPosition(), Player.getPosition(),Boss.getPosition(), rocketdamage, rocket_radius, isdashing);
                    explosions.push_back(newexplosion);
                }
                bullets[i].deleteme = true;
                break;
            }
        }
    }
    for (int i = 0; i < bullets.size(); i++)
    {
        for (int j = 0; j < Enemies.size(); j++)
        {
            FloatRect current_zombie_Bound = Enemies[j]->shape.getGlobalBounds();
            if (Enemies[j]->type == 2)
            {
                current_zombie_Bound = FloatRect(current_zombie_Bound.left + 100, current_zombie_Bound.top, current_zombie_Bound.width - 200, current_zombie_Bound.height);
            }
            if (!bullets[i].IsFireBall && bullets[i].shape.getGlobalBounds().intersects(current_zombie_Bound) && Enemies[j]->last_hit_bullet_id != bullets[i].id && !Enemies[j]->isdeath)
            {
                Enemies[j]->last_hit_bullet_id = bullets[i].id;
                if (bullets[i].isRocket)
                {    
                    Explosion newexplosion;
                    newexplosion.explosionlogic(bullets[i].shape.getPosition(), Player.getPosition(),Boss.getPosition(), rocketdamage, rocket_radius, isdashing);
                    explosions.push_back(newexplosion);
                }
                else
                {
                    Enemies[j]->health -= bullets[i].damage;
                    if (Curr_Gun_state == Shotgun)
                    {
                        Enemies[j]->currentvelocity = -Vector2f(Enemies[j]->currentvelocity.x + 35, Enemies[j]->currentvelocity.y + 35);
                    }
                    else if (Curr_Gun_state == Sniper)
                    {
                        Enemies[j]->currentvelocity = -Vector2f(Enemies[j]->currentvelocity.x + 500, Enemies[j]->currentvelocity.y + 500);
                    }
                    else if (Curr_Gun_state == MiniGun)
                    {
                        Enemies[j]->currentvelocity = -Vector2f(Enemies[j]->currentvelocity.x + 20, Enemies[j]->currentvelocity.y + 20);
                    }
                    else
                    {
                        Enemies[j]->currentvelocity = -Vector2f(Enemies[j]->currentvelocity.x + 100, Enemies[j]->currentvelocity.y + 100);
                    }
                    Enemies[j]->isEnemyhit = true;
                    SpawnBlood(*Enemies[j]);
                }
                if (Curr_Gun_state != Sniper)
                {
                    bullets[i].deleteme = true;
                    break;
                }
            }
        }
        if (!bullets[i].IsFireBall && !bullets[i].Isturret && bullets[i].shape.getGlobalBounds().intersects(Boss.getGlobalBounds()) && BossHealth > 0)
        {
            BossHealth -= bullets[i].damage;
            bullets[i].deleteme = true;
            break;
        }
    }
    for (int i = 0; i < muzzleEffects.size(); i++)
    {
        if (muzzleEffects[i].deleteme)
        {
            muzzleEffects.erase(muzzleEffects.begin() + i);
            break;
        }
        if (muzzleEffects[i].Isturret)
        {
            muzzleEffects[i].handlemuzzleeffect(playerdeltatime, Turret.getPosition());
        }
        else
        {
            muzzleEffects[i].handlemuzzleeffect(playerdeltatime, test.getPosition());
        }
    }
    for (int i = 0; i < bullets.size(); i++)
    {
        if (bullets[i].deleteme)
        {
            bullets.erase(bullets.begin() + i);
            break;
        }
    }
    for (int i = 0; i < explosions.size(); i++)
    {
        if (explosions[i].deleteme)
        {
            explosions.erase(explosions.begin() + i);
            break;
        }
        explosions[i].ExpoAnimation(RocketExplosionAnimation, dt);
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
            break;
        case RocketLauncher:
            RocketL_S.setTexture(RocketLauncher_Image);
            break;
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
            break;
        case RocketLauncher:
            RocketL_S.setTexture(RocketLauncher_Image);
            break;
        }
    }
    else
    {
        SMG_S.setTexture(SMG_Shoot_Animations[0]);
        Pistol_S.setTexture(pistol_shoot_animations[0]);
        ShotGun_S.setTexture(shotgun_shoot_animation[0]);
        Sniper_S.setTexture(sniper_shoot_animations[0]);
        RocketL_S.setTexture(RocketLauncher_Image);
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
    if (!IsBossDead)
    {
        if (camera_shake_counter > 0)
        {
            float magnitudereduction = camera_shake_magnitude * (camera_shake_counter / camera_shake_duration);
            cameraoffset_shake = Vector2f((rand() / static_cast<float>(RAND_MAX)) * magnitudereduction - magnitudereduction / 2, (rand() / static_cast<float>(RAND_MAX)) * magnitudereduction - magnitudereduction / 2);
            view.setCenter(Player.getPosition() + cameraoffset_shake);
            camera_shake_counter -= playerdeltatime;
        }
        else
        {
            view.setCenter(Player.getPosition());
        }
    }
    else
    {
        view.setCenter(Boss.getPosition());
    }
}

//zombie functions

void SpawnZombiesWaves(float dt)
{
    float multiplier = (Current_Wave1 / (float)Level1NumWaves) * 1.5f;
    SpawningZombieCounter += dt;
    if (canspawn && SpawningZombieCounter >= 1)
    {
        int randomthing = 1 + rand() % 10;
        if (randomthing <= 6)
        {
            auto newzombie = make_unique<Zombie>();
            newzombie->type = 0;
            newzombie->shape.setPosition(50 + rand() % 1770, 50 + rand() % 930);
            newzombie->damage *= multiplier;
            newzombie->health *= multiplier;
            newzombie->maxvelocity *= multiplier;
            newzombie->SetSpawnLocation();
            Enemies.push_back(move(newzombie));
        }
        else if (randomthing > 6 &&randomthing < 8)
        {
            auto newzombie = make_unique<Zombie>();
            newzombie->type = 2;
            newzombie->shape.setPosition(150 + rand() % 1770, 150 + rand() % 930);
            newzombie->damage *= multiplier * 4;
            newzombie->animation_duration *= 3;
            newzombie->Death_animation_duration /= 2;
            newzombie->maxwalkframes = 8;
            newzombie->maxhitframes = 4;
            newzombie->maxdeathframes = 8;
            newzombie->health *= 10;
            newzombie->attackdistance *= 2;
            newzombie->shape.setTexture(Golem_atk_animation[2]);
            newzombie->maxvelocity *= multiplier / 2;
            newzombie->SetSpawnLocation();
            Enemies.push_back(move(newzombie));
        }
        else
        {
            auto newskull = make_unique<SkullFire>();
            newskull->type = 1;
            newskull->health *= multiplier;
            newskull->shape.setPosition(50 + rand() % 1770, 50 + rand() % 930);
            newskull->maxvelocity *= multiplier;
            newskull->SetSpawnLocation();
            Enemies.push_back(move(newskull));
        }       
        TotalSpawnedZombies++;
        SpawningZombieCounter = 0;
    }
    if (TotalSpawnedZombies >= 25 * Current_Wave1)
    {
        canspawn = false;
    }
    if (Enemies.size()== 0 && !canspawn && !PortalOpen )
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
    if (Current_Wave1 > 3 && current_level <= 3)
    {
        canspawn = false;
        PortalOpen = true;
        UpdatePortalAnimation();
    }
}
void HandleZombieBehaviour(float dt)
{
    for (int i = 0; i < Enemies.size(); i++)
    {
        if (Enemies[i]->remove_Enemy)
        {
            Enemies.erase(Enemies.begin() + i);
            Enemies.shrink_to_fit();
            break;
        }
        Enemies[i]->SpawnAnimation(Zombie_spawn_animation, dt);
        switch (Enemies[i]->type)
        {
        case 0:
            Enemies[i]->EnemyBehaviour(zombie_hit_animation, zombie_atk_animation, zombie_walk_animation, zombie_death_animation, isdashing,dt, Player.getPosition(),Boss.getPosition());
            break;
        case 1:
            Enemies[i]->EnemyBehaviour(zombie_hit_animation, zombie_atk_animation, SkullFire_animations, zombie_death_animation, isdashing,dt, Player.getPosition(), Boss.getPosition());
            break;
        case 2:
            Enemies[i]->EnemyBehaviour(Golem_hit_animation, Golem_atk_animation, Golem_walk_animation, Golem_death_animation, isdashing, dt, Player.getPosition(), Boss.getPosition());
            break;
        }
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
    for (int i = 0; i < Enemies.size(); i++)
    {
        if (Enemies[i]->isdeath)
        {
            continue;
        }
        for (int j = 0; j < Wall_Bounds.size(); j++)
        {
            FloatRect current_zombie_Bound =Enemies[i]->shape.getGlobalBounds();
            if (Enemies[i]->type == 2)
            {
                current_zombie_Bound = FloatRect(current_zombie_Bound.left, current_zombie_Bound.top, current_zombie_Bound.width - 50, current_zombie_Bound.height);
            }
            FloatRect intersection;
            FloatRect Wall_bound = Wall_Bounds[j];
            if (current_zombie_Bound.intersects(Wall_bound))
            {
                current_zombie_Bound.intersects(Wall_bound, intersection);
                if (intersection.width < intersection.height)
                {
                    if (current_zombie_Bound.left < Wall_bound.left)
                    {
                        Enemies[i]->shape.move((- (Enemies[i]->currentvelocity.x / abs(Enemies[i]->currentvelocity.x)) * Enemies[i]->currentvelocity.x - 150)* dt, 0);
                    }
                    else
                    {
                        Enemies[i]->shape.move( (-(Enemies[i]->currentvelocity.x / abs(Enemies[i]->currentvelocity.x)) * -Enemies[i]->currentvelocity.x + 150)* dt, 0);
                    }
                }
                else
                {
                    if (current_zombie_Bound.top < Wall_bound.top)
                    {
                        Enemies[i]->shape.move(0, ( - (Enemies[i]->currentvelocity.y / abs(Enemies[i]->currentvelocity.y)) * Enemies[i]->currentvelocity.y-150) * dt);
                    }
                    else
                    {
                        Enemies[i]->shape.move(0, ( - (Enemies[i]->currentvelocity.y / abs(Enemies[i]->currentvelocity.y)) * -Enemies[i]->currentvelocity.y + 150)* dt);
                    }
                }
            }
        }
        for (int j = 0; j < Enemies.size(); j++)
        {
                if (i == j || Enemies[j]->type == 2)
                {
                    continue;
                }
                FloatRect Current_Zombie_Bound = Enemies[i]->shape.getGlobalBounds();
                FloatRect intersection;
                FloatRect Current_other_zombie_bound = Enemies[j]->shape.getGlobalBounds();
                if (Current_Zombie_Bound.intersects(Current_other_zombie_bound))
                {

                    Current_Zombie_Bound.intersects(Current_other_zombie_bound, intersection);
                    if (intersection.width < intersection.height)
                    {
                        if (Current_Zombie_Bound.left < Current_other_zombie_bound.left)
                        {
                            Enemies[i]->shape.move(-50 * dt, 0);
                        }
                        else
                        {
                            Enemies[i]->shape.move(50 * dt, 0);
                        }
                    }
                    else
                    {
                        if (Current_Zombie_Bound.top < Current_other_zombie_bound.top)
                        {
                            Enemies[i]->shape.move(0, -50 * dt);
                        }
                        else
                        {
                            Enemies[i]->shape.move(0, 50 * dt);
                        }
                    }
                }
                    
        }
    }
  
}

//Boss-Fight Functions

void BossUpdateStuff(float dt)
{
    if (!IsDialogueActive)
    {
        for (int i = 0; i < LaserBeams.size(); i++)
        {
            if (LaserBeams[i].deleteme)
            {
                LaserBeams.erase(LaserBeams.begin() + i);
                break;
            }
            LaserBeams[i].LaserBeamAnimation(dt);
        }
        if (BossHealth <= 0 && !IsBossDead)
        {
            view.zoom(1);
            Enemies.clear();
            bullets.clear();
            Boss_image_counter = 0;
            MusicPlayer.stop();
            MusicPlayer.setBuffer(VictoryTheme);
            MusicPlayer.play();
            Player.setPosition(Boss.getPosition().x, Boss.getPosition().y + 200);
            IsBossDead = true;
        }
        if (!IsBossDead)
        {
            BossMovement(dt);
            BossAbilitySelector(dt);
        }
        else
        {
            Curr_Boss_State = Death;
        }
    }
    if (!BossDone)
    {
        BossAnimationHandler();
    }
}
void BossMovement(float dt)
{
    if (!Ability_Ready && !isCasting)
    {
        ability_counter += dt;
        if (ability_counter > Abilities_freq)
        {
            ability_counter = 0;
            Ability_Ready = true;
        }
    }
    Boss_Player_dir = Vector2f(Player.getPosition() - Boss.getPosition());
    distancefromplayer = sqrt(Boss_Player_dir.x * Boss_Player_dir.x + Boss_Player_dir.y * Boss_Player_dir.y);
    Boss_Player_Norm_dir = Boss_Player_dir / distancefromplayer;
    if (sqrt(Boss_move_dir.x * Boss_move_dir.x + Boss_move_dir.y * Boss_move_dir.y) < BossMaxSpeed)
    {
        Boss_move_dir = Vector2f(Boss_move_dir.x += BossMaxSpeed * dt, Boss_move_dir.y += BossMaxSpeed * dt);
    }
    else
    {
        Boss_move_dir = Vector2f(Boss_move_dir.x += -(Boss_move_dir.x / abs(Boss_move_dir.x)) * dt * 500, Boss_move_dir.y += -(Boss_move_dir.y / abs(Boss_move_dir.y)) * dt * 500);
    }
    if (distancefromplayer > 250 && !isCasting)
    {
        Curr_Boss_State = Walk;
        Boss.move(Vector2f(Boss_move_dir.x * Boss_Player_Norm_dir.x * dt, Boss_move_dir.y * Boss_Player_Norm_dir.y * dt));
    }
    if (Ability_Ready)
    {
        isCasting = true;
        Ability_Ready = false;
        BossAbilityRoller();
    }
}
void BossAnimationCounter(int maximagecounter, bool isdeath)
{
    Boss_animation_counter += playerdeltatime;
    if (isdeath)
    {
        Boss_animation_switchtime = 0.7;
    }
    if (Boss_animation_counter >= Boss_animation_switchtime)
    {
        Boss_animation_counter = 0;
        Boss_image_counter++;
        if (Boss_image_counter >= maximagecounter)
        {
            if (!isdeath)
            {
                Boss_image_counter = 0;
            }
            else
            {
                BossDone = true;
            }
        }
    }
}
void BossAnimationHandler()
{
    switch (Curr_Boss_State)
    {
    case Walk:
        BossAnimationCounter(8, false);
        Boss.setTexture(Boss_walk_animation[Boss_image_counter]);
        break;
    case Death:
        BossAnimationCounter(9, true);
        Boss.setTexture(Boss_Death_animation[Boss_image_counter]);
        break;
    case LaserAtk:
        BossAnimationCounter(13, false);
        Boss.setTexture(Boss_LaserAtk_animation[Boss_image_counter]);
        break;
    case BlindAtk:
        BossAnimationCounter(17, false);
        Boss.setTexture(Boss_BlindAtk_animation[Boss_image_counter]);
        break;
    case BulletAtk:
        BossAnimationCounter(13, false);
        Boss.setTexture(Boss_BulletsAtk_animation[Boss_image_counter]);
        break;
    }
}
void BossAbilityRoller()
{
    RandomAbility = rand() % 7;
    if (RandomAbility <= 1)
    {
        SelectedAbility = 1;
    }
    if (RandomAbility > 1 &&RandomAbility <4 )
    {
        SelectedAbility = 2;
    }
    if(RandomAbility == 4)
    {
        SelectedAbility = 3;
    }
    if (RandomAbility == 5 || RandomAbility == 6)
    {
        SelectedAbility = 4;
    }
    if (SelectedAbility == previousSelected)
    {
        BossAbilityRoller();
        return;
    }
    switch (SelectedAbility)
    {
    case 1:
        LockingIn = false;
        Curr_Boss_State = LaserAtk;
        BossSoundPlayer.setBuffer(BossLaserSound);
        previousSelected = 1;
        break;
    case 2:
        Curr_Boss_State = BulletAtk;
        BossSoundPlayer.setBuffer(BossFireBallSound);
        previousSelected = 2;
        break;
    case 3:
        IsBlind = true;
        blindcounter = 0;
        Curr_Boss_State = BlindAtk;
        BossSoundPlayer.setBuffer(BossBlindSound);
        BossSoundPlayer.play();
        previousSelected = 3;
        break;
    case 4:
        Curr_Boss_State == LaserAtk;
        BossSoundPlayer.setBuffer(BossSpawnEnemiesSound);
        BossSoundPlayer.play();
        previousSelected = 4;
        break;
    }
}
void BossAbilitySelector(float dt)
{
    if (isCasting)
    {
        if (RandomAbility <= 1)
        {
            BossLaserBeam(dt);
        }
        if (RandomAbility == 2 || RandomAbility == 3)
        {
            BossBulletHell(dt);
        }
        if(RandomAbility == 4)
        {
            BossBlind(dt);
        }     
        if (RandomAbility == 5 || RandomAbility == 6)
        {
            BossSpawnEnemies();
        }
    }
}
void BossLaserBeam(float dt)
{
    if (LaserShotsFired < NumberOfLaserShots)
    {
        lasercounter += dt;
        if (!LockingIn)
        {
            endpoint = Player.getPosition();
            LockingIn = true;
        }
        if (lasercounter > 0.4)
        {
            BossSoundPlayer.play();
            lasercounter = 0;
            Explosion newexplosion;
            newexplosion.explosionlogic(endpoint, Player.getPosition(),Boss.getPosition(), 100, 100, isdashing);
            explosions.push_back(newexplosion);
            LaserBeam newlaser;
            newlaser.start = Boss.getPosition();
            newlaser.end = endpoint;
            newlaser.laserSprite.setTexture(laserTex);
            newlaser.LaserBeamLogic(laserTex);
            LaserBeams.push_back(newlaser);
            LaserShotsFired++;
            LockingIn = false;
        }
    }
    else
    {
        isCasting = false;
        LaserShotsFired = 0;
    }
}
void BossBlind(float dt)
{
    if (blindcounter > blindduration / 4)
    {
        isCasting = false;
    }
}
void BossBulletHell(float dt)
{
    Boss_FireRate_counter += dt;
    countertest += dt;
    if (Boss_FireRate_counter > Boss_Fire_Rate && currentBulletwaves < numberofbulletwaves)
    {
        currentBulletwaves++;
        Boss_FireRate_counter = 0;
        BossSoundPlayer.play();
        for (int i = 0; i <Boss_NumberofBulletsToShoot; i++)
        {     
            bullet newFireball;
            newFireball.IsFireBall = true;
            float angle = (i / static_cast<float>(numberofbulletsegments)) * 2 * pi;
            angle += (currentBulletwaves) * 45;
            float x = Boss.getPosition().x + RadiusOfBulletRing * cos(angle);
            float y = Boss.getPosition().y + RadiusOfBulletRing * sin(angle);
            newFireball.shape.setPosition(x, y);
            newFireball.maxvelocity = fireballspeed / 10;
            Vector2f dir = Vector2f(x, y) - Boss.getPosition();
            Vector2f norm = dir / sqrt(dir.x * dir.x + dir.y * dir.y);
            newFireball.shape.setRotation(atan2(norm.y , norm.x) * 180 / pi);
            newFireball.currentvelocity = newFireball.maxvelocity * norm;
            newFireball.damage = Fireball_Damage;
            bullets.push_back(newFireball);
        }
    }
    if (currentBulletwaves >= numberofbulletwaves)
    {
        isCasting = false;
        currentBulletwaves = 0;
    }
}
void BossSpawnEnemies()
{
    for (int i = 0; i < 1; i++)
    {
        auto newzombie = make_unique<Zombie>();
        newzombie->type = 2;
        newzombie->shape.setPosition((Boss.getPosition().x - 300) + rand() % 301, (Boss.getPosition().y + 300) - rand() % 301);
        newzombie->damage *= 8;
        newzombie->animation_duration *= 3;
        newzombie->Death_animation_duration /= 2;
        newzombie->maxwalkframes = 8;
        newzombie->maxhitframes = 4;
        newzombie->maxdeathframes = 8;
        newzombie->health *= 10;
        newzombie->attackdistance *= 2;
        newzombie->shape.setTexture(Golem_atk_animation[2]);
        newzombie->maxvelocity /= 2;
        newzombie->SetSpawnLocation();
        Enemies.push_back(move(newzombie));
    }
    for (int i = 0; i <4 ; i++)
    {
        auto newzombie = make_unique<Zombie>();
        newzombie->type = 0;
        newzombie->shape.setPosition((Boss.getPosition().x - 300) + rand() % 301, (Boss.getPosition().y + 300) - rand() % 301);
        newzombie->SetSpawnLocation();
        Enemies.push_back(move(newzombie));
    }
    for (int i = 0; i < 2; i++)
    {
        auto newskull = make_unique<SkullFire>();
        newskull->type = 1;
        newskull->shape.setPosition((Boss.getPosition().x - 300) + rand() % 301, (Boss.getPosition().y + 300) - rand() % 301);
        newskull->SetSpawnLocation();
        Enemies.push_back(move(newskull));
    }
    isCasting = false;
}

void SwtichCurrentWallBounds()
{
    Wall_Bounds.clear();
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
    case 4:
        for (int i = 0; i < Wall_Bounds4.size(); i++)
        {
            Wall_Bounds.push_back(Wall_Bounds4[i]);
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

//level 4 functions
void Wall4()
{

    RectangleShape wall11;
    RectangleShape wall22;
    RectangleShape wall33;
    RectangleShape wall44;
    wall11.setSize(Vector2f(64,1920));
    wall11.setPosition(0, 0);
    wall22.setSize(Vector2f(64, 1920));
    wall22.setPosition(1856,0 );
    wall33.setSize(Vector2f(1920, 64));
    wall33.setPosition(0, 0);
    wall44.setSize(Vector2f(1920, 64));
    wall44.setPosition(0, 1856);

    Wall_Bounds4.push_back(wall11.getGlobalBounds());
    Wall_Bounds4.push_back(wall22.getGlobalBounds());
    Wall_Bounds4.push_back(wall33.getGlobalBounds());
    Wall_Bounds4.push_back(wall44.getGlobalBounds());
}


//drawing function
void Draw()
{
    Player.setOrigin(Vector2f(Player.getLocalBounds().width / 2, Player.getLocalBounds().height / 2));
    Boss.setOrigin(Boss.getLocalBounds().width / 2, Boss.getLocalBounds().height / 2);
    RadialLight light;
    light.setRange(400);
    light.setIntensity(150);
    light.setColor(Color(229, 238, 141));
    CircleShape testpoint(5);
    testpoint.setOrigin(testpoint.getLocalBounds().width / 2, testpoint.getLocalBounds().height / 2);
    testpoint.setPosition(Player.getPosition());
    Crosshair.setOrigin(Crosshair.getLocalBounds().width / 2, Crosshair.getLocalBounds().height / 2);
    LockinCrosshair.setOrigin(LockinCrosshair.getLocalBounds().width / 2, LockinCrosshair.getLocalBounds().height / 2);
    ambientlight.clear();
    if (menu_num != 5 && menu_num != 6) {
        window.clear();
    }
    if (!IsDialogueActive)
    {
        if (floor(dir_vector.x) > 0)
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
    }
    else
    {
        Player.setScale(2, 2);
    }
    test.setOrigin(test.getLocalBounds().width / 2, test.getLocalBounds().height / 2);
    switch (current_level)
    {
    case 1:
        Portal_S.setPosition(580, 790);
        speedmachine.setPosition(Vector2f(200, 25));
        reloadmachine.setPosition(Vector2f(1080, 580)); 
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 7; j++)
            {
                window.draw(background1[j][i]);
            }
        }
        smg_buying.setPosition(Vector2f(480, 270));
        shotgun_buying.setPosition(Vector2f(1440, 270));
        sniper_buying.setPosition(Vector2f(1440, 810));
        Rocket_buying.setPosition(Vector2f(480, 810));
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
        Rocket_buying.setPosition(Vector2f(1700, 200));
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
        smg_buying.setPosition(Vector2f(480, 270));
        shotgun_buying.setPosition(Vector2f(1440, 270));
        sniper_buying.setPosition(Vector2f(1440, 810));
        Rocket_buying.setPosition(Vector2f(480, 810));
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
        case 4:
            window.draw(BossLevel);
            break;
    }
    if (current_level != 4)
    {
        for (int i = 0; i < 4; i++)
        {
            window.draw(void1[i]);
        }

    }
    if (current_level != 4)
    {
        window.draw(speedmachine);
        window.draw(reloadmachine);
    }
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
    if (current_level != 4)
    {
        WeaponsBuyDrawing();
    }
    window.draw(Player);
    if (current_level ==4)
    {
        light.setRange(200);
        light.setPosition(Player.getPosition());
        window.draw(light);
        ambientlight.draw(light);
    }

    if (!BossDone && current_level == 4)
    {
        if (Boss_Player_Norm_dir.x > 0)
        {
            Boss.setScale(2, 2);
        }
        else
        {
            Boss.setScale(-2, 2);
        }
        window.draw(Boss);
        light.setIntensity(100);
        light.setColor(Color(155, 25, 25));
        light.setPosition(Boss.getPosition());
        window.draw(light);
        ambientlight.draw(light);
    }
    for (int i = 0; i < Enemies.size(); i++)
    {
        Enemies[i]->SpawnEffect.setOrigin(Enemies[i]->SpawnEffect.getLocalBounds().width / 2, Enemies[i]->SpawnEffect.getLocalBounds().height / 2);
        window.draw(Enemies[i]->SpawnEffect);
        if (Enemies[i]->Spawn_imagecounter <= 6)
        {
            window.draw(Enemies[i]->SpawnLight);
            ambientlight.draw(Enemies[i]->SpawnLight);
        }
        Enemies[i]->shape.setOrigin(Enemies[i]->shape.getLocalBounds().width / 2, Enemies[i]->shape.getLocalBounds().height / 2);
        if (Enemies[i] -> type == 1)
        {
            Enemies[i]->explosionArea.setRadius(75);
            Enemies[i]->explosionArea.setFillColor(Color(180, 0, 0, 16));
            Enemies[i]->explosionArea.setOrigin(Enemies[i]->explosionArea.getLocalBounds().width / 2, Enemies[i]->explosionArea.getLocalBounds().height / 2);
            Enemies[i]->explosionArea.setPosition(Enemies[i]->shape.getPosition());

            window.draw(Enemies[i]->explosionArea);

            Enemies[i]->explosionArea.setFillColor(Color(180, 0, 0, 8));
            Enemies[i]->explosionArea.setRadius(rocket_radius);
            Enemies[i]->explosionArea.setOrigin(Enemies[i]->explosionArea.getLocalBounds().width / 2, Enemies[i]->explosionArea.getLocalBounds().height / 2);
            Enemies[i]->explosionArea.setPosition(Enemies[i]->shape.getPosition());
            window.draw(Enemies[i]->explosionArea);
        }
        if(Enemies[i]->norm_Direction.x < 0)
        {
            switch (Enemies[i]->type)
            {
            case 0:
                Enemies[i]->shape.setScale(-2, 2);
                break;
            case 1:
                Enemies[i]->shape.setScale(-1, 1);
                break;
            case 2:
                Enemies[i]->shape.setScale(2, 2);
            }
        }
        else
        {
            switch (Enemies[i]->type)
            {
            case 0:
                Enemies[i]->shape.setScale(2, 2);
                break;
            case 1:
                Enemies[i]->shape.setScale(1, 1);
                break;
            case 2:
                Enemies[i]->shape.setScale(-2, 2);
            }
        }
        window.draw(Enemies[i]->shape);
    }
    for (int i = 0; i < bullets.size(); i++)
    {
        bullets[i].effect.setOrigin(bullets[i].effect.getLocalBounds().width / 2, bullets[i].effect.getLocalBounds().height / 2);
        bullets[i].effect.setOrigin(bullets[i].effect.getLocalBounds().width / 2, bullets[i].effect.getLocalBounds().height / 2);
        window.draw(bullets[i].lighteffect);
        if (!bullets[i].IsFireBall) { bullets[i].guntrail.drawTrail(window); }
        if (bullets[i].isRocket)
        {
            if (isSlowing)
            {
                window.draw(bullets[i].shape);
            }
            window.draw(bullets[i].explosionArea);
        }      
        if (bullets[i].DrawEffect)
        {
            window.draw(bullets[i].effect);
        }
        if (isSlowing || bullets[i].IsFireBall)
        {
            window.draw(bullets[i].shape);
        }
        ambientlight.draw(bullets[i].lighteffect);
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
        test.setPosition(MiniGun_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 70, MiniGun_S.getPosition().y - 5 + sin(Gun.getRotation() / 180 * pi) * 70);
        MiniGun_S.setScale(Gun.getScale().x * 4, Gun.getScale().y * 4);
        MiniGun_S.setPosition(Gun.getPosition());
        MiniGun_S.setRotation(Gun.getRotation());
        window.draw(MiniGun_S);
        break;
    case RocketLauncher:
        RocketL_S.setOrigin(RocketL_S.getLocalBounds().width / 4, RocketL_S.getLocalBounds().height / 2);
        test.setPosition(RocketL_S.getPosition().x + cos(Gun.getRotation() / 180 * pi) * 10, RocketL_S.getPosition().y + sin(Gun.getRotation() / 180 * pi) * 50);
        RocketL_S.setScale(Gun.getScale().x * 4, Gun.getScale().y * 4);
        RocketL_S.setPosition(Gun.getPosition());
        RocketL_S.setRotation(Gun.getRotation());
        window.draw(RocketL_S);
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
    if (!IsDialogueActive)
    {
        if (!IsBossDead)
        {
            if (isTurretActive)
            {
                Turret.setOrigin(Turret.getLocalBounds().width / 2, Turret.getLocalBounds().height / 2);
                window.draw(Turret);
            }
            if (LockingIn)
            {
                explosionArea.setRadius(100);
                explosionArea.setFillColor(Color(180, 0, 0, 64));
                explosionArea.setOrigin(explosionArea.getLocalBounds().width / 2, explosionArea.getLocalBounds().height / 2);
                explosionArea.setPosition(endpoint);
                window.draw(explosionArea);
            }
            for (int i = 0; i < LaserBeams.size(); i++)
            {
                window.draw(LaserBeams[i].laserSprite);
                window.draw(LaserBeams[i].beamLight);
                ambientlight.draw(LaserBeams[i].beamLight);
            }
            for (int i = 0; i < explosions.size(); i++)
            {
                window.draw(explosions[i].shape);
            }
            UI();
            Combo();
            if (LockingIn)
            {
                LockinCrosshair.setPosition(Player.getPosition());
                window.draw(LockinCrosshair);
            }
        }
        if (BossDone)
        {
            light.setIntensity(200);
            light.setColor(Color(255, 176, 176));
            light.setRange(WinningColor.a * 2);
            light.setOrigin(light.getLocalBounds().width / 2, light.getLocalBounds().height / 2);
            if (winnerAlpha < 255)
            {
                winnerAlpha += playerdeltatime * 40;
            }
            winningText.setFont(normal_font); // select the font 
            winningText.setString("Thank you for Playing <3 \n\t\t    Score: " + to_string(Score));
            winningText.setCharacterSize(36);
            if (WinningColor.a < 250)
            {
                WinningColor.a = static_cast<sf::Uint8>(winnerAlpha);
            }
            winningText.setFillColor(WinningColor);
            winningText.setOrigin(winningText.getLocalBounds().width / 2, winningText.getLocalBounds().width / 2);
            winningText.setPosition(window.mapPixelToCoords(Vector2i(960, 960)));
            light.setPosition(window.mapPixelToCoords(Vector2i(960, 800)));
            window.draw(light);
            ambientlight.draw(light);
            exitButton.setPosition(window.mapPixelToCoords(Vector2i(960, 400))); FloatRect collesion2 = exitButton.getGlobalBounds();
            exitButton.setOrigin(exitButton.getLocalBounds().width / 2, exitButton.getLocalBounds().height / 2);
            exitButton.setColor(Color(exitButton.getColor().r, exitButton.getColor().g, exitButton.getColor().b, winnerAlpha));
            window.draw(exitButton);
            if (collesion2.contains(MousePos))
            {
                if (Mouse::isButtonPressed(Mouse::Left))
                {
                    if (Score > highest_score)
                    {
                        highest_score = Score;
                    }
                    EffectsPlayer.stop();
                    menu_num = 1;
                    MusicPlayer.setBuffer(music[3]);
                    MusicPlayer.play();
                    music_trigger = false;
                }
                if (exitButton.getScale().x < 0.3 && exitButton.getScale().y < 0.3)
                {
                    exitButton.setScale(exitButton.getScale().x + playerdeltatime * 2, exitButton.getScale().x + playerdeltatime * 2);
                }
            }
            else
            {
                if (exitButton.getScale().x > 0.2 && exitButton.getScale().y > 0.2)
                {
                    exitButton.setScale(exitButton.getScale().x - playerdeltatime * 2, exitButton.getScale().x - playerdeltatime * 2);
                }
            }
            window.draw(winningText);
        }
    }
    else
    {
        Dialogue();
    }
    if (menu_num == 0)
    {
        window.draw(Crosshair);
    }
    if (menu_num != 5 && menu_num != 6)
    {
        window.display();
    }
    ambientlight.display();
}
void WeaponsBuyDrawing()
{
    RadialLight light;
    light.setRange(100);
    light.setIntensity(200);
    light.setColor(Color::Yellow);

    //smg price text 
    money_smg_text.setFont(normal_font);
    money_smg_text.setString("$" + to_string(money_smg));
    money_smg_text.setCharacterSize(16);
    money_smg_text.setFillColor(Color::White);
    money_smg_text.setOrigin(money_smg_text.getLocalBounds().width / 2, money_smg_text.getLocalBounds().height / 2);
    money_smg_text.setPosition(smg_buying.getPosition().x - 20, smg_buying.getPosition().y - 25);
    if (smg_player_intersects && !smg_buy)
        window.draw(money_smg_text);
    //sniper price text 
    money_sniper_text.setFont(normal_font);
    money_sniper_text.setString("$" + to_string(money_sniper));
    money_sniper_text.setCharacterSize(16);
    money_sniper_text.setFillColor(Color::White);
    money_sniper_text.setOrigin(money_sniper_text.getLocalBounds().width / 2, money_sniper_text.getLocalBounds().height / 2);

    money_sniper_text.setPosition(sniper_buying.getPosition().x, sniper_buying.getPosition().y - 25);
    if (sniper_player_intersects && !sniper_buy)
        window.draw(money_sniper_text);
    //shotgun price text 
    money_shotgun_text.setFont(normal_font);
    money_shotgun_text.setString("$" + to_string(money_shotgun));
    money_shotgun_text.setCharacterSize(16);
    money_shotgun_text.setFillColor(Color::White);
    money_shotgun_text.setOrigin(money_shotgun_text.getLocalBounds().width / 2, money_shotgun_text.getLocalBounds().height / 2);

    money_shotgun_text.setPosition(shotgun_buying.getPosition().x - 20, shotgun_buying.getPosition().y - 20);
    if (shotgun_player_intersects && !shotgun_buy)
        window.draw(money_shotgun_text);
    //Rocket price text
    money_Rocket_text.setFont(normal_font);
    money_Rocket_text.setString("$" + to_string(money_rocket));
    money_Rocket_text.setCharacterSize(16);
    money_Rocket_text.setFillColor(Color::White);
    money_Rocket_text.setOrigin(money_Rocket_text.getLocalBounds().width / 2, money_Rocket_text.getLocalBounds().height / 2);

    money_Rocket_text.setPosition(Rocket_buying.getPosition().x, Rocket_buying.getPosition().y - 25);
    if (rocket_player_intersects && !rocket_buy)
        window.draw(money_Rocket_text);
    //speedmachine price text 
    speedmachine_text.setFont(normal_font);
    speedmachine_text.setString("$" + to_string(speedmoney));
    speedmachine_text.setCharacterSize(16);
    speedmachine_text.setFillColor(Color::White);
    speedmachine_text.setPosition(speedmachine.getPosition().x + 20, speedmachine.getPosition().y - 25);
    if (Player.getGlobalBounds().intersects(speedmachine.getGlobalBounds()))
        window.draw(speedmachine_text);
    //
    reloadmachine_text.setFont(normal_font);
    reloadmachine_text.setString("$" + to_string(reloadmoney));
    reloadmachine_text.setCharacterSize(16);
    reloadmachine_text.setFillColor(Color::White);
    reloadmachine_text.setPosition(reloadmachine.getPosition().x + 20, reloadmachine.getPosition().y - 25);
    if (Player.getGlobalBounds().intersects(reloadmachine.getGlobalBounds()))
        window.draw(reloadmachine_text);

    if (!sniper_buy)
    {
        sniper_buying.setOrigin(sniper_buying.getLocalBounds().width / 2, sniper_buying.getLocalBounds().height / 2);

        window.draw(sniper_buying);
        light.setPosition(sniper_buying.getPosition().x, sniper_buying.getPosition().y );
        window.draw(light);
        ambientlight.draw(light);
    }

    //smg
    smg_buying.setTexture(smg_photo);

    if (!smg_buy)
    {
        smg_buying.setOrigin(smg_buying.getLocalBounds().width / 2, smg_buying.getLocalBounds().height / 2);
        window.draw(smg_buying);
        light.setPosition(smg_buying.getPosition().x , smg_buying.getPosition().y );
        window.draw(light);
        ambientlight.draw(light);
    }
    // shotgun 

    if (!shotgun_buy)
    {
        shotgun_buying.setOrigin(shotgun_buying.getLocalBounds().width / 2, shotgun_buying.getLocalBounds().height / 2);

        window.draw(shotgun_buying);
        light.setPosition(shotgun_buying.getPosition().x, shotgun_buying.getPosition().y );
        window.draw(light);
        ambientlight.draw(light);
    }
    if (!rocket_buy)
    {
        Rocket_buying.setOrigin(Rocket_buying.getLocalBounds().width / 2, Rocket_buying.getLocalBounds().height / 2);

        window.draw(Rocket_buying);
        light.setPosition(Rocket_buying.getPosition().x , Rocket_buying.getPosition().y );
        window.draw(light);
        ambientlight.draw(light);
    }
}
void Combo()
{
    Text EnemiesKilledText;
    Text ComboText;
    EnemiesKilledText.setFont(normal_font);
    ostringstream ss;
    EnemiesKilledText.setString(to_string(EnemiesKilledWithoutHit));
    EnemiesKilledText.setCharacterSize(72);
    EnemiesKilledText.setFillColor(Color::White);
    EnemiesKilledText.setOrigin(EnemiesKilledText.getLocalBounds().width / 2, EnemiesKilledText.getLocalBounds().height / 2);
    EnemiesKilledText.setPosition(window.mapPixelToCoords(Vector2i(100, 270)));
    

    ComboText.setFont(normal_font);
    ss << "Score x " << fixed << std::setprecision(1) << float(1 + floor(EnemiesKilledWithoutHit / 10.0f) / 10);
    string str = ss.str();
    ComboText.setString(str);
    ComboText.setCharacterSize(26);
    ComboText.setFillColor(Color::White);
    ComboText.setOrigin(ComboText.getLocalBounds().width / 2, ComboText.getLocalBounds().height / 2);
    ComboText.setPosition(EnemiesKilledText.getPosition().x, EnemiesKilledText.getPosition().y + 50);
    if (EnemiesKilledWithoutHit % 10 == 0 && EnemiesKilledWithoutHit != 0 && EnemiesKilledWithoutHit != previouskilledenemiescombo && !isComboAnimation)
    {
        previouskilledenemiescombo = EnemiesKilledWithoutHit;
        isComboAnimation = true;
        EffectsPlayer.setBuffer(Combo_buffer);
        EffectsPlayer.play();
    }
    if (isComboAnimation)
    {
        Combo_timer_counter += playerdeltatime *4;
        if (Combo_timer_counter <= 6)
        {
            if ((int)Combo_timer_counter % 2 == 0)
            {
                ComboText.setFillColor(Color(136, 8, 8));
                EnemiesKilledText.setFillColor(Color(136, 8, 8));
            }
            else
            {
                ComboText.setFillColor(Color::White);
                EnemiesKilledText.setFillColor(Color::White);
            }
        }
        else
        {
            isComboAnimation = false;
        }
    }
    window.draw(ComboText);
    window.draw(EnemiesKilledText);
}
void UI()
{
    RectangleShape health_bar(Vector2f(170, 15));
    health_bar.setScale(Vector2f((Player_Health / 100.0) * 2, 2));
    health_bar.setPosition(window.mapPixelToCoords(Vector2i(20, 1020)));
    health_bar.setFillColor(Color(136, 8, 8));
    RectangleShape missing_health_bar(Vector2f(170, 15));
    missing_health_bar.setFillColor(Color::White);
    missing_health_bar.setScale(Vector2f(2, 2));
    missing_health_bar.setPosition(window.mapPixelToCoords(Vector2i(20, 1020)));
    RectangleShape health_bar_background(Vector2f(175 * 2, 20 * 2));
    health_bar_background.setFillColor(Color::Black);
    health_bar_background.setPosition(health_bar.getPosition().x - 5, health_bar.getPosition().y - 5);
    window.draw(health_bar_background);
    window.draw(missing_health_bar);
    window.draw(health_bar);

    health_precent_text.setFont(normal_font);
    health_precent_text.setString(to_string(Player_Health));
    health_precent_text.setCharacterSize(52);
    health_precent_text.setPosition(health_bar.getPosition().x + 360, health_bar.getPosition().y-30 );
    health_precent_text.setFillColor(Color(136, 8, 8));
    window.draw(health_precent_text);

    precent_sign.setFillColor(Color(136, 8, 8));
    precent_sign.setFont(normal_font);
   // precent_sign.setScale(2, 2);
    precent_sign.setString(" /100 ");
    precent_sign.setCharacterSize(32);
    if (Player_Health >= 100)
    {
        precent_sign.setPosition(health_bar.getPosition().x + 420, health_bar.getPosition().y);
    }
    else
    {
        precent_sign.setPosition(health_bar.getPosition().x + 400, health_bar.getPosition().y);
    }

    if (current_level == 4 && BossHealth > 0)
    {
        //BossFight HealthBar
        RectangleShape Boss_health_bar(Vector2f(300, 10));
        Boss_health_bar.setScale(Vector2f((BossHealth / 6000.0) * 2, 2));
        Boss_health_bar.setPosition(window.mapPixelToCoords(Vector2i(480, 150)));
        Boss_health_bar.setFillColor(Color(136, 8, 8));
        RectangleShape Boss_missing_health_bar(Vector2f(300, 10));
        Boss_missing_health_bar.setFillColor(Color::White);
        Boss_missing_health_bar.setScale(Vector2f(2, 2));
        Boss_missing_health_bar.setPosition(window.mapPixelToCoords(Vector2i(480, 150)));
        RectangleShape Boss_health_bar_background(Vector2f(305 * 2, 15 * 2));
        Boss_health_bar_background.setFillColor(Color::Black);
        Boss_health_bar_background.setPosition(Boss_health_bar.getPosition().x-5, Boss_health_bar.getPosition().y-5);
        window.draw(Boss_health_bar_background);
        window.draw(Boss_missing_health_bar);
        window.draw(Boss_health_bar);
    }
    else if(current_level != 4)
    {
        // to print  current wave  

        current_wave.setFont(normal_font);
        current_wave.setString(" CurreNT Wave\n\t\t   " + to_string(Current_Wave1));
        current_wave.setCharacterSize(42);
        current_wave.setFillColor(Color(136, 8, 8));
        current_wave.setOrigin(current_wave.getLocalBounds().width / 2, current_wave.getLocalBounds().height / 2);
        current_wave.setPosition(window.mapPixelToCoords(Vector2i(960, 115)));
        window.draw(current_wave);
    }


    window.draw(precent_sign);
    //to draw money photo
    money.setTexture(money_photo);
    money.setScale(Vector2f(0.07, 0.07));
    money.setPosition(window.mapPixelToCoords(Vector2i(0, 920)));
    window.draw(money);
    //{  to draw score and coins title and health percent text  }

    //score

    score_text.setFont(normal_font); // select the font 
    score_text.setString("Score : " + to_string(Score));
    score_text.setCharacterSize(36);
    score_text.setFillColor(Color(136, 8, 8));
    score_text.setPosition(window.mapPixelToCoords(Vector2i(300,930 )));
    window.draw(score_text);

    // to print money number

    money_text.setFont(normal_font); // select the font 
    money_text.setString(" : " + to_string(Money));
    money_text.setCharacterSize(36);
    money_text.setFillColor(Color(255, 215, 0));
    money_text.setPosition(window.mapPixelToCoords(Vector2i(100, 930)));
    window.draw(money_text);

    // amo and amo stack

    current_ammo_text.setFont(normal_font);
    current_ammo_text.setString(to_string(*current_ammo));
    current_ammo_text.setCharacterSize(52);
    current_ammo_text.setFillColor(Color::White);
    if (Curr_Gun_state == Gun_State::Smg)
    {
        current_ammo_text.setPosition(window.mapPixelToCoords(Vector2i(1720, 980)));
    }
    else

    {
        current_ammo_text.setPosition(window.mapPixelToCoords(Vector2i(1720, 980)));
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
    ammo_stock_text.setPosition(window.mapPixelToCoords(Vector2i(1820, 980)));
    ammo_stock_text.setCharacterSize(36);
    ammo_stock_text.setFillColor(Color::White);
    if (Curr_Gun_state != MiniGun)
    {
        window.draw(ammo_stock_text);
        window.draw(current_ammo_text);
    }
    //end draw guns 
    slow_ability.setPosition(window.mapPixelToCoords(Vector2i(1200, 50)));
    MiniGun_ability.setPosition(window.mapPixelToCoords(Vector2i(1200, 50)));
    Turret_ability.setOrigin(Turret_ability.getLocalBounds().width / 2, Turret_ability.getLocalBounds().height / 2);
    Turret_ability.setPosition(window.mapPixelToCoords(Vector2i(960, 100)));
    if (Slowready && current_level == 2)
    {
        window.draw(slow_ability);
    }
    if (isMinigunReady && current_level == 3)
    {
        window.draw(MiniGun_ability);
    }
    if (isTurretReady && current_level == 4)
    {
        window.draw(Turret_ability);
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
        Score += 125;
        ReloadSound.setBuffer(rifle_pickup_sound);
        ReloadSound.play();
    }
    if (Player.getGlobalBounds().intersects(shotgun_buying.getGlobalBounds()) && Money >= money_shotgun && !shotgun_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_shotgun;
        shotgun_buy = true;
        Score += 250;
        ReloadSound.setBuffer(shotgun_pickup_Buffer);
        ReloadSound.play();
    }
    if (Player.getGlobalBounds().intersects(sniper_buying.getGlobalBounds()) && Money >= money_sniper && !sniper_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_sniper;
        sniper_buy = true;
        Score += 500;
        ReloadSound.setBuffer(sniper_pickup_Sound);
        ReloadSound.play();
    }
    if (Player.getGlobalBounds().intersects(Rocket_buying.getGlobalBounds()) && Money >= money_rocket && !rocket_buy && Keyboard::isKeyPressed(Keyboard::E))
    {
        Money -= money_rocket;
        rocket_buy = true;
        Score += 1000;
        ReloadSound.setBuffer(shotgun_pickup_Buffer);
        ReloadSound.play();
    }
}

//Dialogue Function
void Dialogue()
{
    DialogueCounter += playerdeltatime;
    Current_D_Text.setFont(normal_font);
    Current_D_Text.setCharacterSize(36);
    Current_D_Text.setPosition(window.mapPixelToCoords(Vector2i(200, 850)));
    DialogueBox.setOrigin(DialogueBox.getLocalBounds().width / 2, DialogueBox.getLocalBounds().height / 2);
    DialogueBox.setPosition(window.mapPixelToCoords(Vector2i(960, 936)));

    if (Keyboard::isKeyPressed(Keyboard::Space) && current_dialogue <3 && finishedCurrentDialogue)
    {
        DisplayedString.clear();
        finishedCurrentDialogue = false;
        current_dialogue++;
        letterindex = 0;
        DialogueCounter = 0;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Space) && current_dialogue >= 3)
    {
        IsDialogueActive = false;
        //start game
    }

    switch (current_dialogue)
    {
    case 0:
        Current_String = Boss_D_Intro;
        view.setCenter(Boss.getPosition());
        break;
    case 1:
        Current_String = Player_D_Intro;
        view.setCenter(Player.getPosition());
        break;
    case 2:
        Current_String = Boss_D_Outro;
        view.setCenter(Boss.getPosition());
        break;
    }
    if (DialogueCounter > DialogueSpeed && letterindex < Current_String.size())
    {
        DisplayedString += Current_String[letterindex];
        Current_D_Text.setString(DisplayedString);
        letterindex++;
        DialogueCounter = 0;
    }
    else if (letterindex >= Current_String.size() && !finishedCurrentDialogue)
    {
        DisplayedString += "\n -Press Space";
        Current_D_Text.setString(DisplayedString);
        finishedCurrentDialogue = true;
    }
    window.draw(DialogueBox);
    window.draw(Current_D_Text);
}

//Menu Functions
void StartNewGame()
{
    current_level = 1;
    SwtichCurrentWallBounds();
    Player.setPosition(900, 300);
    Player_Health = 100;
    Score = 0;
    menu_num = 0;
    TotalSpawnedZombies = 0;
    PortalOpen = false;
    smg_buy = false;
    shotgun_buy = false;
    speed_pow = false;
    reload_pow = false;
    sniper_buy = false;
    rocket_buy = false;
    speedmulti = 1;
    reloadmulti = 1;
    Money = 0;
    HealthPacks.clear();
    bloodeffects.clear();
    Enemies.clear();
    explosions.clear();
    bullets.clear();
    AmmoPacks.clear();
    muzzleEffects.clear();
    LaserBeams.clear();
    Curr_Gun_state = Pistol;
    Current_Wave1 = 0;
    MusicPlayer.stop();
    current_song = 0;
    MusicPlayer.play();
    BossDone = false;
    IsBossDead = false;
    BossHealth = 6000;
    Ability_Ready = false;
    isCasting = false;
    IsBlind = false;
    LockingIn = false;
    turretLockedin = false;
    currentBulletwaves = 0;
    pistolbulletsloaded = 9;
    riflebulletsloaded = 30;
    shotgunbulletsloaded = 8;
    sniperbulletsloaded = 5;
    rocketbulletsloaded = 1;
    rifleammostock = 120;
    shotgunammostock = 24;
    sniperammostock = 5;
    rocketammostock = 1;
    IsDialogueActive = false;
    finishedCurrentDialogue = false;
    DialogueCounter = 0;
    letterindex = 0;
    current_dialogue = 0;
    MusicPlayer.stop();
    music_trigger = false;
}

void game_openning_menu()
{
    PlayButton.setPosition(globalcenter.x - 500 , globalcenter.y - 100); FloatRect collesion1 = PlayButton.getGlobalBounds();
    PlayButton.setOrigin(PlayButton.getLocalBounds().width / 2, PlayButton.getLocalBounds().height / 2);
    window.draw(PlayButton);
    if (collesion1.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            StartNewGame();
        }
        if (PlayButton.getScale().x < 0.3 && PlayButton.getScale().y < 0.3)
        {
            PlayButton.setScale(PlayButton.getScale().x + playerdeltatime * 2, PlayButton.getScale().x + playerdeltatime *2);
        }
    }
    else
    {
        if (PlayButton.getScale().x > 0.2 && PlayButton.getScale().y > 0.2)
        {
            PlayButton.setScale(PlayButton.getScale().x - playerdeltatime * 2, PlayButton.getScale().x - playerdeltatime *2);
        }
    }
    creditsButton.setPosition(globalcenter.x - 500, globalcenter.y); FloatRect collesion2 = creditsButton.getGlobalBounds();
    creditsButton.setOrigin(PlayButton.getLocalBounds().width / 2, PlayButton.getLocalBounds().height / 2);
    window.draw(creditsButton);
    if (collesion2.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 3;
        }
        if (creditsButton.getScale().x < 0.3 && creditsButton.getScale().y < 0.3)
        {
            creditsButton.setScale(creditsButton.getScale().x + playerdeltatime * 2, creditsButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (creditsButton.getScale().x > 0.2 && creditsButton.getScale().y > 0.2)
        {
            creditsButton.setScale(creditsButton.getScale().x - playerdeltatime * 2, creditsButton.getScale().x - playerdeltatime * 2);
        }
    }
    exitButton.setPosition(globalcenter.x - 500, globalcenter.y + 200); FloatRect collesion3 = exitButton.getGlobalBounds();
    exitButton.setOrigin(PlayButton.getLocalBounds().width / 2, PlayButton.getLocalBounds().height / 2);
    exitButton.setColor(Color(exitButton.getColor().r, exitButton.getColor().g, exitButton.getColor().b, 255));

    window.draw(exitButton);
    if (collesion3.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 4;
        }
        if (exitButton.getScale().x < 0.3 && exitButton.getScale().y < 0.3)
        {
            exitButton.setScale(exitButton.getScale().x + playerdeltatime * 2, exitButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (exitButton.getScale().x > 0.2 && exitButton.getScale().y > 0.2)
        {
            exitButton.setScale(exitButton.getScale().x - playerdeltatime * 2, exitButton.getScale().x - playerdeltatime * 2);
        }
    }


    controlsButton.setPosition(globalcenter.x - 500, globalcenter.y + 100); FloatRect collesion4 = controlsButton.getGlobalBounds();
    controlsButton.setOrigin(PlayButton.getLocalBounds().width / 2, PlayButton.getLocalBounds().height / 2);
    window.draw(controlsButton);
    if (collesion4.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 7;
        }
        if (controlsButton.getScale().x < 0.3 && controlsButton.getScale().y < 0.3)
        {
            controlsButton.setScale(controlsButton.getScale().x + playerdeltatime * 2, controlsButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (controlsButton.getScale().x > 0.2 && controlsButton.getScale().y > 0.2)
        {
            controlsButton.setScale(controlsButton.getScale().x - playerdeltatime * 2, controlsButton.getScale().x - playerdeltatime * 2);
        }
    }
}

void Credits(Font font)
{
    Text first; first.setFont(font); first.setString("Abdullah Sheriff"); first.setFillColor(Color(225, 225, 225, 225)); first.setPosition(globalcenter.x - 300, globalcenter.y - 150); first.setCharacterSize(32); window.draw(first);
    Text second; second.setFont(font); second.setString("Abdelrahman Ahmed Saber"); second.setFillColor(Color(225, 225, 225, 225)); second.setPosition(globalcenter.x - 300, globalcenter.y - 100); second.setCharacterSize(32); window.draw(second);
    Text third; third.setFont(font); third.setString("Abdelrahman Ahmed Ezzat"); third.setFillColor(Color(225, 225, 225, 225)); third.setPosition(globalcenter.x - 300, globalcenter.y - 50); third.setCharacterSize(32); window.draw(third);
    Text fourth; fourth.setFont(font); fourth.setString("Abdelrahman Tamer Mohamed"); fourth.setFillColor(Color(225, 225, 225, 225)); fourth.setPosition(globalcenter.x - 300, globalcenter.y); fourth.setCharacterSize(32); window.draw(fourth);
    Text fifth; fifth.setFont(font); fifth.setString("Shahd Hani"); fifth.setFillColor(Color(225, 225, 225, 225)); fifth.setPosition(globalcenter.x - 300, globalcenter.y +50); fifth.setCharacterSize(32); window.draw(fifth);
    Text sixth; sixth.setFont(font); sixth.setString("Mohamed Magdy"); sixth.setFillColor(Color(225, 225, 225, 225)); sixth.setPosition(globalcenter.x - 300, globalcenter.y + 100); sixth.setCharacterSize(32); window.draw(sixth);
}

void Exit()
{
    escapeButton.setOrigin(escapeButton.getLocalBounds().width / 2, escapeButton.getLocalBounds().height / 2);
    escapeButton.setPosition(globalcenter.x, globalcenter.y - 150);window.draw(escapeButton);

    noButton.setPosition(globalcenter.x - 150, globalcenter.y - 50); FloatRect collesion1 = noButton.getGlobalBounds();
    noButton.setOrigin(noButton.getLocalBounds().width / 2, noButton.getLocalBounds().height / 2);

    window.draw(noButton);
    if (collesion1.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 1;
        }
        if (noButton.getScale().x < 0.3 && noButton.getScale().y < 0.3)
        {
            noButton.setScale(noButton.getScale().x + playerdeltatime * 2, noButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (noButton.getScale().x > 0.2 && noButton.getScale().y > 0.2)
        {
            noButton.setScale(noButton.getScale().x - playerdeltatime * 2, noButton.getScale().x - playerdeltatime * 2);
        }
    }

    yesButton.setPosition(globalcenter.x + 150, globalcenter.y - 50); FloatRect collesion2 = yesButton.getGlobalBounds();
    yesButton.setOrigin(yesButton.getLocalBounds().width / 2, yesButton.getLocalBounds().height / 2);

    window.draw(yesButton);
    if (collesion2.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            ofstream outf("savegame.txt");
            outf << highest_score;
            outf.close();
            window.close();
        }
        if (yesButton.getScale().x < 0.3 && yesButton.getScale().y < 0.3)
        {
            yesButton.setScale(yesButton.getScale().x + playerdeltatime * 2, yesButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (yesButton.getScale().x > 0.2 && yesButton.getScale().y > 0.2)
        {
            yesButton.setScale(yesButton.getScale().x - playerdeltatime * 2, yesButton.getScale().x - playerdeltatime * 2);
        }
    }
}

void Pause()
{

    resumeButton.setPosition(globalcenter.x + 250, globalcenter.y); FloatRect collesion1 = resumeButton.getGlobalBounds();
    resumeButton.setOrigin(resumeButton.getLocalBounds().width / 2, resumeButton.getLocalBounds().height / 2);
    window.draw(resumeButton);
    if (collesion1.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            menu_num = 0;
        }
        if (resumeButton.getScale().x < 0.3 && resumeButton.getScale().y < 0.3)
        {
            resumeButton.setScale(resumeButton.getScale().x + playerdeltatime * 2, resumeButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (resumeButton.getScale().x > 0.2 && resumeButton.getScale().y > 0.2)
        {
            resumeButton.setScale(resumeButton.getScale().x - playerdeltatime * 2, resumeButton.getScale().x - playerdeltatime * 2);
        }
    }
    exitButton.setPosition(globalcenter.x - 250, globalcenter.y); FloatRect collesion2 = exitButton.getGlobalBounds();
    exitButton.setOrigin(exitButton.getLocalBounds().width / 2, exitButton.getLocalBounds().height / 2);
    window.draw(exitButton);
    if (collesion2.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            if (Score > highest_score)
            {
                highest_score = Score;
            }
            menu_num = 1;
            MusicPlayer.setBuffer(music[3]);
            MusicPlayer.play();
            music_trigger = false;
        }
        if (exitButton.getScale().x < 0.3 && exitButton.getScale().y < 0.3)
        {
            exitButton.setScale(exitButton.getScale().x + playerdeltatime * 2, exitButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (exitButton.getScale().x > 0.2 && exitButton.getScale().y > 0.2)
        {
            exitButton.setScale(exitButton.getScale().x - playerdeltatime * 2, exitButton.getScale().x - playerdeltatime * 2);
        }
    }
}

void open_menu()
{
    Vector2i pixelpos = Mouse::getPosition(window);
    MousePos = window.mapPixelToCoords(pixelpos);
    switch (menu_num)
    {
    case 1:game_openning_menu(); break;
    case 3:Credits(normal_font); break;
    case 4:Exit(); break;
    case 5:Pause(); break;
    case 6:Game_over(); break;
    case 7:Controls();
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

void Menu_Background()
{   
    if (menu_num == 5 || menu_num == 6)
    {
        Draw();
    }
    else
    {
        menu_background.setScale(Vector2f(0.65, 0.65));
        menu_background.setPosition(globalcenter.x - (960 * 0.65), globalcenter.y - (540 * 0.65));
        window.draw(menu_background);
        Text high_score; high_score.setFont(normal_font); high_score.setString(" high score : " + to_string(highest_score)); high_score.setFillColor(Color(225, 225, 225, 225)); high_score.setPosition(globalcenter.x + 300, globalcenter.y - 310); high_score.setCharacterSize(32); window.draw(high_score);

    }

}

void Game_over()
{
    MusicPlayer.stop();
    overButton.setOrigin(overButton.getLocalBounds().width / 2, overButton.getLocalBounds().height / 2);
    overButton.setPosition(globalcenter.x, globalcenter.y - 100);window.draw(overButton);
    backButton.setOrigin(backButton.getLocalBounds().width / 2, backButton.getLocalBounds().height / 2);
    backButton.setPosition(globalcenter.x, globalcenter.y); FloatRect collesion2 = backButton.getGlobalBounds();
    window.draw(backButton);
    if (collesion2.contains(MousePos))
    {
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            if (highest_score < Score)
            {
                highest_score = Score;
            }
            EffectsPlayer.stop();
            menu_num = 1;
            MusicPlayer.setBuffer(music[3]);
            MusicPlayer.play();
            music_trigger = false;
        }
        if (backButton.getScale().x < 0.3 && backButton.getScale().y < 0.3)
        {
            backButton.setScale(backButton.getScale().x + playerdeltatime * 2, backButton.getScale().x + playerdeltatime * 2);
        }
    }
    else
    {
        if (backButton.getScale().x > 0.2 && backButton.getScale().y > 0.2)
        {
            backButton.setScale(backButton.getScale().x - playerdeltatime * 2, backButton.getScale().x - playerdeltatime * 2);
        }
    }
}

void Controls()
{
    control.setTexture(Control);

    control.setScale(Vector2f(0.65, 0.65));
    control.setPosition(globalcenter.x - (960 * 0.65), globalcenter.y - (540 * 0.65));
    window.draw(control);
}