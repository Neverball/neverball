# Neverball alpha release checklist

```sh
TAG="1.7.0-alpha.3"
BASENAME="neverball-${TAG}"
```

## Publish a tag

```sh
cd neverball
git status
git tag -a "$TAG" -m "Neverball "$TAG""
git push origin tag "$TAG"
```

## Build a Windows 32-bit build

This uses Docker.

```sh
# Compile data on host
make -j8 sols locales desktops
# Compile executables in a Docker container
docker run --rm --volume $(pwd):/neverball --workdir /neverball parasti/neverball-mingw mingw32-make -j8 -o sols -o locales -o desktops PLATFORM=mingw
# Copy DLLs from Docker to current directory
docker run --rm --volume $(pwd):/neverball --workdir /neverball parasti/neverball-mingw mingw-list-deps --copy neverball.exe neverputt.exe mapc.exe
```

## Make a ZIP

```sh
# Symlink to force a prefix on archive filenames
ln -s . ./"$BASENAME"
# Make the ZIP, manage infinite recursion due to the symlink
zip -r ./"$BASENAME".zip ./"$BASENAME" -x '*/'"$BASENAME"'/*' '*/.vscode/*' '*/.git/*' '*.o' '*.d' '.DS_Store'
# Remove the symlink
rm "$BASENAME"
```

## Write release notes

Just do it.

## Create a release on Github

https://github.com/Neverball/neverball/releases/new

Attach the ZIP file. Set as pre-release because alpha.