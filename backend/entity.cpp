#include "entity.h"

// Entity methods

Entity::Entity(std::string a_name, int a_zone, int a_pos_x, int a_pos_y, int a_strength, int a_dexterity, int a_intelligence, int a_health, int a_give_xp)
{
    name = a_name;
    zone = a_zone;
    position_x = a_pos_x;
    position_y = a_pos_y;
        
    hp_current = a_health;
    hp_max = a_health;
    health = calculate_health();
        
    strength = a_strength;
    dexterity = a_dexterity;
    intelligence = a_intelligence;
        
    give_xp = a_give_xp;
     
}

bool Entity::is_attacked(int a_attack, int a_damage)
{
    bool hit = false;
    if (a_attack >= 10 + calculate_armor()) { hit = true; }
    if (hit) { hp_current -= a_damage; }
    if (hp_current < 0) { hp_current = 0; }
    return hit;
}
    
void Entity::heal(int a_boost_hp)
{
    hp_current += a_boost_hp;
    if (hp_current > hp_max) { hp_current = hp_max; }
}
    

// Player methods


Player::Player(std::string a_name, int a_zone, int a_pos_x, int a_pos_y) : Entity(a_name, a_zone, a_pos_x, a_pos_y, 0, 0, 0, 4, 10) 
{
    level = 0;
    experience = 0;
    next_experience = 0;
    give_attribute_points = 0;
    status_message = "Welcome " + a_name + "!";
}

void Player::store(void)
{
    cape::Udc udc_player(CAPE_UDC_NODE);
    udc_player.add("name", name);
    udc_player.add("zone", zone);
    udc_player.add("position_x", position_x);
    udc_player.add("position_y", position_y);
    udc_player.add("hp_current", hp_current);
    udc_player.add("hp_max",     hp_max);
    udc_player.add("strength",   strength);
    udc_player.add("dexterity",  dexterity);
    udc_player.add("intelligence", intelligence);
    udc_player.add("experience", experience);
    udc_player.add("give_attribute_points", give_attribute_points);

    std::string to_file = udc_player.to_string();
    std::ofstream MyFile("save_players/" + name + ".txt");
    MyFile << to_file;
    MyFile.close();
}

void Player::load(void)
{
    CapeErr err;
    std::string filename = "save_players/" + name + ".txt";
        
    // check existance of savegame
    std::ifstream myFile;
    myFile.open(filename.c_str());
    if(myFile) 
    {
            
        myFile.close();
        printf("loading player from %s ... ", filename.c_str()); 
                       
        cape::Udc content = cape_json_from_file (filename.c_str(), err);
          
        if (content.get("position_x").valid())  { position_x    = content.get("position_x"); }
        if (content.get("position_y").valid())  { position_y    = content.get("position_y"); }
        if (content.get("zone").valid())        { zone          = content.get("zone"); }
        if (content.get("hp_current").valid())  { hp_current    = content.get("hp_current"); }
        if (content.get("hp_max").valid())      { hp_max        = content.get("hp_max"); }
        if (content.get("strength").valid())    { strength      = content.get("strength"); }
        if (content.get("dexterity").valid())   { dexterity     = content.get("dexterity"); }
        if (content.get("intelligence").valid()) { intelligence = content.get("intelligence"); }
        if (content.get("experience").valid())  { experience = content.get("experience"); }
        if (content.get("give_attribute_points").valid())  { give_attribute_points = content.get("give_attribute_points"); }
            
        calculate_current_level();
        calculate_next_level();
            
    } else { printf("File does not exist yet!\n"); }
}
   
void Player::give_attribute(int a_attribute)
{
    if (give_attribute_points > 0)
    {
        if (0 == a_attribute) { strength++; }
        else if (1 == a_attribute) { dexterity++; }
        else { intelligence++; }
            
        give_attribute_points--;
           
        if (give_attribute_points == 0) // finalize level up
        {
            experience -= next_experience;
            calculate_current_level();
            calculate_next_level();  
        }
    }
}
        

int Player::boost_xp(int a_boost) 
{
    
    calculate_current_level();
    calculate_next_level();

    experience += a_boost;
        
    // level up
    if (experience >= next_experience)
    {
        give_attribute_points += 1;            
        hp_max += rand() % (strength + 1) + 1;
        hp_current = hp_max;
            
    }
}

   

