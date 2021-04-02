#include "qwebs.h"
#include "qwebs_connection.h"

// cape includes
#include <aio/cape_aio_ctx.h>
#include <aio/cape_aio_timer.h>
#include <sys/cape_err.h>
#include <sys/cape_log.h>
#include <fmt/cape_json.h>

#include <hpp/cape_stc.hpp>

#include <iostream>
#include <fstream> 
#include <string>
#include <vector>

//#include <ctype.h>
//#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#define VOLUME_SECTOR_MAX_X 1000
#define VOLUME_SECTOR_MAX_Y 1000

#define DISPLAY_M_HALF 5
#define DISPLAY_N_HALF 20

//-----------------------------------------------------------------------------
// todo:
// - Frontend Timer (Player Request)
// - PVP
// - Monster
// - more Sectors
// - Weltmatrix-Transfer
//-----------------------------------------------------------------------------


// this shall become the WorldManager
int get_zone(int a_sector_x, int a_sector_y)
{
    return 0;
}


class Entity
{
    
public:     // public members
    
    Entity(std::string a_name, int a_zone, int a_pos_x, int a_pos_y)
    {
        name = a_name;
        zone = a_zone;
        position_x = a_pos_x;
        position_y = a_pos_y;
        
        hp_current = 4;
        hp_max = 4;
        health = calculate_health();
        
        strength = 0;
        dexterity = 0;
        intelligence = 0;
        
    }
    
    std::string get_name() { return name; }
    int get_zone() { return zone; }
    int get_position_x() { return position_x; }
    int get_position_y() { return position_y; }

    int get_hp_current() { return hp_current; }
    int get_hp_max() { return hp_max; }
    int calculate_health() { health = hp_current * 100 / hp_max; return health; }
    bool is_dead() { return hp_current <= 0; }
    
    int get_strength() { return strength; }
    int get_dexterity() { return dexterity; }
    int get_intelligence() { return intelligence; }
    
    int calculate_armor() { return get_dexterity(); }
    int calculate_weapon() { return 1; }
    int create_attack() { return rand() % 20 + 1 + get_dexterity(); }
    int create_damage() { int damage = rand() * calculate_weapon() / RAND_MAX + 1 + get_strength(); if (damage < 0) { damage = 0; } return damage; }
    
    void set_position(int a_x, int a_y) { position_x = a_x; position_y = a_y; }
    void move_right() { position_x++; if (position_x > VOLUME_SECTOR_MAX_X)  { position_x = VOLUME_SECTOR_MAX_X; } }
    void move_left()  { position_x--; if (position_x < 0)                    { position_x = 0; } }
    void move_up()    { position_y++; if (position_y > VOLUME_SECTOR_MAX_Y)  { position_y = VOLUME_SECTOR_MAX_Y; } }
    void move_down()  { position_y--; if (position_y < 0)                    { position_y = 0; } }
    
    
    bool is_attacked(int a_attack, int a_damage)
    {
        bool hit = false;
        if (a_attack >= 10 + calculate_armor()) { hit = true; }
        if (hit) { hp_current -= a_damage; }
        return hit;
    }
    
    void heal()
    {
        hp_current += 1;
        if (hp_current > hp_max) { hp_current = hp_max; }
    }
   
    
protected:    // private attributes
    std::string name;
    int zone;
    int position_x;
    int position_y;
    int health;
    
    // attributes
    int strength, dexterity, intelligence;
    
    int hp_current, hp_max; // health is in percent
    
};

class Monster : public Entity
{
public:
    Monster(std::string a_name, int a_zone, int a_pos_x, int a_pos_y) : Entity(a_name, a_zone, a_pos_x, a_pos_y) {}
};

class Player : public Entity
{
public:
    Player(std::string a_name, int a_zone, int a_pos_x, int a_pos_y) : Entity(a_name, a_zone, a_pos_x, a_pos_y) {}

    void store(void)
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

        std::string to_file = udc_player.to_string();
        std::ofstream MyFile("save_players/" + name + ".txt");
        MyFile << to_file;
        MyFile.close();
    }
    
    void load(void)
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
            
        } else { printf("File does not exist yet!\n"); }
    }
};




std::vector<Player> players;
std::vector<Monster> monsters;

//-----------------------------------------------------------------------------

int __STDCALL main_on_json (void* user_ptr, QWebsRequest request, CapeErr err)
{

    const CapeString method = qwebs_request_method (request);

    //printf ("METHOD: %s\n", method);
    
    if (cape_str_equal (method, "GET"))
    {
        
        // 1. parse request 
        
        
        // get the dictonary of the query part of the request
        CapeMap query_values = qwebs_request_query (request);

        if (query_values)
        {

            CapeMapNode name = cape_map_find(query_values, "name");
        
            if (name) 
            {
                
                // retrieve the value
                const CapeString value = (const CapeString)cape_map_node_value(name);

                printf("received authentication name: %s\n", value);
                
                // find player
                std::vector<Player>::iterator it;     
                for (it=players.begin(); it!=players.end(); ++it)
                {
                    if (it->get_name() == value) { break; }
                }
                    
                if (it != players.end()) // player found
                {
                    
                    // 2. send response

                    cape::Udc udc_players_list(CAPE_UDC_LIST);
                    cape::Udc udc_player_stats(CAPE_UDC_NODE);
                
                    udc_player_stats.add("hp_current", it->get_hp_current());
                    udc_player_stats.add("hp_max", it->get_hp_max());
                    udc_player_stats.add("strength", it->get_strength());
                    udc_player_stats.add("dexterity", it->get_dexterity());
                    udc_player_stats.add("intelligence", it->get_intelligence());
                    udc_player_stats.add("armor", it->calculate_armor());
                    udc_player_stats.add("weapon", it->calculate_weapon());
                    
                    udc_players_list.add(udc_player_stats);

                    std::cout << "UDC to be sent: " <<  udc_players_list << std::endl;
                    
                    qwebs_request_send_buf (&request, udc_players_list.to_string().c_str(), "application/json", err);                
                        
                }
            }
        }
    }      
 
    
  /*
  * POST contains players name, command and position
  *  1. handle players command
  *  2. SEND all players and monsters position and health
  */
  
    else if (cape_str_equal (method, "POST"))
    {
        
        // analyze request ....
        
        CapeStream h = qwebs_request_body (request);
    
        if (h)
        {

            printf("received a command!\n");
            
            const CapeString value = cape_stream_get(h);
            printf("value: %s\n", value);
            
            cape::Udc content = cape_json_from_s (value);
            
            // example: {"player_name":"Sardus","command":"nix"}

            
            if ((false == content.get("player_name").valid()) ||
                (false == content.get("command").valid()) ||
                (false == content.get("player_x").valid()) ||
                (false == content.get("player_y").valid()))
            {
                printf("Problem!\n");
            } else
            {
                std::string player_name = content.get("player_name");
                printf("player name: %s\n", player_name.c_str());
                
                std::string player_cmd = content.get("command");
                printf("player command: %s\n", player_cmd.c_str());
                
                int player_x = content.get("player_x");
                printf("player x: %d\n", player_x);

                int player_y = content.get("player_y");
                printf("player y: %d\n", player_y);
                
                // find player index
                
                if ("Your Name" != player_name)
                {
                                
                    std::vector<Player>::iterator it;     
                    for (it=players.begin(); it!=players.end(); ++it)
                    {
                        if (it->get_name() == player_name) 
                        { 
                            it->set_position(player_x, player_y);
                            break; 
                        }
                    }
                    
                    // there is a new player!
                    if (it == players.end())
                    {                           
                        players.push_back(Player(player_name, 0, player_x, player_y));
                        players[players.size()-1].load(); // try to load (if savegame exists)
                        
                    } else // old player, solve commands:
                
                    {
                    
                        // =========== work on commands ============
                        if ("attack" == player_cmd)
                        {
                            printf("Got attack command!\n");
                            
                            // finding player
                            
                            std::vector<Player>::iterator other_player_it;     
                            for (other_player_it=players.begin(); other_player_it!=players.end(); ++other_player_it)
                            {
                                if  ((other_player_it->get_name() != player_name) && 
                                    ( other_player_it->get_position_x() == it->get_position_x() ) &&
                                    ( other_player_it->get_position_y() == it->get_position_y()))
                                { 
                                    break; 
                                }
                            }
                        
                            if (other_player_it != players.end()) // found some player to attack
                            {
                                
                                int attack = it->create_attack();
                                printf("%s attacks player %s with %d ", it->get_name().c_str(), other_player_it->get_name().c_str(), attack);
                                int damage = it->create_damage();
                                bool hit = other_player_it->is_attacked(attack, damage);
                                
                                if (other_player_it->is_dead()) { printf("and wins!\n"); }
                                else if (!hit) { printf("but misses!\n"); }
                                else  { printf("and causes %d damage!\n", damage); }
                                
                            } else // search monsters                            
                            {
                                
                                std::vector<Monster>::iterator monster_it;     
                                for (monster_it=monsters.begin(); monster_it!=monsters.end(); ++monster_it)
                                {
                                    if (( monster_it->get_position_x() == it->get_position_x() ) &&
                                        ( monster_it->get_position_y() == it->get_position_y()))
                                    { 
                                        break; 
                                    }
                                }
                                
                                if (monster_it != monsters.end()) // found some monster to attack
                                {
                                    
                                    int attack = it->create_attack();
                                    printf("%s attacks %s with %d ", it->get_name().c_str(), monster_it->get_name().c_str(), attack);
                                    int damage = it->create_damage();
                                    bool hit = monster_it->is_attacked(attack, damage);
                                    
                                    if (monster_it->is_dead()) 
                                    { 
                                        printf("and wins!\n"); 
                                        
                                        // delete monster ...
                                        
                                    }
                                    else if (!hit) { printf("but misses!\n"); }
                                    else  { printf("and causes %d damage!\n", damage); }
                                
                                }
                                
                            }
                            
                        }
                        
                        else if ("food" == player_cmd)
                        {
                            printf("Got food command!\n");
                        
                            it->heal();
                        
                        
                        }
                        
                    } // no new player
                    
                } // valid player name
            }
        }
        
        //CapeStream h = qwebs_request_body (request);
        qwebs_request_send_buf (&request, "[{\"name\":\"danger zone\"}]", "application/json", err);
        
    }

    return CAPE_ERR_CONTINUE;
}



int __STDCALL main_on_json_players (void* user_ptr, QWebsRequest request, CapeErr err)
{

  const CapeString method = qwebs_request_method (request);

  //printf ("METHOD: %s\n", method);

  if (cape_str_equal (method, "GET"))
  {
 
      
    // 1. parse request 
        
        
    // get the dictonary of the query part of the request
    CapeMap query_values = qwebs_request_query (request);

    if (query_values)
    {

        CapeMapNode name = cape_map_find(query_values, "name");
        
        if (name) 
        {
                
            // retrieve the value
            const CapeString value = (const CapeString)cape_map_node_value(name);

            printf("received authentication name for entity list: %s\n", value);   
      
      
            // find player
            std::vector<Player>::iterator own_it;     
            for (own_it=players.begin(); own_it!=players.end(); ++own_it)
            {
                if (own_it->get_name() == value) { break; }
            }
                 
            if (own_it != players.end()) // player found
            {
        
                
                // build the player structure
                printf("players list: %d\n", players.size());
                
                
                //cape::Udc root_node(CAPE_UDC_NODE);  // too heavy to parse for now
                cape::Udc udc_entity_list(CAPE_UDC_LIST);
            
                
                for(std::vector<Player>::iterator it=players.begin(); it!=players.end(); ++it)
                {
                    
                    // send only entities that are visible on the screen (1 is for border, 10 for extrapolation)
                    if ((it->get_position_x() > (own_it->get_position_x() - DISPLAY_N_HALF - 11 )) &&
                        (it->get_position_x() < (own_it->get_position_x() + DISPLAY_N_HALF + 11)) &&
                        (it->get_position_y() > (own_it->get_position_y() - DISPLAY_M_HALF - 11)) &&
                        (it->get_position_y() < (own_it->get_position_y() + DISPLAY_M_HALF + 11)))
                    {
                        cape::Udc udc_new_player(CAPE_UDC_NODE);
                        udc_new_player.add("name", it->get_name());
                        udc_new_player.add("zone", it->get_zone());
                        udc_new_player.add("position_x", it->get_position_x());
                        udc_new_player.add("position_y", it->get_position_y());
                        udc_new_player.add("health", it->calculate_health());
                        udc_entity_list.add(udc_new_player);
                    }
                } 
                
                for(std::vector<Monster>::iterator it=monsters.begin(); it!=monsters.end(); ++it)
                {
                    // send only entities that are visible on the screen (1 is for border, 10 for extrapolation)
                    if ((it->get_position_x() > (own_it->get_position_x() - DISPLAY_N_HALF - 11)) &&
                        (it->get_position_x() < (own_it->get_position_x() + DISPLAY_N_HALF + 11)) &&
                        (it->get_position_y() > (own_it->get_position_y() - DISPLAY_M_HALF - 11)) &&
                        (it->get_position_y() < (own_it->get_position_y() + DISPLAY_M_HALF + 11)))
                    {
                        cape::Udc udc_new_monster(CAPE_UDC_NODE);
                        udc_new_monster.add("name", it->get_name());
                        udc_new_monster.add("zone", it->get_zone());
                        udc_new_monster.add("position_x", it->get_position_x());
                        udc_new_monster.add("position_y", it->get_position_y());
                        udc_new_monster.add("health", it->calculate_health());
                        udc_entity_list.add(udc_new_monster); 
                    }
                } 
                
                //root_node.add("players", udc_entity_list); // too heavy to parse for now
                
                
                //std::cout << "UDC to be sent: " <<  udc_entity_list << std::endl;
                
                
                //qwebs_request_send_buf (&request, "[{\"name\":\"second request\"}]", "application/json", err);
                qwebs_request_send_buf (&request, udc_entity_list.to_string().c_str(), "application/json", err);
            }  
        } 
    }
  }

  return CAPE_ERR_CONTINUE;
}


static int __STDCALL callback__on_timer(void* ptr)
{
    
  // move monsters
  
  for(std::vector<Monster>::iterator monster_it=monsters.begin(); monster_it!=monsters.end(); ++monster_it)
  {
      monster_it->move_left();
  }

    
  // store players
    
  try
  {
    //static_cast<GVCPDevice*> (ptr)->on_timer();
      
    // store players (choose one randomly)
    if (players.size() > 0)
    {
        int r = rand() % players.size();
        printf("storing player %s\n", players[r].get_name().c_str());
        players[r].store();
    }
    
    return TRUE;
  }
  catch (std::runtime_error& e)
  {
    cape_log_fmt (CAPE_LL_ERROR, "WEBS", "on timer", e.what());
    return FALSE;
  }
  catch (...)
  {
    cape_log_fmt (CAPE_LL_ERROR, "WEBS", "on timer", "unknown exception");
    return FALSE;
  }
}

//-----------------------------------------------------------------------------


//% testopt -c foo
//cvalue = foo

int main (int argc, char *argv[])
{
  int res;
  CapeErr err = cape_err_new ();
  srand(0);
  
  // local objects
  CapeAioContext aio_context = NULL;
  QWebs webs = NULL;
  CapeUdc sites = NULL;

  // allocate memory for the AIO subsystem
  aio_context = cape_aio_context_new ();

  // setup timer
  
  CapeAioTimer timer = cape_aio_timer_new();
  
  res = cape_aio_timer_set (timer, 5000, NULL, callback__on_timer, err);
  if(res){
    goto exit_and_cleanup;
  }

  // try to start the AIO
  res = cape_aio_context_open (aio_context, err);
  if (res)
  {
    goto exit_and_cleanup;
  }

  
  // now that context is there, start timer  
  res = cape_aio_timer_add(&timer, aio_context);
  if(res){
    goto exit_and_cleanup;
  } 
  
  // enable interupt by ctrl-c
  res = cape_aio_context_set_interupts (aio_context, CAPE_AIO_ABORT, CAPE_AIO_ABORT, err);
  if (res)
  {
    goto exit_and_cleanup;
  }
  
  // create a node to contain all sites
  sites = cape_udc_new (CAPE_UDC_NODE, NULL); // was CAPE_UDC_LIST before

  // add the default site 'public'
  cape_udc_add_s_cp (sites, "/", "public");

  // allocate memory and initialize the qwebs library context
  webs = qwebs_new (sites, "127.0.0.1", 80, 4, "pages", NULL);
  // 80 is the only possible port because device runs on it and cannot receive ports in the URL

  // register an API which can be called by http://127.0.0.1/json/
  res = qwebs_reg (webs, "json", NULL, main_on_json, err);
  if (res)
  {
    goto exit_and_cleanup;
  }
  
  // register players list (POST -> interpret command and send overview about all players and monsters)
  res = qwebs_reg (webs, "players", NULL, main_on_json_players, err);
  if (res)
  {
    goto exit_and_cleanup;
  }
  
  // prepare monsters
  for (int i=0; i<3000; ++i) monsters.push_back(Monster("rat", 0, rand() % 1000, rand() % 1000));
  
  

  /*
  res = qwebs_reg_page (webs, "hidden.htm", NULL, main_on_page, err);
  if (res)
  {
    goto exit_and_cleanup;
  }*/

  // finally attach the qwebs library to the AIO subsystem
  // -> this will enable all events
  // -> at this point request can be processes
  res = qwebs_attach (webs, aio_context, err);
  if (res)
  {
    goto exit_and_cleanup;
  }

  // handle all events
  res = cape_aio_context_wait (aio_context, err);

exit_and_cleanup:

  if (cape_err_code (err))
  {
    cape_log_fmt (CAPE_LL_ERROR, "QWEBS", "main", "fetched error: %s", cape_err_text (err));
  }

  cape_udc_del (&sites);
  qwebs_del (&webs);
  cape_aio_context_del (&aio_context);

  cape_err_del (&err);

  return res;
}

//-----------------------------------------------------------------------------

// todo:
// clear players and monsters at destruction
// improve heal/food function (level based)
