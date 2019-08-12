#!/usr/bin/env bash

BOT_NAME=${1:-bot_$(date +%s)}

set -e

if (cmake . && make); then
    mv MyBot ./$BOT_NAME
    mv ./$BOT_NAME ../manager/bots
else
    echo "Build failed"
fi
