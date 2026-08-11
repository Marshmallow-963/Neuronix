magick -size 128x128 xc:none -fill black -draw "circle 64,64 64,1" \
-fill white -pointsize 40 -gravity center -draw "text 0,0 'CH'" \
iz_rs_neuron.png

for file in *.png; do
    ffmpeg -i "$file" -frames:v 1 -update 1 "${file%.png}.qoi"
done
