# OptiCraft JVM — ClassFile foundation v142

Этот этап добавляет bounds-checked parser для Java classfile и не заявляет запуск игры.

## API
- `opticraft_classfile_parse`: разбирает header, constant pool, поля, методы и атрибуты.
- `opticraft_classfile_summary`: возвращает версию, flags и количества.
- `opticraft_classfile_utf8`: получает UTF8 constant.
- `opticraft_classfile_find_method`: ищет метод по имени и descriptor и возвращает access flags/code length.

## Границы
- Не выполняет bytecode.
- Не разрешает символические ссылки constant pool.
- Не загружает зависимости и не инициализирует классы.
- Не подключён в Switch NRO.
- Parser принимает один classfile byte buffer; JAR/ZIP entry extraction остаётся отдельным слоем.

## Проверки
Запуск host smoke test: `sh runtime/jvm/tests/run_classfile_parser_smoke.sh`.

## Следующий этап
Добавить constant-pool resolver и class metadata builder, затем безопасный ClassLoader поверх JAR entry reader. Инициализацию `<clinit>` следует вести отдельным состоянием класса с защитой от повторного выполнения и обнаружением циклических инициализаций.
