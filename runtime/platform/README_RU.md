# Platform ABI v1

Вводится C ABI между JVM/JNI-адаптерами и платформенным backend. Это не JVM и не запуск Minecraft JAR.

Контракт: lifecycle, снимок HID input, монотонные наносекунды, чтение относительных ресурсов и вывод RGBA8888 кадра 1280x720.

Host backend предназначен для тестов на ПК; файлы читаются из romfs/, дисплей намеренно не эмулируется. Switch backend использует libnx HID, системные тики, framebuffer и чтение файлов по относительному пути.

Проверка host backend: sh runtime/platform/tests/run_host_test.sh.

Switch backend включён в NRO build и уже вызывается главным циклом нативного прототипа: platform poll supplies normalized input, and RGBA8888 present sends the software-rendered frame to the Horizon framebuffer. C++ LWJGL-like input bridge is also compiled into the NRO. Java JNI registration is built/tested separately on host because the NRO still has no JNI VM headers/runtime.

Ограничения: нет аудио, JNI Invocation API, полной LWJGL реализации, OpenGL, потоковой файловой системы и сенсорных событий.
