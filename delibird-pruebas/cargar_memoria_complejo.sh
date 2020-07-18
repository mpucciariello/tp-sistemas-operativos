#!/bin/sh
./game-boy BROKER CATCH_POKEMON Pikachu 9 3 #19b
./game-boy BROKER CATCH_POKEMON Squirtle 9 3 #20b

./game-boy BROKER CAUGHT_POKEMON 10 OK
./game-boy BROKER CAUGHT_POKEMON 11 FAIL

./game-boy BROKER CATCH_POKEMON Bulbasaur 1 7 #21
./game-boy BROKER CATCH_POKEMON Charmander 1 7 #22

./game-boy BROKER GET_POKEMON Pichu #9
./game-boy BROKER GET_POKEMON Raichu #10