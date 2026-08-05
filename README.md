## Juego en el tren

Play the game from a browser: [Play the game in your browser →](https://ponchoalv.github.io/juegoeneltren/)

### I'm Using no build

#### For building the firt time run:

```
cc buildgame.c -o buildgame && ./buildgame
```


#### Once `buildgame` was build you can rebuild with

```
./buildgame
```


#### If you want to run it, simply run:


```
./buildgame -run
```

#### If you want to build the webassembly version

```
./buildgame -web
```

####  once, with Emscripten activated

```
./buildgame -web
```

# serve the generated game

```
cd build/web && python3 -m http.server 8080
```

#### deps

If option `-no-fetch` provided to `./buildgame` you will need to download / clone the deps under `/deps` folder... for now is only raylib, the link is present in the folder readme.
Otherwise raylib will be clone from my fork `git@github.com:ponchoalv/raylib.git.`
