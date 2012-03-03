#!/bin/sh

echo "Creating $1"
convert $1 -resize 180x252 card_mask.png -compose CopyOpacity -composite .card_temp.png
composite -compose atop card_border.png .card_temp.png $1
convert -quality 0 +dither -colors 256 $1 $1 
rm .card_temp.png
