# Отчёт v125 — LWJGL 2 Input ABI

## Реализовано
- Keyboard event record: 18 байт — key (int32), pressed (byte), character (int32), timestamp (int64), repeat (byte).
- Mouse event record: 22 байта — button (byte), pressed (byte), x (int32), y (int32), wheel (int32), timestamp (int64).
- Многобайтовые поля сериализуются BIG_ENDIAN, как их читает Java ByteBuffer в LWJGL 2.
- Для движения мыши используется button = -1; для отсутствия колёсного события wheel = 0.
- Добавлены polling arrays: 256 байт состояния клавиш и 8 байт состояния кнопок мыши.
- C API получает snapshot и выдаёт event buffers + poll state + позицию курсора.
- JNI facade получил pollLwjgl(width, height), возвращающий четыре byte[]: keyboard events, mouse events, keyboard state, mouse state.
- Добавлены host regression tests для размеров, порядка полей, timestamp, key/button state и mouse motion.
- Switch NRO автоматически компилирует новый serializer как часть runtime/input/src; JNI entrypoint по-прежнему отдельно собирается для JVM smoke test.

## Проверки
CI запускается на ветке runtime/lwjgl-abi-v125. Итоговые статусы будут внесены после завершения GitHub Actions.

## Ограничения
- Это совместимый слой формата событий и poll-state, а не полноценная реализация всего LWJGL.
- Нет реальной JNI-совместимой VM внутри NRO.
- Полный ввод текста/раскладка символов пока не реализованы; controller key events передают CHAR_NONE и repeat=false.
- Оригинальный OptiCraft/Minecraft JAR ещё не запускается.
- Физический запуск на Nintendo Switch не подтверждён.
