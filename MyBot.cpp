#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include <random>
#include <ctime>
#include <map>

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


        // Game logic
        std::map<shared_ptr<Ship>, Position> all_positions;
        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;


            // Create a mapping of all the
            int max_halite = 0;
            Direction best_move = Direction::STILL;
            for (const auto& dir: ALL_CARDINALS){
                // get the halite at each spot
                Position pos_after = ship->position.directional_offset(dir);
                all_positions[ship] = pos_after;
                int halite_contender = game_map->at(pos_after)->halite;


                // Make sure we're not running into one another.
                // Since we iterate all the options, then make sure there are no collisions
                // THEN assign, we are sure that the value at the very end is as close to optimal
                for (auto it=all_positions.begin(); it!=all_positions.end(); ++it){
                    if ((it->first != ship) && (it->second != pos_after)){
                        if (halite_contender >= max_halite){
                            max_halite = halite_contender;
                            best_move = dir;
                        }
                    }
                }


            }
            command_queue.push_back(ship->move(best_move));
        }

        if (
            game.turn_number <= 200 &&
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
