# TODOs

# MyBot.cpp

- refactor and remove redundancies in code

# Implementation

## Simple

- [X] The agents don't need to be greedy - currently, they move between all cardinal directions to the max, but moving costs halite

- [X] increase the amount required before they head back
    - long journeys require halite to reach the base 

- [X] increase the threshold before they flip
    - by the time they get back the amt is very low

## Pre-planning

### Startup

- [ ] Pre-compute the entire map for halite, then cluster to determine best regions to go (partition the map)


### Return traversal

- [ ] compute graph of all visited points for min cost to return home

- [ ] If not cost efficient at all, and is close to many others, convert into depot