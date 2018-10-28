#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include <random>
#include <ctime>

using namespace std;
using namespace hlt;

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
    unsigned  int rng_seed = get_seed(argc, argv);
    mt19937 rng(rng_seed);

    Game game;
    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("MyCppBot");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");

    for (;;) {
        game.update_frame();
        shared_ptr<Player> me = game.me;

        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;
        std::unordered_map<shared_ptr<Ship>, bool> ship_status;  // default initialized False

        // Game logic


        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;


            if ((ship_status[ship]) || (ship->has_enough(500))){
                if (ship->halite != 0){
                    Direction go_back = game_map->naive_navigate(ship, me->shipyard->position);
                    command_queue.push_back(ship->move(go_back));
                    ship_status[ship] = true;
                    continue;
                } else {
                    ship_status[ship] = false;
                    // go on to gather below
                }
            }

            int max_halite = 0;
            Direction best_safe_move = Direction::STILL;

            for (const auto& dir: ALL_CARDINALS){
                // get the halite at each spot
                Position pos_after = ship->position.directional_offset(dir);
                int halite_contender = game_map->at(pos_after)->halite;


                // Track the best move and make it safe
                if (halite_contender >= max_halite){
                    max_halite = halite_contender;
                    best_safe_move = game_map->naive_navigate(ship, pos_after);
                }
            }

            command_queue.push_back(ship->move(best_safe_move));
        }

        if (
            game.turn_number <= 200 &&
            me->halite >= constants::SHIP_COST &&
            !game_map->at(me->shipyard)->is_occupied())
        {
            if (me->ships.size() == 0){
                command_queue.push_back(me->shipyard->spawn());
            }

        }


        // End logic
        if (!game.end_turn(command_queue)) {
            break;
        }
    }

    return 0;
}
