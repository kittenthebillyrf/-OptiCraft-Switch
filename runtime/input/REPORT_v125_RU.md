# Отчёт v125 — LWJGL 2 Input ABI

## Реализовано
- Keyboard event record: 18 байт — key (int32), pressed (byte), character (int32), timestamp (int64), repeat (byte).
- Mouse event record: 22 байта — button (byte), pressed (byte), x (int32), y (int32), wheel (int32), timestamp (int64).
- Многобайтовые поля сериализуются BIG_ENDIAN, как их читает Java ByteBuffer в LWJGL 2.
- Для движения мыши используется button = -1; для отсутствия колёсного события wheel = 0.
- Добавлены polling arrays: 256 байт состояния клавиш и 8 байт состояния кнопок мыши.
- C API получает snapshot и выдаёт event buffers + poll state + позицию курсора.
- JNI facade получил pollLwjgl(width, height), возвращающий четыре byte[]: keyboard events, mouse events, keyboard state, mouse state.
- Новый serializer автоматически входит в Switch NRO через runtime/input/src.
- Добавлены unit и integration tests: сериализация записей, переходы клавиш/кнопок, timestamp, mouse motion, C API и JNI facade.

## Локальная проверка
- LWJGL event ABI test: PASS.
- Platform/Input C API integration test: PASS.
- Java 21 JNI smoke test с pollLwjgl(): PASS.
- Компиляция host C++ тестов с -Wall -Wextra -Werror -pedantic: PASS.

## Проверка Switch
GitHub Actions workflow обновлён для ветки runtime/lwjgl-abi-v125. Результат NRO-сборки в этом обновлении ещё не подтверждён; физический запуск на Nintendo Switch не проверен.

## Ограничения
- Это совместимый слой формата событий и poll-state, а не полноценная реализация всего LWJGL.
- JNI-совместимая VM не встроена в NRO.
- Полный ввод текста/раскладка символов пока не реализованы; controller key events передают CHAR_NONE и repeat=false.
- Оригинальный OptiCraft/Minecraft JAR ещё не запускается.
