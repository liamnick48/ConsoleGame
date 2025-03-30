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
//stats at the top of the screen (total gold, health, enemy kills) PARTIALLY DONE
//keybind to quit, to restart and to save the game PARTIALLY DONE
//WoW-esque levelling with increased max HP and damage each level PARTIALLY DONE
//enemy types (and a way to actually differentiate them besides letters)
//combat encounter screen
//vendor NPC, currency system
//maybe randomize player start position?
//obstacles and environment details (dwarf fortress style)
//map border and doors leading to new maps (wishful thinking)
//save/load state
//stat page that the player can open at any time (and maybe classes?)
//maybe WoW-style durability system for weapons and armor?

class ConsoleGame {
private:
    static const int rows = 20;         // customizable map size, probably abandon later?
    static const int columns = 20;
    char map[rows][columns];
    double experience;
    int total_level;
    int food_collected;                 // placeholder
    int monsters_defeated;
    int player_row;
    int player_col;

    // storing coordinates of enemies and food, was added to fix previous bugs
    std::vector<std::pair<int, int>> enemies;
    std::vector<std::pair<int, int>> food;

    // reworked message system added to avoid popups above the map, but rather below it
    std::string message_log;

    bool can_move(int target_row, int target_col);
    void level_up();
    void defeat_monster();
    void eat_food();

public:
    double health;
    void introduction();
    void initialize();
    void add_message(const std::string& message);
    void generate(unsigned int seed);
    void draw();
    void move(char action);
    bool process();

    std::string get_experience_bar();
};

// dynamic exp bar added to show how close the player is to the next level, static one removed for future rework
std::string ConsoleGame::get_experience_bar() {
    int bar_length = 40;
    int filled_length = static_cast<int>((experience / 100.0) * bar_length);
    std::string bar(filled_length, char(177));
    bar.append(bar_length - filled_length, '.');
    return "[" + bar + "] " + std::to_string(static_cast<int>(experience)) + "/100";
}

void ConsoleGame::introduction() {
    system("Color 70");  // temporary color change (reason: looks cool)
    std::cout << "Welcome to [PLACEHOLDER]!" << std::endl;
    std::cout << "Your goal is to defeat enemies and collect food to survive." << std::endl;
    std::cout << "CONTROLS: 'W', 'A', 'S', 'D' to move, 'Q' to quit the game, 'R' to restart." << std::endl;
    std::cout << "Good luck, and have fun!" << std::endl << std::endl;
}

// initializing game variables, food likely to be replaced with gold (which will be spendable at a vendor NPC)
void ConsoleGame::initialize() {
    health = 100.0;
    total_level = 1;
    experience = 0.0;
    food_collected = 0;
    monsters_defeated = 0;
    player_row = 0;
    player_col = 0;

    // generates empty spaces across the map, marked as "."
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            map[i][j] = '.';
        }
    }
    map[player_row][player_col] = 'P';  // "P" is just a placeholder letter for the player
}

void ConsoleGame::add_message(const std::string& message) {
    message_log += message + "\n ";
}

void ConsoleGame::generate(unsigned int seed) {
    srand(seed);
    enemies.clear();
    food.clear();

    // randomizing entities, with "X" indicating enemies and "F" indicating food (placeholders)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            int randVal = rand() % 20;
            if (randVal == 0) {
                map[i][j] = 'X';
                enemies.push_back({ i, j });
            }
            else if (randVal == 1) {
                map[i][j] = 'F';
                food.push_back({ i, j });
            }
        }
    }
}

// drawing the current map, only one for now
void ConsoleGame::draw() {

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Health: " << health << "  |  Level: " << total_level << "  |  Enemies Defeated: " << monsters_defeated << std::endl;
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

// basic WASD input movements
void ConsoleGame::move(char action) {
    int target_row = player_row;
    int target_col = player_col;

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
        map[player_row][player_col] = '.';
        player_row = target_row;
        player_col = target_col;
        map[player_row][player_col] = 'P';
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

void ConsoleGame::defeat_monster() {
    health -= 10;
    experience += 10;
    monsters_defeated++;
    level_up();
    add_message("\nMonster defeated! Health: " + std::to_string(health));
}

// all works now :D
void ConsoleGame::eat_food() {
    health += 10;
    food_collected++;
    add_message("\nFood eaten! Health: " + std::to_string(health));
}

// determining whether the player ate food or defeated an enemy, added necessary erase function
bool ConsoleGame::process() {
    for (auto& enemy : enemies) {
        if (enemy.first == player_row && enemy.second == player_col) {
            defeat_monster();
            enemies.erase(std::remove(enemies.begin(), enemies.end(), enemy), enemies.end());
            break;
        }
    }

    for (auto& food_item : food) {
        if (food_item.first == player_row && food_item.second == player_col) {
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

            system("cls");

            game.move(action);
            game.process();

            if (game.health <= 0) {
                system("cls");
                std::cout << "\nYou have died. Your deeds of heroism will be remembered." << std::endl;
                break;
            }

            game.draw();
        }
    }

    return 0;
}