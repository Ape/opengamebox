#!/bin/sh

convert $1 card_mask.png -compose CopyOpacity -composite .card_temp.png
composite -compose atop card_border.png .card_temp.png $1
rm .card_temp.png
