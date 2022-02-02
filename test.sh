#!/bin/bash
function test() {
    to_grep=${@: -1} # extract last argument (bash only)
    set -- "${@:1:$(($#-1))}" # remove it from the list (bash only)
    echo -n "$* "
    $* | grep "${to_grep}" >/dev/null && echo "ok" && return
    echo "ERROR"
}

echo "Simple cuts tests"
test ./numberMatchSolver --one-cut . "No cut found"
test ./numberMatchSolver --one-cut 1 "No cut found"
test ./numberMatchSolver --one-cut .1 "No cut found"
test ./numberMatchSolver --one-cut 1. "No cut found"
test ./numberMatchSolver --one-cut 12 "No cut found"
test ./numberMatchSolver --one-cut .. "No cut found"
test ./numberMatchSolver --one-cut 11 "A cut was found at 1 and 2"
test ./numberMatchSolver --one-cut .11 "A cut was found at 2 and 3"
test ./numberMatchSolver --one-cut .1..1 "A cut was found at 2 and 5"
test ./numberMatchSolver --one-cut 12 "No cut found"
test ./numberMatchSolver --one-cut .12 "No cut found"
test ./numberMatchSolver --one-cut .1..2 "No cut found"
test ./numberMatchSolver --one-cut ........11........ "A cut was found at 9 and 10"
test ./numberMatchSolver --one-cut ........1....2...1 "A cut was found at 9 and 18"
test ./numberMatchSolver --one-cut .......1.2.......1 "A cut was found at 8 and 18"
test ./numberMatchSolver --one-cut ........1......31. "A cut was found at 9 and 17"
test ./numberMatchSolver --one-cut 1.......2........1 "No cut found"
test ./numberMatchSolver --one-cut 1.......2........1.......1 "A cut was found at 18 and 26"
test ./numberMatchSolver --one-cut 1.......2.......1........1 "A cut was found at 17 and 26"
test ./numberMatchSolver --one-cut 1.......2......11......... "A cut was found at 16 and 17"
test ./numberMatchSolver --one-cut 1.......2......1.......1.. "A cut was found at 16 and 24"
test ./numberMatchSolver --one-cut ........19........ "A cut was found at 9 and 10" 
test ./numberMatchSolver --one-cut ........9....2...1 "A cut was found at 9 and 18" 
test ./numberMatchSolver --one-cut .......1.2.......1 "A cut was found at 8 and 18" 
test ./numberMatchSolver --one-cut ........4......36. "A cut was found at 9 and 17" 
test ./numberMatchSolver --one-cut 1.......2........1 "No cut found"
test ./numberMatchSolver --one-cut 1.......2......37.........1 "A cut was found at 16 and 17"

echo -e "\nPreventing wrap in diagonal cut search"
test ./numberMatchSolver --line-length 4 --one-cut .8...9.8 "No cut found"
test ./numberMatchSolver --line-length 4 --one-cut .8.......9...7..8... "No cut found"
test ./numberMatchSolver --one-cut ........1.....5..6..51....763 "No cut found"

echo -e "\nSome full games"
test ./numberMatchSolver --line-length 5 ..2..131....13....1..2... "Found 1 winning game"
test ./numberMatchSolver 928971895545435472689218659 "Found 1 winning game"
test ./numberMatchSolver 781924818262157235753161412 "Found 1 winning game"

echo -e "\nOption diagonal"
test ./numberMatchSolver -d n 1...23....1....... "No winning cut sequences found"
test ./numberMatchSolver -d y 1...23....1....... "Found 1 winning game"
