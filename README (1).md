# Pinky Lite 🌸  v0.2

Бело-розовый VST3 для FL Studio. Made by **seoulbby**.

Volume-шейпер в стиле ShaperBox / Infiltrator, фильтр с LFO, шесть типов искажения,
ring mod, bitcrush и автопан. Всё синхронизировано с темпом проекта.

## Что внутри

| Секция | Что делает |
|--------|-----------|
| **Volume** | LFO-огибка громкости (Sine / Triangle / Ramp / Pulse / Bounce), Rate от 1/1 до 1/32 по темпу FL |
| **Filter** | Low-pass / High-pass / Band-pass, Cutoff + Reso, LFO Mod качает срез той же формой |
| **Distort** | 6 типов: Soft, Hard, Fold, Tube, Fuzz, Rectify. Drive, Bias (асимметрия), Tone, LFO Mod (drive пульсирует с LFO) |
| **Crush** | Понижение битности и частоты дискретизации |
| **Ring** | Ring mod: Amount и Freq |
| **Pan** | Автопан той же формой LFO |
| **Output** | Mix (dry/wet) и Gain |

## Настройки (кнопка Settings справа вверху)

- **Oversampling** Off / 2x / 4x: чище искажение, небольшая задержка (FL компенсирует её сам)
- **Safe limiter**: мягко ограничивает выход
- **Theme**: Pinky / Bubblegum / Peach
- **Animations**: бегунок на графике LFO
- **Reset sound**: сброс всех ручек к значениям по умолчанию

Двойной клик по ручке сбрасывает её.

## Сборка через GitHub (без установки Visual Studio)

1. Загрузи в репозиторий файлы проекта (новые заменят старые одноимённые).
2. Файл сборки лежит в `.github/workflows/build.yml`.
3. После коммита во вкладке **Actions** сам запустится запуск «Build Pinky Lite».
4. Когда появится зелёная галочка, скачай артефакт **Pinky-Lite-VST3** со страницы запуска.
5. Скопируй `Pinky Lite.vst3` в `C:\Program Files\Common Files\VST3\` (FL Studio перед этим закрой).
6. FL Studio: **Options → Manage plugins → Find plugins**.

## Сборка локально (Windows)

```
cmake -B build
cmake --build build --config Release
```

Готовый плагин: `build\PinkyLite_artefacts\Release\VST3\Pinky Lite.vst3`

## Идеи на будущее

- Рисуемая мышью кривая огибающей, как в ShaperBox
- Отдельные LFO для фильтра, дисторшна и панорамы
- Пресеты
- Pitch-модуль
