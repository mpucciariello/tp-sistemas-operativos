#!/bin/sh
./game-boy BROKER CAUGHT_POKEMON 1 OK
./game-boy BROKER CAUGHT_POKEMON 2 FAIL

./game-boy BROKER NEW_POKEMON Pikachu 2 3 1

./game-boy BROKER CATCH_POKEMON Onyx 4 5

./game-boy SUBSCRIBE NEW_QUEUE 10

./game-boy BROKER CATCH_POKEMON Charmander 4 5
