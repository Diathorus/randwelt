import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-ingame',
  templateUrl: './ingame.component.html',
  styleUrls: ['./ingame.component.css']
})

export class IngameComponent implements OnInit
{

  // human indication mapping for @
  display_human_own_player : string = "1";
  display_human_other_player : string = "2";


  html_answer_1: string;
  html_answer_2: string;
  zone_name: string;
  second: string;

  players: Entity[];
  player_number : number;

  own_player_x : number = 3;
  own_player_y : number = 3;
  field_on_player : string;

  VOLUME_M = 10;
  VOLUME_N = 40;

  clean_matrix:string[][]; // matrix without entities
  final_matrix:string[][];

  constructor(private http: HttpClient)
  {
    this.createWorld(0);
    this.applyPlayersToMatrix();
  }


  requestHTTP()
  {

    class Data { name: string };
    var data: Data[]

    // send data to the server
    this.http.post("json/x", JSON.stringify ({player_name:this.authentication, command:this.command_message, player_x:this.own_player_x, player_y:this.own_player_y})).subscribe ((data) =>
    {
      this.zone_name = data[0].name           // next callback
    });

    // get data from the server (for now from file)
    this.http.get("json/x").subscribe ((data) =>
    {
      this.html_answer_1 = data[0].name            //Next callback
      this.html_answer_2 = data[1].name
    },
    (error) => {                                   //Error callback
          console.error('Request failed with error')
          alert(error);
    },
    () => {                                        //Complete callback
          console.log('Request completed')
    });

    // get other players from the server


    var parsing_players: Entity[];


    this.http.get("players/x").subscribe ((parsing_players) => // data works
    {

      var j: any;

      // we have to delete the players list, in case that real players list shrinks
      this.players = [];

      //var output : string = "online: "; this.player_number = 0;
      for(j in parsing_players)
      {
        this.players[j] = new Entity();
        this.players[j].name = parsing_players[j].name;
        this.players[j].zone = parsing_players[j].zone;
        this.players[j].position_x = parsing_players[j].position_x;
        this.players[j].position_y = parsing_players[j].position_y;

        if (this.authentication == this.players[j].name)
        {
          this.own_player_x = this.players[j].position_x; // for further movement
          this.own_player_y = this.players[j].position_y; // for further movement
        }

        this.applyPlayersToMatrix();

        //output += parsing_players[j].name + ", "; this.player_number++;

      }
      //alert (output);

    },
    (error) => {                                   //Error callback
          console.error('Request failed with error')
          alert(error);
    },
    () => {                                        //Complete callback
          console.log('Request completed')
    });
  }


  // this part needs to be moved to the server!



  // descriptive texts

  text_zone_description : string = "default zone description";
  text_status_message : string = "status message";
  text_label_zone : string = "default";

  // command string

  command_message : string = "Your Name";

  // old variables

  authentication : string = "Your Name";
  first : boolean = true;

  //zone_name : string = "";//"monastery_barracks"

  round : number = 0;
  zone : number = 0;
  strength : number = 0;
  dexterity : number = 0;
  intelligence : number = 0;
  give_me_points : number = 0;
  level : number = 1;
  current_health : number = 10;
  max_health : number = 10;
  coins : number = 0;
  next_coins : number = 0;
  current_weapon_damage : number = 3;
  current_armor : number = 0;

  /* survival */
  thirst : boolean = false;
  hunger : number = 0;

  /* effects */
  charisma_bonus_small : number = 0;
  charisma_bonus_large : number = 0;

  /* collecting skills */
  skill_flower_plucking : number = 0;
  skill_wood_lumbering : number = 0;
  skill_ore_mining : number = 0;
  skill_carcass_gutting : number = 0;

  /* crafting skills */
  skill_potion_brewing : number = 0;
  skill_leather_working : number = 0;
  skill_stone_masoning : number = 0;
  skill_wood_carpentering : number = 0;

  /* monster */
  Monster_Type = {
    MONSTER_NONE: 0, MONSTER_BAT_SMALL: 1, MONSTER_FOX: 2, MONSTER_LEPER: 3,
    MONSTER_GNOME: 4, MONSTER_BEETLE_FIRE: 5, MONSTER_RAT_GIANT: 6,
    MONSTER_THUG: 7, MONSTER_FUNGUS_MAN: 8, MONSTER_TRAINING_1: 9,
    MONSTER_TRAINING_2: 10, MONSTER_TRAINING_3: 11,

    // zone 0: light forest: rat, fox, horned rabbit, slug, ...
    // zone 1: highlands: ...
    // zone 2: dark forest: crows
    // zone 3: woodcutter village
    // zone 4: cursed forest



    properties: {
      0: {label: "no monsters" ,  strength: 0,  dexterity: 0, intelligence: 0, health_dice: 0, health_number: 0 },
      1: {label: "monstrous bat", strength: -2, dexterity: 3, intelligence: 0, health_dice: 6, health_number: 1 },
      2: {label: "hungry fox",    strength: -1, dexterity: 2, intelligence: 1, health_dice: 4, health_number: 2 },
      3: {label: "sick leper",    strength: 1,  dexterity: 0, intelligence: 1, health_dice: 6, health_number: 2 },
      4: {label: "rat gnome",     strength: -1, dexterity: 4, intelligence: 1, health_dice: 5, health_number: 2 },
      5: {label: "fire beetle",   strength: 2,  dexterity: 1, intelligence: 0, health_dice: 4, health_number: 3 },
      6: {label: "giant rat",     strength: 1,  dexterity: 3, intelligence: 1, health_dice: 4, health_number: 2 },
      7: {label: "brutal thug",   strength: 3,  dexterity: 2, intelligence: 0, health_dice: 6, health_number: 2 },
      8: {label: "fungus-with-man",strength: 5, dexterity: 1, intelligence: 0, health_dice: 8, health_number: 2 },
      9:  {label: "weak training device",       strength: -1, dexterity: -1, intelligence: 0, health_dice: 1, health_number: 1 },
      10: {label: "training device",            strength: 0, dexterity: 0, intelligence: 0, health_dice: 2, health_number: 1 },
      11: {label: "reinforced training device", strength: 1, dexterity: 1, intelligence: 0, health_dice: 3, health_number: 1 }
    }

  };

  /* enemies */
  monster_type = this.Monster_Type.MONSTER_NONE;

  monster_health : number = 0;
  monster_encounter : boolean = false;
  monster_corpse : boolean = false;


  /* inventory */
  Item_Type = {
    ITEM_EMPTY: 0, ITEM_FLOWER_RED: 1, ITEM_FLOWER_BLUE: 2, ITEM_FLOWER_YELLOW: 3,
    ITEM_WOOD_BLUE: 4, ITEM_WOOD_YELLOW: 5, ITEM_WOOD_RED: 6,
    ITEM_MINING_CLAY: 7, ITEM_MINING_IRON: 8, ITEM_MINING_GOLD: 9,
    ITEM_POTION_RED: 10, ITEM_POTION_BLUE: 11, ITEM_POTION_YELLOW: 12,
    ITEM_POTION_PURPLE: 13, ITEM_POTION_ORANGE: 14, ITEM_POTION_GREEN: 15,
    ITEM_POTION_WHITE: 16, ITEM_POTION_BLACK: 17,
    ITEM_WOOD_BEAM: 18, ITEM_WOOD_PLANK: 19, ITEM_WOOD_SPAR: 20, ITEM_WOOD_LADDER: 21,
    ITEM_MINING_BRICK: 22, ITEM_MINING_INGOT: 23,
    ITEM_GUTTING_FLESH: 24, ITEM_GUTTING_FUR: 25,
    ITEM_ROTTED_FLESH: 26, ITEM_PELT_ARMOR: 27,
    ITEM_FOOD_APPLE: 28,

    properties: {
      0: {label: " " , price: 0, weight: 9999},
      1: {label: "red flower", price: 1, weight: 10 },
      2: {label: "blue flower", price: 2, weight: 10 },
      3: {label: "yellow flower", price: 3, weight: 10 },
      4: {label: "blue wood", price: 1, weight: 1 },
      5: {label: "yellow wood", price: 2, weight: 1 },
      6: {label: "red wood", price: 3, weight: 1 },
      7: {label: "clay", price: 2, weight: 1 },
      8: {label: "iron nugget", price: 3, weight: 2 },
      9: {label: "gold nugget", price: 4, weight: 2 },
     10: {label: "red potion", price: 3, weight: 7 },
     11: {label: "blue potion", price: 4, weight: 7 },
     12: {label: "yellow potion", price: 5, weight: 7 },
     13: {label: "purple potion", price: 8, weight: 7 },
     14: {label: "orange potion", price: 9, weight: 7},
     15: {label: "green potion", price: 10, weight: 7 },
     16: {label: "white potion", price: 15, weight: 7 },
     17: {label: "black potion", price: 20, weight: 7 },
     18: {label: "wood beam", price: 2, weight: 1 },
     19: {label: "wood plank", price: 3, weight: 1 },
     20: {label: "wood spar", price: 4, weight: 1 },
     21: {label: "wood ladder", price: 4, weight: 0.3 },
     22: {label: "brick", price: 5, weight: 1 },
     23: {label: "iron ingot", price: 7, weight: 1 },
     24: {label: "flesh", price: 1, weight: 7 },
     25: {label: "fur", price: 3, weight: 2 },
     26: {label: "flesh", price: 0, weight: 7 },
     27: {label: "pelt armor", price: 10, weight: 1},
     28: {label: "apple", price: 1, weight: 7}
    }

  };

  //var myItem = Item_Type.ITEM_EMPTY;

  /*var m_slot = new Object();
  m_slot.type = Item_Type.ITEM_FLOWER_RED;
  m_slot.number = 2;*/

  inventory_number:number[] = [0, 0, 0, 0, 0, 0, 0, 0, 0];

  // todo: add type here
  inventory_type = [this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY,
  this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY, this.Item_Type.ITEM_EMPTY];


  insertInventory(a_type, a_number, a_strength)
  {
    for (var i=0; i<9; i++)
    {
      if (a_type == this.inventory_type[i])
      {
        if ((this.inventory_number[i]+a_number) <= (this.Item_Type.properties[this.inventory_type[i]].weight*(a_strength+3)))
        {
          this.inventory_number[i] += a_number; return true;
        } else { return false; } // do not fill up more than 1 slot with a particular item
      }
    }

    for (i=0; i<9; i++)
    {
      if ((this.Item_Type.ITEM_EMPTY == this.inventory_type[i]) || (this.inventory_number[i] < 1))
      {
        if (a_number <= (this.Item_Type.properties[this.inventory_type[i]].weight*(a_strength+3)))
        {
          this.inventory_type[i] = a_type;
          this.inventory_number[i] = a_number;
          return true;
        }
      }
    }

    return false; // inventory is full
  }

  removeInventory(a_type, a_number)
  {
    for (var i=0; i<9; i++)
    {
      if (a_type == this.inventory_type[i])
      {
        if (this.inventory_number[i] >= a_number)
        {
          this.inventory_number[i] -= a_number; return true;
        }
      }
    }

    return false; // not much enough in inventory
  }

  changeInventory(a_type_old, a_type_new)
  {
    for (var i=0; i<9; i++)
    {
      if (a_type_old == this.inventory_type[i]) { this.inventory_type[i] = a_type_new; }
    }
    return true;
  }

  getInventoryDamage()
  {
    var l_max_damage = 3;
    var l_current_damage = 0;
    for (var i=0; i<9; i++)
    {
      if (this.Item_Type.ITEM_WOOD_BEAM == this.inventory_type[i]) { l_current_damage = 4; }
      else { l_current_damage = 0; }
      if (l_current_damage > l_max_damage) { l_max_damage = l_current_damage; }
    }
    return l_max_damage;
  }

  getInventoryArmor()
  {
    var l_max_armor = 0;
    var l_current_armor = 0;
    for (var i=0; i<9; i++)
    {
      if (this.Item_Type.ITEM_PELT_ARMOR == this.inventory_type[i]) { l_current_armor = 1; }
      else { l_current_armor = 0; }
      if (l_current_armor > l_max_armor) { l_max_armor = l_current_armor; }
    }
    return l_max_armor;
  }

  sellInventory(a_name, a_charisma_bonus_small, a_charisma_bonus_large)
  {
    for (var i=0; i<9; i++)
    {
      if (a_name == this.Item_Type.properties[this.inventory_type[i]].label.toString())
      {
        var display_slot = i+1;
        this.text_status_message += "I sold one " + a_name + " from slot " + display_slot + ". ";
        this.coins += (this.Item_Type.properties[this.inventory_type[i]].price * (1+a_charisma_bonus_large)) + a_charisma_bonus_small;
        this.inventory_number[i] -= 1;
        if (this.inventory_number[i] < 1) { this.inventory_number[i] = 0; this.inventory_type[i] = this.Item_Type.ITEM_EMPTY; }
        return;
      }
      else if (a_name == this.Item_Type.properties[this.inventory_type[i]].label.toString() + "s")
      {
        var display_slot = i+1;
        this.text_status_message += "I sold " + this.inventory_number[i] + " " + a_name + " from slot " + display_slot + ". ";
        this.coins += ((this.Item_Type.properties[this.inventory_type[i]].price * this.inventory_number[i]) * (1+a_charisma_bonus_large)) + a_charisma_bonus_small;
        this.inventory_number[i] = 0; this.inventory_type[i] = this.Item_Type.ITEM_EMPTY;
        return;
      }
    }

    this.text_status_message += "I do not have this item! ";
  }

  useInventory(a_name)
  {
    for (var i=0; i<9; i++)
    {
      if (a_name == this.Item_Type.properties[this.inventory_type[i]].label.toString())
      {
        if (this.inventory_number[i] < 1) { return 0; }
        return this.inventory_type[i];
      }
    }
    return 0;
  }

  throwDice(a_max_value)
  {
    var l_value : number = Math.floor((Math.random() * (a_max_value)) + 1);
    return l_value;
  }

  collecting(a_multiplier, a_check_target, a_item_type, a_stat, a_advancement, a_skill)
  {
    var l_dice = this.throwDice(20) * a_multiplier;
    var l_checks = l_dice - a_skill;
    if (l_checks < 1) l_checks = 1;
    var l_check_target = a_check_target;
    var l_result = true;
    for (var i=0;i<l_checks; i++) { if ((this.throwDice(20) + a_stat) < l_check_target) { l_result = false; } }
    var l_gain = false;
    if ((true == l_result) && (this.throwDice(a_advancement) > a_skill) && (this.throwDice(a_advancement) > a_skill)) { l_gain = true; }

    this.text_status_message += "(" + l_dice + "-" + a_skill + ") x W20 >= " + l_check_target + " - " + a_stat;

    if (false == l_result) { this.text_status_message +=  " failed! "; }
    else if (false == l_gain) { this.text_status_message += " succeeded -> one resource gained! "; }
    else { this.text_status_message += " succeeded -> one resource and one skill level gained! "; }

    if (true == l_result) { if (false == this.insertInventory(a_item_type, 1, this.getStrength())) { this.text_status_message += "I am not strong enough to carry more of this stuff! "; }}
    if (true == l_gain) { return 1; } else { return 0; }
  }

  crafting(a_multiplier, a_check_target, a_result_item_type, a_min_ress_item_number, a_ress_item_type, a_stat, a_advancement, a_skill)
  {
    var l_dice = this.throwDice(20) * a_multiplier;
    var l_checks = l_dice - a_skill;
    if (l_checks < 1) l_checks = 1;
    var l_ressources = l_checks - 1 + a_min_ress_item_number;

    this.text_status_message += "Ressources needed: " + l_ressources;
    if (true == this.removeInventory(a_ress_item_type, l_ressources))
    {

      var l_check_target = a_check_target;
      var l_result = true;
      for (var i=0;i<l_checks; i++) { if ((this.throwDice(20) + a_stat) < l_check_target) { l_result = false; } }
      var l_gain = false;
      if ((true == l_result) && (this.throwDice(a_advancement) > a_skill) && (this.throwDice(a_advancement) > a_skill)) { l_gain = true; }

      this.text_status_message += "(" + l_dice + "-" + a_skill + ") x W20 >= " + l_check_target + " - " + a_stat;

      if (false == l_result) { this.text_status_message +=  " failed! "; }
      else if (false == l_gain) { this.text_status_message += " succeeded -> one product gained! "; }
      else { this.text_status_message += " succeeded -> one product and one skill level gained! "; }

      if (true == l_result) { if (false == this.insertInventory(a_result_item_type, 1, this.getStrength())) { this.text_status_message += "My inventory is full! "; }}
      if (true == l_gain) { return 1; } else { return 0; }

    } else { this.text_status_message += ", i have not enough ressources! "; }
    return 0;
  }

  getStrength() : number
  {
    var l_strength : number = this.strength;
    if (this.hunger < 0) l_strength += this.hunger;
    return l_strength;
  }

  getDexterity() : number
  {
    var l_dexterity : number = this.dexterity;
    if (this.hunger < 0) l_dexterity += this.hunger;
    return l_dexterity;
  }

  getIntelligence() : number
  {
    var l_intelligence : number = this.intelligence;
    if (this.hunger < 0) l_intelligence += this.hunger;
    return l_intelligence;
  }


// ====================================================
//  M A I N
// ====================================================


  ngOnInit() {
    this.first = true; // for authentication
    this.onCommandSubmitted();
  }


  onCommandSubmitted()
  {

    this.requestHTTP();

    this.text_status_message = "";


    this.round++;

    // survival
    if (true == this.thirst) { this.current_health--; this.text_status_message += "I need something to drink! "; }
    else if (1 == this.throwDice(100) && (this.zone > 0)) { this.thirst = true; this.text_status_message += "I need something to drink! "; } // warning 1 round before hp--

    if (this.hunger < 0) { this.text_status_message += "I am hungry and need something to eat! "; }
    else if (1 == this.throwDice(100)) { this.hunger--; }

    if (this.throwDice(600) <= this.zone) { this.changeInventory(this.Item_Type.ITEM_GUTTING_FLESH, this.Item_Type.ITEM_ROTTED_FLESH); }

    // level up intelligence (not supported by old browsers: next_coins = 10 * (3**(level-1));
    this.next_coins = 10; for (var i=1; i<this.level; i++) { this.next_coins *= 3; }
    if (this.coins >= this.next_coins)
    {
      this.level++; this.coins -= this.next_coins; this.give_me_points++; this.max_health += this.throwDice(6);
      this.text_status_message += "I advanced to the next level but lost all my coins! ";
    }

    if (this.give_me_points > 0) { this.text_status_message += "I should give me strength, dexterity or intelligence! "; }

    // weapon damage
    this.current_weapon_damage = this.getInventoryDamage();
    this.current_armor = this.getInventoryArmor();

    // horror event
    if ((this.throwDice(20)+this.getIntelligence() < (this.zone*5)) // two consecutive Will Saves
    &&  (this.throwDice(20)+this.getIntelligence() < (this.zone*5)))
    {
      if (false == this.monster_encounter)
      {
        this.monster_encounter = true;
        this.monster_type = this.throwDice(this.level-1+this.zone); // max monsterlevel is own level + 2 in zone 3
        this.monster_health = this.Monster_Type.properties[this.monster_type].health_number *
          this.throwDice(this.Monster_Type.properties[this.monster_type].health_dice);
      }
    }

    // display monsters and corpses
    this.text_status_message += "I see "; if (true == this.monster_encounter) {this.text_status_message += "a "; }
    this.text_status_message += this.Monster_Type.properties[this.monster_type].label.toString();
    if (true == this.monster_encounter)
    {
     this.text_status_message += " (" + this.Monster_Type.properties[this.monster_type].strength;
     this.text_status_message += "/" + this.Monster_Type.properties[this.monster_type].dexterity;
     this.text_status_message += "/" + this.Monster_Type.properties[this.monster_type].intelligence;
     this.text_status_message += "/" + this.monster_health; this.text_status_message += ")! ";
    }
    else { this.text_status_message += ". "; }
    if (true == this.monster_corpse) { this.text_status_message += "I see a corpse. "; }

    // monster attack
    if (true == this.monster_encounter)
    {
      var l_attack = this.throwDice(20) + this.Monster_Type.properties[this.monster_type].dexterity;
      var l_hit = false; if (l_attack >= 10 + (this.getDexterity() + this.current_armor)) { l_hit = true; }
      var l_damage = this.throwDice(3) + this.Monster_Type.properties[this.monster_type].strength;
      if (l_damage < 0) { l_damage = 0; }
      if (true == l_hit)
      {
        this.current_health -= l_damage;
        this.text_status_message += "The " + this.Monster_Type.properties[this.monster_type].label.toString();
        this.text_status_message += " attacks with " + l_attack + " and causes " + l_damage + " damage! ";
      } else
      {
        this.text_status_message += "The " + this.Monster_Type.properties[this.monster_type].label.toString();
        this.text_status_message += " attacks with " + l_attack + " but failed. ";
      }
    }

    // command processing

    if ((true == this.first) && (this.command_message != "Your Name")) // wait for first real input
    {
      alert("Welcome " + this.command_message + "!");
      this.authentication = this.command_message;
      this.first = false;
    }

    if (this.current_health < 1)
    {
      this.text_status_message += "I am dead and can never continue this game! ";
    }

    else if ("give me strength" == this.command_message)
    {
      if (this.give_me_points > 0)
      { this.strength++; this.give_me_points--; this.text_status_message += "I feel much stronger now! ";
      } else { this.text_status_message += "I have no more points left, must wait for next level! "; }
    }
    else if ("give me dexterity" == this.command_message)
    {
      if (this.give_me_points > 0)
      { this.dexterity++; this.give_me_points--; this.text_status_message += "I feel much more dexterous now! ";
      } else { this.text_status_message += "I have no more points left, must wait for next level! "; }
    }
    else if ("give me intelligence" == this.command_message)
    {
      if (this.give_me_points > 0)
      { this.intelligence++; this.give_me_points--; this.text_status_message += "I feel much more clever now! ";
      } else { this.text_status_message += "I have no more points left, must wait for next level! "; }
    }

    else if ("walk forth" == this.command_message)
    {
      if (this.zone < 3)
      {
        if ((this.throwDice(20)+this.getIntelligence()) >= this.zone*5)
        {
          this.monster_encounter = false; this.monster_type = this.Monster_Type.MONSTER_NONE; this.monster_corpse = false;
          this.zone++; this.text_status_message += "It is much more dangerous here! ";
        } else { this.text_status_message += "I am lost in this woods! "; }
      }
      else { this.text_status_message += "This is already the most dangerous place! "; }
    }
    else if ("walk back" == this.command_message)
    {
      if (this.zone > 0)
      {
        if ((this.throwDice(20)+this.getIntelligence()) >= (10+this.zone*5))
        {
          this.monster_encounter = false; this.monster_type = this.Monster_Type.MONSTER_NONE; this.monster_corpse = false;
          this.zone--; this.text_status_message += "I feel much safer now! ";
        } else { this.text_status_message += "I am lost in this woods! "; }
      }
      else { this.text_status_message += "This is already the safest place! "; }
      if (0 == this.zone) { this.thirst = false; }
    }
    else if ("take some sleep" == this.command_message)
    {
      if (this.zone < 1)
      {
        if (this.coins > 0)
        {
          this.text_status_message += "The time passes, and I regain some power! ";
          this.hunger = this.strength;
          this.current_health++; if (this.current_health > this.max_health) { this.current_health = this.max_health; }
          this.coins--;
        } else { this.text_status_message += "I have no coins to spend for a bed and a meal! "; }
      } else { this.text_status_message += "I cannot sleep here! "; }
    }
    else if ("attack" == this.command_message)
    {
      if (true == this.monster_encounter)
      {
        var l_attack : any = this.throwDice(20) + this.getDexterity();
        var l_hit = false;
        if (l_attack >= 10 + this.Monster_Type.properties[this.monster_type].dexterity) { l_hit = true; }
        var l_damage : any = this.throwDice(this.current_weapon_damage) + this.getStrength();
        if (l_damage < 0) { l_damage = 0; }
        if (true == l_hit)
        {
          this.monster_health -= l_damage;
          if (this.monster_health < 1)
          {
            this.monster_encounter = false;
            this.monster_type = this.Monster_Type.MONSTER_NONE;
            this.monster_corpse = true;
            this.text_status_message += "I attack with " + l_attack + " and cause " + l_damage + " damage -> victory! ";
          } else { this.text_status_message += "I attack with " + l_attack + " and cause " + l_damage + " damage! "; }
        } else { this.text_status_message += "I attack with " + l_attack + " but miss. "; }
      } else { this.text_status_message += "I see no enemy here. "; }
    }
    else if ("pluck flower" == this.command_message)
    {
          if (this.zone < 1) { this.text_status_message += "There are no flowers in this village! "; }
     else if (this.zone < 2) { this.skill_flower_plucking += this.collecting(1, 10, this.Item_Type.ITEM_FLOWER_RED, this.getIntelligence(), 20, this.skill_flower_plucking); }
     else if (this.zone < 3) { this.skill_flower_plucking += this.collecting(2, 15, this.Item_Type.ITEM_FLOWER_BLUE, this.getIntelligence(), 50, this.skill_flower_plucking); }
     else                    { this.skill_flower_plucking += this.collecting(4, 20, this.Item_Type.ITEM_FLOWER_YELLOW, this.getIntelligence(), 90, this.skill_flower_plucking); }
    }
    else if ("lumber wood" == this.command_message)
    {
          if (this.zone < 1) { this.text_status_message += "There are no trees in this village! "; }
     else if (this.zone < 2) { this.skill_wood_lumbering += this.collecting(1, 10, this.Item_Type.ITEM_WOOD_BLUE, this.getStrength(), 20, this.skill_wood_lumbering); }
     else if (this.zone < 3) { this.skill_wood_lumbering += this.collecting(2, 15, this.Item_Type.ITEM_WOOD_YELLOW, this.getStrength(), 50, this.skill_wood_lumbering); }
     else                    { this.skill_wood_lumbering += this.collecting(4, 20, this.Item_Type.ITEM_WOOD_RED, this.getStrength(), 90, this.skill_wood_lumbering); }
    }
    else if ("mine ore" == this.command_message)
    {
          if (this.zone < 2) { this.text_status_message += "I cannot dig here! "; }
     else if (this.zone < 3) { this.skill_ore_mining += this.collecting(1, 10, this.Item_Type.ITEM_MINING_CLAY, this.getStrength(), 20, this.skill_ore_mining); }
     else if (this.zone < 4) { this.skill_ore_mining += this.collecting(2, 15, this.Item_Type.ITEM_MINING_IRON, this.getStrength(), 50, this.skill_ore_mining); }
     else                    { this.skill_ore_mining += this.collecting(4, 20, this.Item_Type.ITEM_MINING_GOLD, this.getStrength(), 90, this.skill_ore_mining); }
    }
    else if ("gut carcass" == this.command_message)
    {
      if (false == this.monster_corpse) { this.text_status_message += "I see no corpse here. "; }
      else
      {
        this.skill_carcass_gutting += this.collecting(1, 5, this.Item_Type.ITEM_GUTTING_FLESH, this.getDexterity(), 20, this.skill_carcass_gutting);
        this.skill_carcass_gutting += this.collecting(2, 10, this.Item_Type.ITEM_GUTTING_FUR, this.getDexterity(), 50, this.skill_carcass_gutting);
        this.monster_corpse = false;
      }
    }

    // crafting: alchemie
    else if ("brew potion" == this.command_message)
    {
      if (this.skill_flower_plucking < 10) { this.text_status_message += "I should pluck more flowers before i can learn brewing potions! "; }
      else
      {
        this.text_status_message += "I can try brewing one of the following potions: ";
        this.text_status_message += ">> brew red potion << needs 2 red flowers ";
        if (this.skill_potion_brewing >= 10) { this.text_status_message += ">> brew blue potion << needs 2 blue flowers "; }
        if (this.skill_potion_brewing >= 20) { this.text_status_message += ">> brew purple potion << needs 1 red and 1 blue potion "; }
        if (this.skill_potion_brewing >= 30) { this.text_status_message += ">> brew yellow potion << needs 2 yellow flowers "; }
        if (this.skill_potion_brewing >= 40) { this.text_status_message += ">> brew orange potion << needs 1 red and 1 yellow potion "; }
        if (this.skill_potion_brewing >= 40) { this.text_status_message += ">> brew green potion << needs 1 blue and 1 yellow potion "; }
        if (this.skill_potion_brewing >= 50) { this.text_status_message += ">> brew white potion << needs 1 red, 1 blue and 1 yellow potion "; }
      }
    }

    else if ("brew red potion" == this.command_message) { this.skill_potion_brewing += this.crafting(1, 5, this.Item_Type.ITEM_POTION_RED, 2, this.Item_Type.ITEM_FLOWER_RED, this.getIntelligence(), 20, this.skill_potion_brewing); }
    else if ("brew blue potion" == this.command_message) { this.skill_potion_brewing += this.crafting(2, 8, this.Item_Type.ITEM_POTION_BLUE, 2, this.Item_Type.ITEM_FLOWER_BLUE, this.getIntelligence(), 30, this.skill_potion_brewing); }
    //else if ("brew purple potion" == this.command_message) { this.skill_potion_brewing += this.crafting(3, 11, this.Item_Type.ITEM_POTION_PURPLE, 2, this.Item_Type.ITEM_FLOWER_BLUE, this.getIntelligence(), 40, this.skill_potion_brewing); }
    else if ("brew yellow potion" == this.command_message) { this.skill_potion_brewing += this.crafting(4, 14, this.Item_Type.ITEM_POTION_YELLOW, 2, this.Item_Type.ITEM_FLOWER_YELLOW, this.getIntelligence(), 50, this.skill_potion_brewing); }


    // crafting: carpenter
    else if ("carpenter" == this.command_message)
    {
      if (this.skill_wood_lumbering < 10) { this.text_status_message += "I should lumber more wood before i can learn carpentering! "; }
      else
      {
        this.text_status_message += "I can try carpentering one of the following items: ";
        this.text_status_message += ">> carpenter wood beam << needs 1 blue wood ";
        if (this.skill_wood_carpentering >= 10) { this.text_status_message += ">> carpenter wood ladder << needs 3 wood beams "; }
      }
    }

    else if ("carpenter wood beam" == this.command_message) { this.skill_wood_carpentering += this.crafting(1, 5, this.Item_Type.ITEM_WOOD_BEAM, 1, this.Item_Type.ITEM_WOOD_BLUE, this.getStrength(), 20, this.skill_wood_carpentering); }
    else if ("carpenter wood ladder" == this.command_message) { this.skill_wood_carpentering += this.crafting(2, 8, this.Item_Type.ITEM_WOOD_LADDER, 3, this.Item_Type.ITEM_WOOD_BEAM, this.getStrength(), 30, this.skill_wood_carpentering); }



    // crafting: leather worker
    else if ("leatherwork" == this.command_message)
    {
      if (this.skill_carcass_gutting < 10) { this.text_status_message += "I should gut more carcasses before i can learn leatherworking! "; }
      else
      {
        this.text_status_message += "I can try leatherworking one of the following items: ";
        this.text_status_message += ">> leatherwork pelt armor << needs 3 furs ";
        //if (this.skill_wood_carpentering >= 10) { this.text_status_message += ">> carpenter wood ladder << needs 3 wood beams "; }
      }
    }

    else if ("leatherwork pelt armor" == this.command_message) { this.skill_leather_working += this.crafting(1, 5, this.Item_Type.ITEM_PELT_ARMOR, 1, this.Item_Type.ITEM_GUTTING_FUR, this.getDexterity(), 20, this.skill_leather_working); }
    //else if ("carpenter wood ladder" == command) { this.skill_wood_carpentering += this.crafting(2, 8, this.Item_Type.ITEM_WOOD_LADDER, 1, this.Item_Type.ITEM_WOOD_BEAM, this.strength, 30, this.skill_wood_carpentering); }





    else
    {
      if (this.command_message.startsWith('sell '))
      {
        if (this.zone < 1)
        {
          this.sellInventory(this.command_message.substring(5), this.charisma_bonus_small, this.charisma_bonus_large);
          this.charisma_bonus_small--; this.charisma_bonus_large--;
          if (this.charisma_bonus_small < 0) { this.charisma_bonus_small = 0; }
          if (this.charisma_bonus_large < 0) { this.charisma_bonus_large = 0; }
        }
        else { this.text_status_message += "I have to find a village!"; }
      }
      else if (this.command_message.startsWith('SAVE,'))
      {
        /*var load_array = this.command_message.split(",");
        // Example: SAVE,Tarok,L,1,S,0,D,0,I,0,G,1,H,10,h,10,c,0,Z,0,S,0,0,0,0,0,0,0,0,INV,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        authentication = load_array[1]; level = Number(load_array[3]); strength = Number(load_array[5]); dexterity = Number(load_array[7]);
        intelligence = Number(load_array[9]); give_me_points = Number(load_array[11]); max_health = Number(load_array[13]);
        current_health = Number(load_array[15]); coins = Number(load_array[17]); zone = Number(load_array[19]);
        if (0 == Number(load_array[21])) { thirst = false; }  else { thirst = true; }; hunger = Number(load_array[23]);
        skill_flower_plucking = Number(load_array[25]); skill_wood_lumbering = Number(load_array[26]); skill_ore_mining = Number(load_array[27]);
        skill_carcass_gutting = Number(load_array[28]); skill_potion_brewing = Number(load_array[29]); skill_wood_carpentering = Number(load_array[30]);
        skill_stone_masoning = Number(load_array[31]); skill_leather_working = Number(load_array[32]);
        for (i=0; i<9; i++) { inventory_number[i] = Number(load_array[34 + 2*i]); inventory_type[i] = Number(load_array[35 + 2*i]); }
        this.text_status_message += "I am now another person! ";*/
      }
      else if (this.command_message.startsWith('use '))
      {
        var l_item_type = this.useInventory(this.command_message.substring(4));
        if (this.Item_Type.ITEM_FLOWER_RED == l_item_type)
        {
          this.current_health += 1;
          if (this.current_health > this.max_health) { this.current_health = this.max_health; }
          this.text_status_message += "I regain some health! ";
          this.removeInventory(l_item_type, 1);
        }
        else if (this.Item_Type.ITEM_FLOWER_BLUE == l_item_type)
        {
          this.charisma_bonus_small = this.throwDice(6);
          this.text_status_message += "I feel more charismatic now! ";
          this.removeInventory(l_item_type, 1);
        }
        else if (this.Item_Type.ITEM_POTION_RED == l_item_type)
        {
          this.current_health += this.throwDice(20); this.thirst = false;
          if (this.current_health > this.max_health) { this.current_health = this.max_health; }
          this.text_status_message += "This is delicious and I regain much health! ";
          this.removeInventory(l_item_type, 1);
        }
        else if (this.Item_Type.ITEM_POTION_BLUE == l_item_type)
        {
          this.charisma_bonus_large = this.throwDice(6);
          this.text_status_message += "I feel more charismatic now! ";
          this.removeInventory(l_item_type, 1);
        }
        else if (this.Item_Type.ITEM_GUTTING_FLESH == l_item_type)
        {
          this.current_health += 1; this.hunger += 1; if (this.hunger > this.strength) { this.hunger = this.strength; }
          if (this.current_health > this.max_health) { this.current_health = this.max_health; }
          this.text_status_message += "I regain some health and reduce my hunger! ";
          this.removeInventory(l_item_type, 1);
        }
        else if (this.Item_Type.ITEM_ROTTED_FLESH == l_item_type)
        {
          this.current_health -= this.throwDice(3);
          this.text_status_message += "Baaah! This is rotten food! ";
          this.removeInventory(l_item_type, 1);
        }
        else if (this.Item_Type.ITEM_FOOD_APPLE == l_item_type)
        {
          this.current_health += 1; this.hunger += 1; if (this.hunger > this.strength) { this.hunger = this.strength; }
          if ((true == this.thirst) && (1 == this.throwDice(3))) { this.thirst = false; }
          if (this.current_health > this.max_health) { this.current_health = this.max_health; }
          this.text_status_message += "This is delicious! ";
          this.removeInventory(l_item_type, 1);
        }
        /*else if (this.Item_Type.ITEM_WOOD_LADDER == l_item_type)
        {
          current_health += 1; hunger += 1; if (hunger > strength) { hunger = strength; }
          if ((true == thirst) && (1 == throwDice(3))) { thirst = false; }
          if (current_health > max_health) { current_health = max_health; }

          text += "I climb onto the ladder ";
          if ((throwDice(20)+getDexterity()) < 5)
          {
            current_health -= throwDice(6); text += "but I fall from the ladder! ";
            if (1 == throwDice(6))
            {
              removeInventory(Item_Type.ITEM_WOOD_LADDER, 1);
              text += "The ladder is destroyed! ";
            }
          } else // success
          {
            if ((throwDice(20)+getIntelligence()) >= 10)
            {
              insertInventory(Item_Type.ITEM_FOOD_APPLE, 1, getStrength());
              text += "and pluck an apple! ";
            } else text += "but nothing happens. ";
          }*/
          else
          {
            this.text_status_message += "I cannot use this! ";
          }
      }
      else
      {
        this.text_status_message += "I do not know this command! ";
      }
    }

    if ("monastery_barracks" == this.zone_name)
    {
      this.text_label_zone = "monastery barracks";
      this.text_zone_description = "...";
    }

    // display zone
    else if (1 == this.zone)
    {
      this.text_label_zone = "woodcutters village";
      this.text_zone_description = "I see some small logs which look like a nice place to spend\
            a night. Grumpy woodcutters linger around and scowl on me.\
            I may take some sleep (but loose coins in dark dreams).\
            Furthermore, I may try to sell some stuff.";
    }

    else if (0 == this.zone)
    {
      this.text_label_zone = "light forest";
      this.text_zone_description = "The sun dances above the light trees, and birds flatter around\
            some bushes. Oaks and beeches rise high, but most of the trees\
            are small and slight, though the wind swishes through their leaves.";
    }

    else if (2 == this.zone)
    {
      this.text_label_zone = "dark forest";
      this.text_zone_description = "The trees are dark and dense. I feel the stiff cold air and \
            water drops like black tears from the dark pines into some\
            green moss. I hear the hammering of a woodpecker, but apart\
            from that the only sound is the moaning of wood.";
    }

    else if (3 == this.zone)
    {
      this.text_label_zone = "cursed forest";
      this.text_zone_description = "The forest is old and dark and i feel remarkably cold. An owl\
            screams somewhere in the murk, and i hear a cruel laugh from\
            behind a demonic yew. I have not seen the sun for hours now,\
            and it feels very dangerous to be here.";
    }

    // display inventory
    var table = document.getElementById("inventory_table");


    // display string for savegame


/*
    text = "SAVE," + authentication; text += ",L," + level + ",S," + strength + ",D," + dexterity + ",I," + intelligence;
    text += ",G," + give_me_points + ",H," + max_health + ",h," + current_health + ",c," + coins + ",Z," + zone;
    text += ",t,"; if (false == thirst) { text += "0"; } else { text += "1" };
    text += ",h," + hunger;
    text += ",S," + skill_flower_plucking + "," + skill_wood_lumbering + "," + skill_ore_mining + "," + skill_carcass_gutting;
    text += "," + skill_potion_brewing + "," + skill_wood_carpentering + "," + skill_stone_masoning + "," + skill_leather_working;
    text += ",INV,"; for (i=0; i<9; i++) { text += inventory_number[i] + "," + inventory_type[i] + ","; }*/

    //document.getElementById("save_status").innerHTML = text;


  }

  /*
  var button = document.getElementById('b');
  button.addEventListener('click', saveGame);

  <button id="b">save game</button>
  */


  // functions for the frontend (do not move to the server!)

  onMoveLeft()
  {

    if (this.own_player_x > 0) { this.own_player_x--; this.onCommandSubmitted(); }

    /*{
      this.matrix[this.own_player_y][this.own_player_x] = this.field_on_player;
      this.own_player_x--;
      this.field_on_player = this.matrix[this.own_player_y][this.own_player_x];
      this.matrix[this.own_player_y][this.own_player_x] = "@";
      this.onCommandSubmitted();
    }*/
  }

  onMoveRight()
  {
    if (this.own_player_x < this.VOLUME_N-1) { this.own_player_x++; this.onCommandSubmitted(); }
    /*{
      this.matrix[this.own_player_y][this.own_player_x] = this.field_on_player;
      this.own_player_x++;
      this.field_on_player = this.matrix[this.own_player_y][this.own_player_x];
      this.matrix[this.own_player_y][this.own_player_x] = "@";
      this.onCommandSubmitted();
    }*/
  }

  onMoveUp()
  {
    if (this.own_player_y > 0) { this.own_player_y--; this.onCommandSubmitted(); }
    /*{
      this.matrix[this.own_player_y][this.own_player_x] = this.field_on_player;
      this.own_player_y--;
      this.field_on_player = this.matrix[this.own_player_y][this.own_player_x];
      this.matrix[this.own_player_y][this.own_player_x] = "@";
      this.onCommandSubmitted();
    }*/
  }

  onMoveDown()
  {
    if (this.own_player_y < this.VOLUME_M-1) { this.own_player_y++; this.onCommandSubmitted();}
    /*{
      this.matrix[this.own_player_y][this.own_player_x] = this.field_on_player;
      this.own_player_y++;
      this.field_on_player = this.matrix[this.own_player_y][this.own_player_x];
      this.matrix[this.own_player_y][this.own_player_x] = "@";
      this.onCommandSubmitted();
    }*/
  }

  createWorld(zone : number)
  {
    this.clean_matrix = [];
    this.final_matrix = [];
    for(var m: number = 0; m < this.VOLUME_M; m++)
    {
      this.clean_matrix[m] = [];
      this.final_matrix[m] = [];
        for(var n: number = 0; n< this.VOLUME_N; n++)
        {

            this.final_matrix[m][n] = "#"; // intial value
            if (zone == 0) // light forest
            {
              var l_value : number = Math.floor(Math.random() * 3);
              this.clean_matrix[m][n] = "ß";
              if (l_value > 0) { this.clean_matrix[m][n] = ","; }
            }

            /*else if (zone == 0)
            {
              var l_value : number = Math.floor(Math.random() * 3);
              this.clean_matrix[m][n] = ":";
              if ((m == 0) || (n == 0)) { this.clean_matrix[m][n] = "#"; }
            }*/
        }
    }
  }

  applyPlayersToMatrix()
  {

    // first, copy clean matrix into result matrix
    for(var m: number = 0; m < this.VOLUME_M; m++)
    {
      for(var n: number = 0; n< this.VOLUME_N; n++)
      {
        this.final_matrix[m][n] = this.clean_matrix[m][n];
      }
    }

    // second, apply players to result matrix

    var j: any;
    for(j in this.players)
    {
      this.final_matrix[this.players[j].position_y][this.players[j].position_x] = this.display_human_other_player;
    }

    this.final_matrix[this.own_player_y][this.own_player_x] = this.display_human_own_player;
  }


// next server to save
// next server -> intrinsic (and periodic save)
// drop -> server inventory
// sell orders -> buying them
// bubbles -> companies -> dungeons
// events -> NPCs -> farming -> cooking
// weather and insanity
// fight skills: stun, bleed, light
// ranged attacks
// coldness -> pelt-armor = max; use fur; use blue wood (fire) -> use wood beam (info)
// new item? blue potion, ...
 //SAVE,Tarok,L,3,S,1,D,2,I,0,G,0,H,20,h,20,c,39,Z,0,t,0,h,1,S,18,17,0,16,7,16,0,1,INV,1,21,1,18,1,27,0,1,5,10,2,25,0,24,0,0,0,0,

}

export class Entity
{
  name: string;
  zone: number;
  position_x: number;
  position_y: number;
  background: string;
};

export class MatrixElement
{
  display: string;
  color: string;
}
