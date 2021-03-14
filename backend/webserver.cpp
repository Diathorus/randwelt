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


class Player
{
    
public:     // public members
    
    Player(std::string a_name, int a_zone, int a_pos_x, int a_pos_y)
    {
        name = a_name;
        zone = a_zone;
        position_x = a_pos_x;
        position_y = a_pos_y;
    }
    
    std::string get_name() { return name; }
    int get_zone() { return zone; }
    int get_position_x() { return position_x; }
    int get_position_y() { return position_y; }
    
    void set_position(int a_x, int a_y) { position_x = a_x; position_y = a_y; }
    
    void store(void)
    {
        cape::Udc udc_player(CAPE_UDC_NODE);
        udc_player.add("name", name);
        udc_player.add("zone", zone);
        udc_player.add("position_x", position_x);
        udc_player.add("position_y", position_y);

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

            printf("success (x: %d, y: %d)!\n", position_x, position_y);
            
        } else { printf("file does not exist yet!\n"); }
    }
    
    
private:    // private attributes
    std::string name;
    int zone;
    int position_x;
    int position_y;
    
};

std::vector<Player> players;


//-----------------------------------------------------------------------------

int __STDCALL main_on_json (void* user_ptr, QWebsRequest request, CapeErr err)
{

  const CapeString method = qwebs_request_method (request);

  printf ("METHOD: %s\n", method);

  if (cape_str_equal (method, "GET"))
  {
    CapeUdc content = cape_json_from_file ("data.json", err);

    if (content)
    {
        // everything is fine
        qwebs_request_send_json (&request, content, err);
    }
  else
  {
    // do somthing to handle error
    qwebs_request_send_json (&request, NULL, err);
  }

  cape_udc_del (&content);
  }
  
  
  /*
   * the only POST contains:
   *  1. players name
   *  2. typed command
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
        
        //CapeUdc content = cape_json_from_s (value);
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
                }
            
            }
        }
    }
   
    printf("Greetings, new player!\n");
    
    //CapeStream h = qwebs_request_body (request);
    qwebs_request_send_buf (&request, "[{\"name\":\"danger zone\"}]", "application/json", err);

  }

  return CAPE_ERR_CONTINUE;
}



int __STDCALL main_on_json_players (void* user_ptr, QWebsRequest request, CapeErr err)
{

  const CapeString method = qwebs_request_method (request);

  printf ("METHOD: %s\n", method);

  if (cape_str_equal (method, "GET"))
  {
 
      /*CapeUdc content = cape_json_from_file ("data.json", err);

    if (content)
    {
        // everything is fine
        qwebs_request_send_json (&request, content, err);
    }
    else
    {
        // do somthing to handle error
        qwebs_request_send_json (&request, NULL, err);
    }
    
    
    cape_udc_del (&content);
    */
      
      
    
    // build the player structure
    printf("players list: %d\n", players.size());
    
    
    //cape::Udc root_node(CAPE_UDC_NODE);  // too heavy to parse for now
    cape::Udc udc_players_list(CAPE_UDC_LIST);
  
    
    for(std::vector<Player>::iterator it=players.begin(); it!=players.end(); ++it)
    {
        cape::Udc udc_new_player(CAPE_UDC_NODE);
        udc_new_player.add("name", it->get_name());
        udc_new_player.add("zone", it->get_zone());
        udc_new_player.add("position_x", it->get_position_x());
        udc_new_player.add("position_y", it->get_position_y());
        udc_players_list.add(udc_new_player); // too heavy to parse for now
    } 
    
    //root_node.add("players", udc_players_list); // too heavy to parse for now
    
    
    std::cout << "UDC to be sent: " <<  udc_players_list << std::endl;
      
    
    //qwebs_request_send_buf (&request, "[{\"name\":\"second request\"}]", "application/json", err);
    qwebs_request_send_buf (&request, udc_players_list.to_string().c_str(), "application/json", err);
  }

  return CAPE_ERR_CONTINUE;
}


static int __STDCALL callback__on_timer(void* ptr)
{
  try
  {
    //static_cast<GVCPDevice*> (ptr)->on_timer();
      
    printf("on timer!\n");
      
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
  
  res = cape_aio_timer_set (timer, 10000, NULL, callback__on_timer, err);
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
  
  // register players list
  res = qwebs_reg (webs, "players", NULL, main_on_json_players, err);
  if (res)
  {
    goto exit_and_cleanup;
  }
  

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

