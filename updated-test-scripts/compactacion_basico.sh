#!/bin/sh
./game-boy BROKER CAUGHT_POKEMON 1 OK
./game-boy BROKER CAUGHT_POKEMON 2 FAIL

./game-boy BROKER CATCH_POKEMON Pikachu 2 3
./game-boy BROKER CATCH_POKEMON Squirtle 5 2

./game-boy BROKER CATCH_POKEMON Onyx 4 5

./game-boy SUBSCRIBE CAUGHT_QUEUE 10

./game-boy BROKER CATCH_POKEMON Vaporeon 4 5
