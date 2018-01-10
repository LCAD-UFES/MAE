#!/bin/sh

: <<'END'
END

declare -a dirs=(
"/dados/ufes/20160825"
"/dados/ufes/20160825-01"
"/dados/ufes/20160825-02"
"/dados/ufes/20160830"
"/dados/ufes/20161021"
"/dados/ufes/20170119"
"/dados/ufes/20171122"
"/dados/ufes/20171205"
)

for i in "${!dirs[@]}"; do
    echo "Saving at ${dirs[$i]}"

    for f in ${dirs[$i]}/*l.png; do
        if ! [ -f "${f%.png}.ppm" ]; then
            echo "Processing ${f%.png}.ppm"
            convert -compress none "$f" "${f%.png}.ppm"
        fi
    done
done
