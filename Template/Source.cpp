#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <sstream>
#include <iomanip>
#include <conio.h>
#include <windows.h>

//implement wishlist:
//stats at the top of the screen (total gold, health, enemy kills)
//keybind to quit, to restart and to save the game
//WoW-esque levelling with increased max HP and damage each level
//enemy types (and a way to actually differentiate them besides letters)
//combat tweaks: exp needs to be greater for more difficult enemies, healing potentially only with potions in inventory, scaling damage based on stats
//vendor NPC, currency system
//maybe randomize player start position?
//obstacles and environment details (dwarf fortress style)
//map border and doors leading to new maps
//save/load state
//stat page that the player can open at any time, the ability to use levels to level up stats
//maybe WoW-style durability system for weapons and armor?

class Player {
public:
    double health;
    int player_row;
    int player_col;

    // RPG stats, for the purpose of leveling
    int agility;
    int strength;
    int stamina;
    int intellect;
    int spirit;

    Player() {
        health = 100.0;
        player_row = 0;
        player_col = 0;

        agility = 1;
        strength = 1;
        stamina = 1;
        intellect = 1;
        spirit = 1;
    }

    void set_position(int row, int col) {
        player_row = row;
        player_col = col;
    }

    void cap_health() {
        if (health > 100.0) {
            health = 100.0;
        }
    }
};

class Enemy {
public:
    std::string name;
    double health;
    double damage;
    int row, col;

    // constructor created to initialize the enemy name/hp/dmg, allows me to create as many as I want
    Enemy(std::string enemy_name, double enemy_health, double enemy_damage, int enemy_row, int enemy_col)
        : name(enemy_name), health(enemy_health), damage(enemy_damage), row(enemy_row), col(enemy_col) {}

    void take_damage(double dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    bool is_alive() const {
        return health > 0;
    }
};

class ConsoleGame {
private:
    static const int rows = 20;         // customizable map size, probably abandon later?
    static const int columns = 20;
    char map[rows][columns];
    double experience;
    int total_level;
    int food_collected;                 // placeholder
    int monsters_defeated;

    Player player;

    // storing coordinates of enemies and food, was added to fix previous bugs
    std::vector<Enemy> enemies;
    std::vector<std::pair<int, int>> food;

    // reworked message system added to avoid popups above the map, but rather below it
    std::string message_log;

    bool can_move(int target_row, int target_col);
    void level_up();
    void defeat_monster();
    void eat_food();

public:
    // getters to call the player's health and position in later functions
    double get_player_health() const { return player.health; }
    int get_player_row() const { return player.player_row; }
    int get_player_col() const { return player.player_col; }

    void introduction();
    void initialize();
    void add_message(const std::string& message);
    void generate(unsigned int seed);
    void draw();
    void move(char action);
    bool process();
    void show_stats();
    void combat(Enemy& enemy);

    std::string get_experience_bar();
};

void ConsoleGame::introduction() {
    system("Color 70");  // temporary color change (reason: looks cool)
    std::cout << "Welcome to Warcraft!" << std::endl;
    std::cout << "Your goal is to defeat enemies and collect food to survive." << std::endl;
    std::cout << "CONTROLS: 'W', 'A', 'S', 'D' to move, 'C' to open stats, 'Q' to quit the game, 'R' to restart." << std::endl;
    std::cout << "Good luck, and have fun!" << std::endl << std::endl;
}

// dynamic exp bar added to show how close the player is to the next level, static one removed for future rework
std::string ConsoleGame::get_experience_bar() {
    int bar_length = 40;
    int filled_length = static_cast<int>((experience / 100.0) * bar_length);
    std::string bar(filled_length, char(177));
    bar.append(bar_length - filled_length, '.');
    return "[" + bar + "] " + std::to_string(static_cast<int>(experience)) + "/100";
}

void ConsoleGame::show_stats() {
    system("cls");
    std::cout << "------ Player Stats ------" << std::endl;
    std::cout << "Level: " << total_level << std::endl;
    std::cout << "Agility: " << player.agility << std::endl;
    std::cout << "Strength: " << player.strength << std::endl;
    std::cout << "Stamina: " << player.stamina << std::endl;
    std::cout << "Intellect: " << player.intellect << std::endl;
    std::cout << "Spirit: " << player.spirit << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "\nPress any key to go back to the game..." << std::endl;
    _getch();
    system("cls");
}

// initializing game variables, food likely to be replaced with gold (which will be spendable at a vendor NPC)
void ConsoleGame::initialize() {
    player = Player();
    total_level = 1;
    experience = 0.0;
    food_collected = 0;
    monsters_defeated = 0;

    // generates empty spaces across the map, marked as "."
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            map[i][j] = '.';
        }
    }
    map[player.player_row][player.player_col] = 'P';  // "P" is just a placeholder letter for the player
}

void ConsoleGame::add_message(const std::string& message) {
    message_log += message + "\n ";
}

void ConsoleGame::generate(unsigned int seed) {
    srand(seed);
    enemies.clear();
    food.clear();

    // door to access new part of the map, Stardew Valley style
    bool door_placed = false;

    // randomizing entities, both mobs and food (placeholders)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            int randVal = rand() % 20;

            if (randVal == 0 and !door_placed and map[i][j] == '.') {    // ensuring that only one door is placed
                map[i][j] = 'D';
                door_placed = true;
            }

            else if (randVal == 0) {
                int enemy_type = rand() % 3;    // 0, 1 or 2 in order to choose enemy type
                Enemy new_enemy("Orc", 30.0, 5.0, i, j);     // default (most common) enemy

                if (enemy_type == 1) {
                    new_enemy = Enemy("Undead", 50.0, 10.0, i, j);
                }
                else if (enemy_type == 2) {
                    new_enemy = Enemy("Tauren", 80.0, 15.0, i, j);
                }

                enemies.push_back(new_enemy);
                map[i][j] = 'X';

            }
            else if (randVal == 1) {
                    map[i][j] = 'F';
                    food.push_back({ i, j });
            }
        }
    }
}

// drawing the main page
void ConsoleGame::draw() {

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Health: " << player.health << "  |  Level: " << total_level << "  |  Enemies Defeated: " << monsters_defeated << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << get_experience_bar() << std::endl << std::endl;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            std::cout << map[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << message_log;
    message_log.clear();
}

// basic WASD input movements, collision
void ConsoleGame::move(char action) {
    int target_row = player.player_row;
    int target_col = player.player_col;

    switch (action) {
    case 'w': target_row--; break;
    case 's': target_row++; break;
    case 'a': target_col--; break;
    case 'd': target_col++; break;
    case 'q': break;
    default:
        add_message("\nInvalid action!\n");
        return;
    }

    if (can_move(target_row, target_col)) {
        map[player.player_row][player.player_col] = '.';  // clear previous position
        player.player_row = target_row;
        player.player_col = target_col;

        if (map[player.player_row][player.player_col] == 'D') {
            add_message("\nWalking through the door, you discover a new part of the map!\n");

            // clear the map and respawn mobs, keeping the player's stats
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < columns; ++j) {
                    map[i][j] = '.';
                }
            }

            // generate a new map but keep stats, level, and counters
            generate(static_cast<unsigned int>(time(0)));
            map[player.player_row][player.player_col] = 'P';
            return;
        }

        map[player.player_row][player.player_col] = 'P';  // place player at new position
    }

    else {
        add_message("\nCan't move there!\n");
    }
}

// checker to see if player can move in the desired direction
bool ConsoleGame::can_move(int target_row, int target_col) {
    if (target_row < 0 or target_row >= rows or target_col < 0 or target_col >= columns) {
        return false;
    }
    // ultimate deluxe placeholder for potential future obstacles (trees, stones, buildings?)
    return true;
}

void ConsoleGame::level_up() {
    if (experience >= 100) {
        experience = 0;
        total_level++;
        add_message("\nCongratulations, you have reached level " + std::to_string(total_level) + "!");
    }
}

void ConsoleGame::combat(Enemy& enemy) {
    char action;
    while (true) {
        system("cls");
        std::cout << "You are fighting a " << enemy.name << "!\n" << std::endl;
        std::cout << "Enemy Health: " << enemy.health << std::endl;
        std::cout << "Your Health: " << player.health << std::endl;
        std::cout << "\nChoose an action:" << std::endl;
        std::cout << "1. Attack (Deal 15 damage)" << std::endl;
        std::cout << "2. Heal (Heal 25 health)" << std::endl;
        std::cout << "3. Flee (Return to the map)" << std::endl;

        action = _getch();

        if (action == '1') {
            enemy.take_damage(15);
            add_message("You attack the " + enemy.name + " for 15 damage!");

            // check if the enemy is dead
            if (!enemy.is_alive()) {
                system("cls");
                add_message("\nThe " + enemy.name + " has been defeated!");
                monsters_defeated++;
                experience += 10;
                return;
            }
        }
        else if (action == '2') {
            player.health += 25;
            if (player.health > 100) {
                player.health = 100;      // capping health at 100
            }
            add_message("You heal yourself for 25 health!");
        }
        else if (action == '3') {
            system("cls");
            add_message("\nYou flee from the battle!");
            return;
        }
        else {
            add_message("Invalid choice. Please choose 1, 2, or 3.");
        }

        // the enemy attacks the player after the player's action
        if (enemy.is_alive()) {
            player.health -= enemy.damage;
            add_message("The " + enemy.name + " attacks you for " + std::to_string(enemy.damage) + " damage.");
        }

        // check if the player is dead
        if (player.health <= 0) {
            system("cls");
            std::cout << "You have been defeated by the " << enemy.name << ". Game Over!" << std::endl;
            break;
        }

        // display the current state again after each turn
        std::cout << "\n" << message_log << std::endl;
        message_log.clear();
        std::cout << "\nPress any key to continue..." << std::endl;
        _getch();
    }
}

// reworking this in favor of the combat screen
void ConsoleGame::defeat_monster() {
    player.health -= 10;
    experience += 10;
    monsters_defeated++;
    level_up();
    add_message("\nMonster defeated! Health: " + std::to_string(player.health));
}

// all works now :D
void ConsoleGame::eat_food() {
    player.health += 10;
    player.cap_health();    // health cap at 100
    food_collected++;
    add_message("\nFood eaten! Health: " + std::to_string(player.health));
}

// determining whether the player ate food or defeated an enemy, added necessary erase function
bool ConsoleGame::process() {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->row == player.player_row and it->col == player.player_col) {
            // trigger combat encounter when the player steps on an enemy
            combat(*it);
            it = enemies.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto& food_item : food) {
        if (food_item.first == player.player_row and food_item.second == player.player_col) {
            eat_food();
            food.erase(std::remove(food.begin(), food.end(), food_item), food.end());
            break;
        }
    }

    return true;

}

int main() {
    ConsoleGame game;

    auto start_game = [&]() {       // lambda function added for the restart game logic
        game.introduction();

        std::cout << "\nPress SPACE to start game..." << std::endl;

        char action;
        while (true) {
            if (_kbhit()) {
                action = _getch();

                if (action == ' ') {
                    break;
                }
            }
        }

        system("cls");

        game.initialize();
        game.generate(static_cast<unsigned int>(time(0)));
        game.draw();

        };

    start_game();

    char action;
    while (true) {
        if (_kbhit()) {             // removed cin >> action in favor of _kbhit for more fluent movement
            action = _getch();

            if (action == 'r') {
                system("cls");
                std::cout << "\nRestarted!\n" << std::endl;
                game.initialize();
                game.generate(static_cast<unsigned int>(time(0)));
                game.draw();
                continue;
            }
            if (action == 'q') {
                system("cls");
                std::cout << "\nYou have quit the game. Thanks for playing!" << std::endl;
                break;
            }
            if (action == 'c') {
                game.show_stats();
                game.draw();
                continue;
            }

            system("cls");

            game.move(action);
            game.process();

            if (game.get_player_health() <= 0) {
                system("cls");
                std::cout << "\nYou have died. Your deeds of heroism will be remembered." << std::endl;
                break;
            }

            game.draw();
        }
    }

    return 0;
}