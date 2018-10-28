# TODOs

# MyBot.cpp

- refactor and remove redundancies in code

# Implementation

## Jittering

- [X] The agents don't need to be greedy - currently, they move between all cardinal directions to the max, but moving costs halite

## over time 

- seems like most matches last about 500 rounds

- [X] increase the amount required before they head back
    - long journeys require halite to reach the base 

- [X] increase the threshold before they flip
    - by the time they get back the amt is very low
    - [ ] FUTURE: convert into depots by calculating the distance
    
3) Pre-compute the entire map for halite, then cluster to determine best regions to go (partition the map)

4) 