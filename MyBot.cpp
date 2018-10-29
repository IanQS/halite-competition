#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"
#include "Eigen/Dense"

#include <random>
#include <ctime>

using namespace std;
using namespace hlt;

// Game Dynamics:
// https://halite.io/learn-programming-challenge/game-overview

unsigned int get_seed(int argc, char* argv[]){
    unsigned int rng_seed;
    if (argc > 1) {
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } else {
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }

    return  rng_seed;
}

int main(int argc, char* argv[]) {
    // CONSTANTS

    const int JOURNEY_BACK_CONST = 450;
    const int JOURNEY_OUT_CONST = 250;
    const int TURN_NUM_CHECK = 200;
    const int RETURN_HYPERPARAM = 15;


    unsigned  int rng_seed = get_seed(argc, argv);
    mt19937 rng(rng_seed);

    Game game;
    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    const std::array<int, 2> MAP_LIMITS = game.game_map->min_turn();
    const int LAST_CHANCE = MAP_LIMITS[1] - RETURN_HYPERPARAM;
    const int MAP_SIZE = MAP_LIMITS[0];
    game.ready("MyCppBot");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");


    for (;;) {
        // journey_back_threshold encourages them to move further away as time goes on
        // build depot then
        int journey_back_threshold = JOURNEY_BACK_CONST + game.turn_number;
        int journey_out_threshold = max(game.turn_number - JOURNEY_OUT_CONST, 0);

        game.update_frame();
        shared_ptr<Player> me = game.me;

        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;
        std::unordered_map<shared_ptr<Ship>, bool> ship_status;  // default initialized False

        // Depot logic
        // this way, we can remove a key-value from the ships_in_play (if converted to depot or smth)
        std::unordered_map<EntityId, std::shared_ptr<Ship>> ships_in_play = me->ships;


        for (const auto& dropoff: me->dropoffs){
            log::log("Depots: {}" + dropoff.second->position.to_string());
        }

        // Game logic
        for (const auto& ship_iterator : ships_in_play) {
            shared_ptr<Ship> ship = ship_iterator.second;

            // Force all the ships to go back
            if (game.turn_number >= LAST_CHANCE){
                Direction go_back;
                if (game_map->calculate_distance(ship->position, me->shipyard->position) == 1){
                     go_back = game_map->naive_navigate(ship, me->shipyard->position, false);
                } else{
                    go_back = game_map->naive_navigate(ship, me->shipyard->position);
                }
                command_queue.push_back(ship->move(go_back));
                ship_status[ship] = true;
                continue;
            }
            if (ship_status[ship] || ship->has_enough(journey_back_threshold)){
                if (ship->halite >= journey_out_threshold){
                    Direction go_back = game_map->naive_navigate(ship, me->shipyard->position);
                    command_queue.push_back(ship->move(go_back));
                    ship_status[ship] = true;
                    continue;
                } else {
                    ship_status[ship] = false;
                    // go on to gather below
                }
            }

            Direction greedy_safe = game_map->greedy_safe_move(ship);
            command_queue.push_back(ship->move(greedy_safe));
        }






        // Construct new ships
        if (
            game.turn_number <= TURN_NUM_CHECK &&
            me->halite >= constants::SHIP_COST &&
            !game_map->at(me->shipyard)->is_occupied())
        {
            command_queue.push_back(me->shipyard->spawn());

        }

        // End logic
        if (!game.end_turn(command_queue)) {
            break;
        }
    }

    return 0;
}
