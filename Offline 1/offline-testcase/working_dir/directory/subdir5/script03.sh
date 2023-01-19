#!/bin/bash

recursive_remove_txt() {
    cd "$1"
    for f in *
    do
        if [ -d "$f" ]; then
            recursive_remove_txt "$f"
        else
		    fn="${f%.*}"
		    ext="${f##*.}"
            if [ $ext = "txt" ]; then
                mv "$f" "$fn"
            fi
        fi
    done
    cd "../"
}

recursive_remove_txt $(pwd)
