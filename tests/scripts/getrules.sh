#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host"
else
  (echo '{"id":1, "method":"Rules.GetRules"}'; sleep 1) | nc $1 2222
fi
