;; ((nil . ((compile-command . "clang -std=c99 -Wall -Wextra -Wpedantic -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL libraylib.a main.c -o build/juego_en_el_tren && ./build/juego_en_el_tren"))))

;; for generating tags I recommend using ctags (universal c-tags) with this option
;; ctags -e -R --c-kinds=+p --c++-kinds=+p .
