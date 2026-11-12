# Отчёт v124 — интеграция Platform/Input/JNI

## Что теперь связано
- libnx HID → семантический OptiInputSnapshot (A/B/X/Y/L/R/D-pad/Plus, стики, touch).
- Главный цикл нативного NRO-прототипа использует opticraft_platform_poll_events().
- Software frame buffer прототипа отправляется в Horizon framebuffer через opticraft_platform_present_rgba8888().
- C++ InputAdapter/SwitchInputBridge преобразуют snapshot в key/mouse transition tuples.
- Java facade и JNI_OnLoad проходят smoke test на Java 21 host JVM.

## CI run 43263651477
- Platform ABI host tests: PASS.
- Platform-to-input bridge tests: PASS.
- Java JNI smoke test: PASS.
- Switch NRO build: PASS.
- Artifact: OptiCraft-World-Prototype-0.1, 85,396 bytes.

## Чего это НЕ означает
- JNI-совместимая VM не встроена в NRO.
- LWJGL Keyboard/Mouse event buffers ещё не сериализуются в точные 18-byte/22-byte структуры.
- Resource resolver оригинального JAR пока не подключён к Platform file API.
- Оригинальный Minecraft/OptiCraft JAR не запускается.
- Физический запуск на Nintendo Switch не проверен.

## Следующий этап v125
1. Реализовать LWJGL 2 Keyboard/Mouse byte-buffer event serialization.
2. Подключить runtime resource/classpath reads к platform filesystem abstraction.
3. Ввести renderer output contract с явным pixel format/stride и тестами.
4. Подготовить JNI adapter boundary для собственного JVM runtime.
