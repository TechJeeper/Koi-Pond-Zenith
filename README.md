# Koi Pond Zenith (Pebble)

A zen top-down koi pond watchface. Two PixelLab koi swim the pond rim like clock hands:

- **White koi** → hour
- **Orange koi** → minute
- Optional **lily** on the ring → seconds

## Settings (phone)

Config uses **Clay** (`@rebble/clay`). Open **Koi Pond Zenith → Settings** in the Pebble/Rebble app:

- Digital time / hour format / leading zero / date
- Seconds lily on/off
- Swim clockwise or counter-clockwise
- Swim ring inset (how far from the screen edge)

## Build

On Windows (Podman or Docker):

```powershell
./build.ps1
```

Output: `build/KoiPondZenith.pbw`

```powershell
./build.ps1 -Phone 192.168.1.23
./build.ps1 -Zip
```

## Appstore

Listing copy and screenshot notes: [`STORE.md`](STORE.md).

```powershell
python tools/generate_store_screenshots.py
```

Output: `store/screenshots/{basalt,diorite,flint,emery}/` at exact Rebble sizes.

## Layout

```
src/c/              watchface + koi clock math
src/pkjs/           Clay settings
resources/images/   PixelLab pond + 8-direction koi sprites
store/screenshots/  Rebble Appstore device shots
```
