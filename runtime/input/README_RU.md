# v124 — Platform → Input → JNI

Импортирован input adapter и SwitchInputBridge из инженерной реализации v122. Platform ABI теперь определяет семантические биты A/B/X/Y/L/R/D-pad/Plus, а Switch backend переводит в них HID-кнопки libnx. Сырые значения HID наружу больше не передаются.

opticraft_input_poll() забирает OptiInputSnapshot из платформы и преобразует его в события key down/up, mouse down/up/motion. opticraft_input_update_snapshot() используется тестами и может быть вызван runtime adapter. Java JNI facade предоставляет init, shutdown и poll.

Host regression test проверяет переходы состояний; JNI smoke test проверяет регистрацию native методов и вызовы на обычной JVM ПК.

Ограничение: это ещё не полная реализация LWJGL Keyboard/Mouse event buffers. Не сериализованы точные 18-byte keyboard и 22-byte mouse records. На Switch этот JNI facade пока требует реальной JNI-совместимой VM; стандартный HotSpot в NRO не встроен.
