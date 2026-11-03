#!/usr/bin/env python3
"""Render Rebble App Store screenshots at exact platform resolutions.

Mirrors Koi Pond Zenith (no watch bezel — store requirement).
"""
from __future__ import annotations

import math
import os
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "store" / "screenshots"
POND = ROOT / "resources" / "images" / "pond" / "pond.png"
KOI = ROOT / "resources" / "images" / "koi"

PLATFORMS = {
    "basalt": (144, 168, True),
    "diorite": (144, 168, False),
    "flint": (144, 168, False),
    "emery": (200, 228, True),
}

DIRS = ["n", "ne", "e", "se", "s", "sw", "w", "nw"]
DARK_GREEN = (0, 85, 0)
ISLAMIC_GREEN = (0, 170, 0)
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)

# Up to 5 store shots — varied times / Clay options.
SCENES = [
    {
        "file": "01-morning.png",
        "hour": 10,
        "minute": 8,
        "second": 22,
        "show_digital": True,
        "show_date": False,
        "show_seconds": True,
        "ring_inset": 18,
    },
    {
        "file": "02-afternoon-date.png",
        "hour": 15,
        "minute": 0,
        "second": 40,
        "show_digital": True,
        "show_date": True,
        "show_seconds": True,
        "ring_inset": 18,
        "date": "Sun 2 Nov",
    },
    {
        "file": "03-evening.png",
        "hour": 18,
        "minute": 45,
        "second": 10,
        "show_digital": True,
        "show_date": False,
        "show_seconds": True,
        "ring_inset": 14,
    },
    {
        "file": "04-noon-clean.png",
        "hour": 12,
        "minute": 30,
        "second": 0,
        "show_digital": False,
        "show_date": False,
        "show_seconds": False,
        "ring_inset": 18,
    },
    {
        "file": "05-night.png",
        "hour": 21,
        "minute": 15,
        "second": 55,
        "show_digital": True,
        "show_date": False,
        "show_seconds": True,
        "ring_inset": 22,
    },
]


def font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    if bold:
        candidates = [
            "C:/Windows/Fonts/segoeuib.ttf",
            "C:/Windows/Fonts/arialbd.ttf",
            "C:/Windows/Fonts/consolab.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        ]
    else:
        candidates = [
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        ]
    for path in candidates:
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def draw_text_center(draw: ImageDraw.ImageDraw, text: str, y: int, w: int, fnt, fill) -> None:
    bbox = draw.textbbox((0, 0), text, font=fnt)
    tw = bbox[2] - bbox[0]
    draw.text(((w - tw) // 2, y), text, font=fnt, fill=fill)


def to_bw(src: Image.Image) -> Image.Image:
    rgba = src.convert("RGBA")
    out = Image.new("RGBA", rgba.size, (0, 0, 0, 0))
    px = rgba.load()
    opx = out.load()
    w, h = rgba.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                continue
            lum = 0.299 * r + 0.587 * g + 0.114 * b
            # Keep near-black outlines; lift midtones so orange koi stays visible.
            if lum < 40:
                opx[x, y] = (0, 0, 0, 255)
            elif lum < 140:
                opx[x, y] = (170, 170, 170, 255)
            else:
                opx[x, y] = (255, 255, 255, 255)
    return out


def clock_point(cx: int, cy: int, radius: int, angle_deg: int) -> tuple[int, int]:
    rad = math.radians(angle_deg)
    x = cx + int(round(math.sin(rad) * radius))
    y = cy - int(round(math.cos(rad) * radius))
    return x, y


def dir_from_angle(angle_deg: int) -> str:
    heading = (angle_deg + 90) % 360
    bucket = ((heading + 22) // 45) % 8
    return DIRS[bucket]


def load_koi(color_name: str, angle_deg: int, color: bool, scale: float) -> Image.Image:
    name = f"{color_name}_{dir_from_angle(angle_deg)}.png"
    img = Image.open(KOI / name).convert("RGBA")
    if not color:
        img = to_bw(img)
    if scale != 1.0:
        nw = max(1, int(round(img.size[0] * scale)))
        nh = max(1, int(round(img.size[1] * scale)))
        img = img.resize((nw, nh), Image.NEAREST)
    return img


def paste_centered(base: Image.Image, sprite: Image.Image, center: tuple[int, int]) -> None:
    x = center[0] - sprite.size[0] // 2
    y = center[1] - sprite.size[1] // 2
    base.paste(sprite, (x, y), sprite)


def format_time(hour: int, minute: int) -> str:
    h = hour % 12
    if h == 0:
        h = 12
    return f"{h:02d}:{minute:02d}"


def render(w: int, h: int, color: bool, scene: dict) -> Image.Image:
    bg = DARK_GREEN if color else BLACK
    img = Image.new("RGB", (w, h), bg)

    pond = Image.open(POND).convert("RGBA")
    if not color:
        pond = to_bw(pond).convert("RGBA")
        flat = Image.new("RGBA", pond.size, (0, 0, 0, 255))
        flat.paste(pond, (0, 0), pond)
        pond = flat

    # Match watchface: stretch pond to fill the whole display.
    pond = pond.resize((w, h), Image.NEAREST)
    img.paste(pond.convert("RGB"), (0, 0))

    cx, cy = w // 2, h // 2
    half = min(w, h) // 2
    water_r = (half * 58) // 72
    radius = water_r - (int(scene["ring_inset"]) - 18)
    radius = max(20, min(radius, water_r))
    koi_scale = min(w / 144.0, h / 168.0)

    hour_angle = ((scene["hour"] % 12) * 30) + (scene["minute"] // 2)
    minute_angle = scene["minute"] * 6
    second_angle = scene["second"] * 6

    draw = ImageDraw.Draw(img)
    if scene["show_seconds"]:
        sx, sy = clock_point(cx, cy, radius, second_angle)
        r = max(2, int(round(3 * koi_scale)))
        fill = ISLAMIC_GREEN if color else WHITE
        draw.ellipse((sx - r, sy - r, sx + r, sy + r), fill=fill, outline=BLACK)

    white = load_koi("white", hour_angle, color, koi_scale)
    orange = load_koi("orange", minute_angle, color, koi_scale)
    paste_centered(img, white, clock_point(cx, cy, radius, hour_angle))
    paste_centered(img, orange, clock_point(cx, cy, radius, minute_angle))

    fg = WHITE
    if scene["show_digital"]:
        time_size = 22 if w >= 200 else 18
        draw_text_center(
            draw,
            format_time(scene["hour"], scene["minute"]),
            cy - (20 if w >= 200 else 16),
            w,
            font(time_size, bold=True),
            fg,
        )
    if scene["show_date"]:
        date_size = 14 if w >= 200 else 12
        draw_text_center(
            draw,
            scene.get("date", "Sun 2 Nov"),
            cy + (8 if w >= 200 else 6),
            w,
            font(date_size),
            fg,
        )

    return img


def main() -> None:
    if OUT.exists():
        shutil.rmtree(OUT)
    for plat, (w, h, color) in PLATFORMS.items():
        dest = OUT / plat
        dest.mkdir(parents=True, exist_ok=True)
        for scene in SCENES:
            shot = render(w, h, color, scene)
            path = dest / scene["file"]
            shot.save(path, "PNG", optimize=True)
            print(f"wrote {path.relative_to(ROOT)} ({w}x{h})")
    print("Done:", OUT)


if __name__ == "__main__":
    main()
