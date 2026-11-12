# Platform ABI v1

Вводится C ABI между JVM/JNI-адаптерами и платформенным backend. Это не JVM и не запуск Minecraft JAR.

Контракт: lifecycle, снимок HID input, монотонные наносекунды, чтение относительных ресурсов и вывод RGBA8888 кадра 1280x720.

Host backend предназначен для тестов на ПК; файлы читаются из romfs/, дисплей намеренно не эмулируется. Switch backend использует libnx HID, системные тики, framebuffer и чтение файлов по относительному пути.

Проверка host backend: sh runtime/platform/tests/run_host_test.sh.

Switch backend включается в NRO build как отдельный translation unit. Его API пока не вызывается главным игровым циклом прототипа: интеграция с существующим JVM runtime, LWJGL InputBridge и renderer остаётся следующим шагом.

Ограничения: нет аудио, JNI Invocation API, полной LWJGL реализации, OpenGL, потоковой файловой системы и сенсорных событий.
