# Pinky Lite 🌸

Бело-розовый VST3 для FL Studio: volume-шейпер (как в ShaperBox / Infiltrator),
фильтр с LFO, drive, bitcrush и автопан — всё синхронизировано с темпом проекта.

## Что внутри

| Секция | Что делает |
|--------|-----------|
| **Volume** | LFO-огибка громкости (Sine / Triangle / Ramp / Pulse / Bounce), Rate от 1/1 до 1/32 по темпу FL |
| **Filter** | Low-pass, Cutoff + Reso, LFO Mod качает срез той же формой |
| **Drive** | Мягкое насыщение (tanh) |
| **Crush** | Понижение битности и частоты дискретизации |
| **Pan** | Автопан той же формой LFO |
| **Output** | Mix (dry/wet) и Gain |

Двойной клик по ручке — сброс к значению по умолчанию.

## Сборка (Windows)

1. Поставь **Visual Studio 2022** (компонент «Разработка классических приложений на C++»), **CMake** и **Git**.
2. В папке проекта выполни:

```
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

JUCE скачается автоматически (нужен интернет при первой сборке).

3. Готовый плагин будет здесь:
   `build\PinkyLite_artefacts\Release\VST3\Pinky Lite.vst3`
   Скопируй его в `C:\Program Files\Common Files\VST3\`.
4. В FL Studio: **Options → Manage plugins → Find plugins** (или Start scan), затем найди **Pinky Lite** в списке эффектов и вставь в слот микшера.

Также собирается Standalone-версия — удобно для быстрой проверки интерфейса.

## Дальше можно добавить

- Редактируемую кривую огибающей (рисовать точки мышью, как в ShaperBox)
- Отдельные модули для фильтра/панорамы со своими LFO
- Пресеты
- Анти-алиасинг (oversampling) для drive
