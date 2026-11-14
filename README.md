# OptiCraft Switch — v147

Порт оригинального OptiCraft JAVA LITE 1.2.5 + OptiFine HD U C7.

## Сборка NRO

GitHub Actions собирает текущий Switch platform diagnostic entrypoint из исходников v147:
- libnx framebuffer initialization/presentation;
- чтение кнопок, стиков и тачскрина;
- выход по кнопке Plus.

Файл результата: **OptiCraft-v147-NRO-diagnostic** в Actions → Build OptiCraft v147 NRO → Artifacts.

> Важно: это первый NRO-пакет на базе реального кода платформенного слоя проекта. Он пока НЕ запускает JVM и оригинальный Minecraft-клиент. Главный экран Minecraft подтверждён на host-side probe; перенос полного JVM/рендерера в libnx остаётся следующим этапом. Не считать этот диагностический NRO готовым портом игры.

## Host validation

```bash
bash tests/run_jvm_object_interpreter_test.sh
bash tests/run_gl_compat_core_test.sh
```
