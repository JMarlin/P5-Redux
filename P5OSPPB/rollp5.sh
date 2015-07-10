#!/bin/bash

#check to see if the user entered a commit message and, if so,
#do a commit to git right before building (just to have better
#build documentation)
if (( $# > 0 )) ; then
    git add --all :/
    git commit -m "$1"
    git push
fi

cd mods
./buildm.sh
cd ..
./build.sh
./boot.sh
