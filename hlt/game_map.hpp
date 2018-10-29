#pragma once

#include "types.hpp"
#include "map_cell.hpp"

#include <vector>

namespace hlt {
    struct GameMap {
        int width;
        int height;
        std::vector<std::vector<MapCell>> cells;

        MapCell* at(const Position& position) {
            Position normalized = normalize(position);
            return &cells[normalized.y][normalized.x];
        }

        MapCell* at(const Entity& entity) {
            return at(entity.position);
        }

        MapCell* at(const Entity* entity) {
            return at(entity->position);
        }

        MapCell* at(const std::shared_ptr<Entity>& entity) {
            return at(entity->position);
        }

        const std::array<int, 2> min_turn(){
            const int map_size = this->width;
            if (map_size == 32){
                return {32, 401};
            } else if (map_size == 40){
                return {40, 426};
            } else if (map_size == 48){
                return {48, 451};
            } else if (map_size == 56){
                return {56, 476};
            } else {
                return {64, 501};
            }
        }

        int calculate_distance(const Position& source, const Position& target) {
            const auto& normalized_source = normalize(source);
            const auto& normalized_target = normalize(target);

            const int dx = std::abs(normalized_source.x - normalized_target.x);
            const int dy = std::abs(normalized_source.y - normalized_target.y);

            const int toroidal_dx = std::min(dx, width - dx);
            const int toroidal_dy = std::min(dy, height - dy);

            return toroidal_dx + toroidal_dy;
        }

        Position normalize(const Position& position) {
            const int x = ((position.x % width) + width) % width;
            const int y = ((position.y % height) + height) % height;
            return { x, y };
        }

        std::vector<Direction> get_unsafe_moves(const Position& source, const Position& destination) {
            const auto& normalized_source = normalize(source);
            const auto& normalized_destination = normalize(destination);

            const int dx = std::abs(normalized_source.x - normalized_destination.x);
            const int dy = std::abs(normalized_source.y - normalized_destination.y);
            const int wrapped_dx = width - dx;
            const int wrapped_dy = height - dy;

            std::vector<Direction> possible_moves;

            if (normalized_source.x < normalized_destination.x) {
                possible_moves.push_back(dx > wrapped_dx ? Direction::WEST : Direction::EAST);
            } else if (normalized_source.x > normalized_destination.x) {
                possible_moves.push_back(dx < wrapped_dx ? Direction::WEST : Direction::EAST);
            }

            if (normalized_source.y < normalized_destination.y) {
                possible_moves.push_back(dy > wrapped_dy ? Direction::NORTH : Direction::SOUTH);
            } else if (normalized_source.y > normalized_destination.y) {
                possible_moves.push_back(dy < wrapped_dy ? Direction::NORTH : Direction::SOUTH);
            }

            return possible_moves;
        }

        Direction naive_navigate(std::shared_ptr<Ship> ship, const Position& destination) {
            // get_unsafe_moves normalizes for us
            for (auto direction : get_unsafe_moves(ship->position, destination)) {
                Position target_pos = ship->position.directional_offset(direction);
                if (!at(target_pos)->is_occupied()) {
                    at(target_pos)->mark_unsafe(ship);
                    return direction;
                }
            }

            return Direction::STILL;
        }

        Direction greedy_safe_move(const std::shared_ptr<Ship>& ship, const int scale = 1,
                const int min_halite = 250){
            /* Heuristically, it's better to drain the shit out of a cell first, then move away
             * as this reduces the cost (10% of available resources in origin cell)
             *
             */
            // First, check that it's not worth staying in the current cell:
            // Gain: 25% of halite available in cell
            if (this->at(ship->position)->halite >= min_halite){
                return Direction::STILL;
            }

            int max_halite = 0;
            Direction best_safe_move = Direction::STILL;
            // If not worth, move
            // Cost: 10% of halite available at turn origin cell is deducted from ship’s current halite
            for (const auto& dir: ALL_CARDINALS){
                // get the halite at each spot
                Position pos_after = ship->position.directional_offset(dir, scale);
                int halite_contender = this->at(pos_after)->halite;


                // Track the best move and make it safe
                if (halite_contender >= max_halite){
                    max_halite = halite_contender;
                    best_safe_move = this->naive_navigate(ship, pos_after);
                }
            }
            return best_safe_move;
        }

        void _update();
        static std::unique_ptr<GameMap> _generate();
    };
}
