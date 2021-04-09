#include "qwebs.h"
#include "qwebs_connection.h"

// cape includes
#include <aio/cape_aio_ctx.h>
#include <aio/cape_aio_timer.h>
#include <sys/cape_err.h>
#include <sys/cape_log.h>


#include <vector>

//#include <ctype.h>
//#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


// constant that should be known also in entity module


#define DISPLAY_M_HALF 5
#define DISPLAY_N_HALF 20

#include "entity.h"


// this shall become the WorldManager
int get_zone(int a_sector_x, int a_sector_y)
{
    return 0;
}

int get_sector_food(void) { return 10; }




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
                    udc_player_stats.add("level", it->get_level());
                    udc_player_stats.add("experience", it->get_experience());
                    udc_player_stats.add("next_experience", it->get_next_experience());
                    udc_player_stats.add("give_points", it->get_give_attribute_points());
                    
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
                            //it->set_position(player_x, player_y);
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
                        
                             if ("move_east" == player_cmd) { it->move_right(); }
                        else if ("move_west" == player_cmd) { it->move_left(); }
                        else if ("move_north" == player_cmd) { it->move_up(); }
                        else if ("move_south" == player_cmd) { it->move_down(); }
                        
                        else if ("give_strength" == player_cmd) { it->give_strength(); }
                        else if ("give_dexterity" == player_cmd) { it->give_dexterity(); }
                        else if ("give_intelligence" == player_cmd) { it->give_intelligence(); }
                        
                        else if ("attack" == player_cmd)
                        {
                            
                            // finding player or monster
                            int index_other_player = -1;
                            int index_other_monster = -1;
                            
                            for (int i=0; i<players.size(); ++i)
                            {
                                if ((player_name != players[i].get_name()) &&
                                    (it->get_position_x() == players[i].get_position_x()) &&
                                    (it->get_position_y() == players[i].get_position_y()))
                                {
                                    index_other_player = i;
                                    break;
                                }
                            }
                            
                            if (index_other_player == -1)
                            {
                                //printf("how many monsters: %d\n", monsters.size());
                                    
                                for (int i=0; i<monsters.size(); ++i)
                                {
                                    if ((it->get_position_x() == monsters[i].get_position_x()) &&
                                        (it->get_position_y() == monsters[i].get_position_y()))
                                    {
                                        index_other_monster = i;
                                        break;
                                    }
                                        
                                }  
                            }
                            
                            // found enemy -> fight
                            if ((index_other_player > -1) || (index_other_monster > -1)) 
                            { 
                                
                                int attack = it->create_attack();
                                
                                printf("%s attacks %s with %d ", it->get_name().c_str(), 
                                       (index_other_player > -1)?players[index_other_player].get_name().c_str():
                                        monsters[index_other_monster].get_name().c_str(), attack);
                                
                                int damage = it->create_damage();
                                
                                bool hit = false;
                                
                                if (index_other_player > -1) { hit = players[index_other_player].is_attacked(attack, damage); }
                                else if (index_other_monster > -1) { hit = monsters[index_other_monster].is_attacked(attack, damage); }
          
                                if (index_other_player > -1)
                                {
                                    if (players[index_other_player].is_dead()) { printf("and wins!\n"); }
                                    else if (!hit) { printf("but misses!\n"); }
                                    else  { printf("and causes %d damage!\n", damage); }                                    
                                } else if (index_other_monster > -1)
                                {
                                    
                                    if (monsters[index_other_monster].is_dead())
                                    { 
                                        printf("and wins!\n"); 
                                        
                                        // give xp
                                        it->boost_xp(monsters[index_other_monster].get_give_xp());
                                        
                                        // delete monster
                                        monsters[index_other_monster] = monsters[monsters.size()-1];
                                        monsters.pop_back();
                                        
                                    }
                                    else if (!hit) { printf("but misses!\n"); }
                                    else  { printf("and causes %d damage!\n", damage); }                               
                                    
                                }
                            } 
                        }
                        
                        else if ("food" == player_cmd)
                        {
                            
                            int roll = rand() % 20 + 1;
                            if (roll >= get_sector_food() - it->get_level())
                            {
                                if (roll == 20) { it->heal(rand()%6+1); } else { it->heal(1); }
                            }
                        
                        
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
            
        // find player
        int index_player = -1;
                            
        // todo: move KI into monster class
        for (int i=0; i<players.size(); ++i)
        {
            if ((monster_it->get_position_x() > (players[i].get_position_x() - monster_it->get_view_x())) &&
                (monster_it->get_position_x() < (players[i].get_position_x() + monster_it->get_view_x())) &&
                (monster_it->get_position_y() > (players[i].get_position_y() - monster_it->get_view_y())) &&
                (monster_it->get_position_y() < (players[i].get_position_y() + monster_it->get_view_y())))
            {
                //printf("found player (%d, %d): %d\n", monster_it->get_position_x(), monster_it->get_position_y(), i);
                index_player = i;
                break;
            }
        }
      
        if (index_player == -1) // nothing found -> walk randomly at 1-4, rest at 5-6
        {
            int r = rand() % 6;
                 if (1 == r) { monster_it->move_left(); }
            else if (2 == r) { monster_it->move_right(); }
            else if (3 == r) { monster_it->move_up(); }
            else if (4 == r) { monster_it->move_down(); }
          
        }
        else // move to player and attack
        {           
              
            if ((monster_it->get_position_x() == players[index_player].get_position_x()) &&
                (monster_it->get_position_y() == players[index_player].get_position_y()))
            {
                // monster attacks player
                
                int attack = monster_it->create_attack();                                
                printf("%s attacks %s with %d ", monster_it->get_name().c_str(), players[index_player].get_name().c_str(), attack);
                int damage = monster_it->create_damage();
                bool hit = players[index_player].is_attacked(attack, damage); 

                if (players[index_player].is_dead()) { printf("and wins!\n"); }
                else if (!hit) { printf("but misses!\n"); }
                else  { printf("and causes %d damage!\n", damage); }                            
                
            }
            else
            {
                // movement (straight forward)
                
                     if (monster_it->get_position_x() > players[index_player].get_position_x()) { monster_it->move_left(); }
                else if (monster_it->get_position_x() < players[index_player].get_position_x()) { monster_it->move_right(); }
                     if (monster_it->get_position_y() > players[index_player].get_position_y()) { monster_it->move_down(); }
                else if (monster_it->get_position_y() < players[index_player].get_position_y()) { monster_it->move_up(); }
                
            }
            
        }
      
      
      
      
      
      
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
  
  res = cape_aio_timer_set (timer, 3000, NULL, callback__on_timer, err);
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
  for (int i=0; i<3000; ++i) monsters.push_back(Monster("rat",      0, rand() % 1000, rand() % 1000, 12, 3, 0, -5, 0, 1, 1));
  for (int i=0; i<3000; ++i) monsters.push_back(Monster("dronte",   0, rand() % 1000, rand() % 1000, 12, 3, 0, -3, 0, 2, 2));
  for (int i=0; i<3000; ++i) monsters.push_back(Monster("bush",     0, rand() % 1000, rand() % 1000, 12, 3, 0, -1, 0, 3, 3));
 
 

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
// timeout (frontend) after each action
// sectors with monsters
// server delivers sectors
// dungeons
// collecting
// inventory
// crafting
// magic
