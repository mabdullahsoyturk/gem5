#!/bin/bash

inputImg=$1
outputImg=$2

convert -depth 8 -size 512x512 GRAY:$inputImg $outputImg
