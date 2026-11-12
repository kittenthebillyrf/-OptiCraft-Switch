# Milestone v123 — Platform ABI v1

Состав: общий C ABI с версией, host backend, libnx backend, host regression test и документация границы интеграции.

Host test проверяет ABI version, lifecycle, monotonic clock, input snapshot, запрет traversal path и честное отсутствие host display. Switch CI build проверит компиляцию libnx backend.

Эти проверки не доказывают запуск Minecraft или подключение JNI к работающей JVM.

Дальнейшая интеграция: согласовать SwitchInputBridge с OptiInputSnapshot; перевести classpath/resource resolver на platform file API; передавать кадры renderer в framebuffer; затем подключать оригинальный bootstrap/JAR.
