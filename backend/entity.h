#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <iostream>
#include <fstream> 
#include <string>

#include <fmt/cape_json.h>
#include <hpp/cape_stc.hpp>


#define VOLUME_SECTOR_MAX_X 1000
#define VOLUME_SECTOR_MAX_Y 1000

class Entity
{
    
public:     // public members
    
    Entity(std::string a_name, int a_pos_x, int a_pos_y, int a_strength, int a_dexterity, int a_intelligence, int a_health, int a_give_xp);
    
    std::string get_name() { return name; }
    int get_position_x() { return position_x; }
    int get_position_y() { return position_y; }

    int get_hp_current() { return hp_current; }
    int get_hp_max() { return hp_max; }
    int calculate_health() { health = hp_current * 100 / hp_max; return health; }
    bool is_dead() { return hp_current <= 0; }
    
    int get_strength() { return strength; }
    int get_dexterity() { return dexterity; }
    int get_intelligence() { return intelligence; }
    
    int calculate_armor() { return 0; } // regard items
    int calculate_weapon() { return 1; } // regard items
    int create_attack() { return rand() % 20 + 1 + get_dexterity(); }
    int create_damage() { int damage = rand() * calculate_weapon() / RAND_MAX + 1 + get_strength(); if (damage < 0) { damage = 0; } return damage; }
    
    void set_position(int a_x, int a_y) { position_x = a_x; position_y = a_y; }
    void move_right() { position_x++; if (position_x > VOLUME_SECTOR_MAX_X)  { position_x = VOLUME_SECTOR_MAX_X; } }
    void move_left()  { position_x--; if (position_x < 0)                    { position_x = 0; } }
    void move_up()    { position_y++; if (position_y > VOLUME_SECTOR_MAX_Y)  { position_y = VOLUME_SECTOR_MAX_Y; } }
    void move_down()  { position_y--; if (position_y < 0)                    { position_y = 0; } }
    
    
    bool is_attacked(int a_attack, int a_damage);    
    void heal(int a_boost_hp);
    
    int get_give_xp() { return give_xp; }

    
protected:    // private attributes
    std::string name;    
    int position_x;
    int position_y;
    
    int health;
    
    // attributes
    int strength, dexterity, intelligence;
    
    // health
    int hp_current, hp_max; // health is in percent
    
    // experience
    int give_xp; // XP to enemy when defeated
    
};

class Monster : public Entity
{
private:
    int m_view_x;
    int m_view_y;
    
public:
    Monster(std::string a_name, int a_zone, int a_pos_x, int a_pos_y, int a_view_x, int a_view_y, int a_strength, int a_dexterity, int a_intelligence, int a_health, int a_give_xp) : 
        Entity(a_name, a_pos_x, a_pos_y, a_strength, a_dexterity, a_intelligence, a_health, a_give_xp) 
        {
            m_view_x = a_view_x; if (m_view_x < 0) { m_view_x = 0; }
            m_view_y = a_view_y; if (m_view_y < 0) { m_view_y = 0; }
        }
    int get_view_x() { return m_view_x; }
    int get_view_y() { return m_view_y; }
    
};

class Player : public Entity
{
    
public:
    Player(std::string a_name, int a_sector_x, int a_sector_y, int a_zone, int a_pos_x, int a_pos_y);
    int calculate_current_level() { level = strength + dexterity + intelligence; return level; }
    int calculate_next_level() { if (level == 0) { next_experience = 10; } else { next_experience = (level+9) * 3; } return next_experience; }
               
    void store(void);
    void load(void);
    
    int get_zone() { return zone; }
    int get_level() { return level; }
    int get_experience() { return experience; }
    int get_next_experience() { return next_experience; }
    int get_give_attribute_points() { return give_attribute_points; }
    std::string get_status_message() { return status_message; }
    
    void give_strength() { give_attribute(0); }
    void give_dexterity() { give_attribute(1); }
    void give_intelligence() { give_attribute(2); }
    
    void give_attribute(int a_attribute);
    int boost_xp(int a_boost);
    void set_status_message(std::string a_message) { status_message = a_message; }

private:
            
    int sector_x;
    int sector_y;
    int zone;

    // === attributes ===
    
    // experience
    int level, experience, next_experience, give_attribute_points;
    std::string status_message; 
    
};

#endif
