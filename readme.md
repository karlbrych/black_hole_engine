# 🕳️ OpenGL - Black Hole Engine

Grafický engine vytvořený v OpenGL pro vizualizaci a zjednodušenou simulaci černých děr a dalších extrémních vesmírných objektů v reálném čase.

Projekt vzniká jako školní práce se zaměřením na 3D grafiku, práci s GPU a shaderové programování.

## 🚀 Funkce

- Real-time vykreslování 3D scény
- Volný pohyb kamerou v prostoru
- Zobrazení černé díry se základními vizuálními efekty

- Simulace:
  - gravitačního čočkování (vizuální deformace okolí) akrečního disku

- Shaderové efekty běžící na GPU
- Modulární struktura enginu (kamera, scéna, objekty)

## 🛠 Použité technologie

|Technologie|Účel|
|-------------|--------|
|C++|Hlavní programovací jazyk|
|OpenGL|Vykreslování grafiky|
|GLFW|Okno aplikace, vstupy, OpenGL kontext|
|GLAD|Načítání funkcí OpenGL|
|GLM|Matematika (vektory, matice, transformace)|
|GLSL|Shadery (výpočty a vizuální efekty na GPU)|
|STB|Načítání textur|
|CMake|Build systém projektu|

## 📦 Kompilace projektu

### Požadavky

- C++17 kompatibilní kompilátor
- CMake 3.10+
- GPU podporující OpenGL 3.3+

### Postup (Arch Linux & Depedencies)

```.
sudo pacman -Syu
sudo pacman -S base-devel cmake glfw-x11 glm mesa
git clone git@github:karlbrych/black_hole_engine.git
mkdir build
cd build
cmake ..
make
./Blackhole
```

### Postup (Windows & Depedencies)

```.
git clone git@github:karlbrych/black_hole_engine.git
mkdir build
cd build
cmake ..
cmake --build ..
.\Blackhole.exe
```

Spustitelný soubor (.exe) se vytvoří ve složce build.

## 🎮 Ovládání

|Klávesa / Myš|Funkce|
|----------|--------------|
|W, A, S, D|Pohyb kamery|
|Myš|Otáčení pohledu|
|ESC|Zavření aplikace|

## 🧠 Co se projekt snaží ukázat

Tenhle engine není vědecký simulátor. Jde o vizuálně přesvědčivou grafickou aproximaci jevů ve vesmíru. Cílem je spojit:

- počítačovou grafiku
- práci s maticemi a 3D prostorem
- shaderové programování
- optimalizaci výkonu

## 🔮 Možná budoucí rozšíření

- Přesnější fyzikální model gravitačního pole
- Ray-marching nebo ray-tracing efekty
- Více typů vesmírných objektů (černé díry, hvězdy, mlhoviny, planety)
- Post-processing efekty (bloom, HDR, motion blur)
- GUI pro úpravu parametrů simulace

## 👨‍💻 Autor

Školní projekt

- Libor Šenar
- Karel Brych
- Martin Hrubeš
- Jan Votroubek

<!--
📷 Ukázky

(Sem můžeš přidat screenshoty nebo GIFy aplikace)
-->