# OptiCraft Switch — runtime gate v92

## Цель этапа

Перейти от работающего libnx diagnostic NRO к реальному запуску оригинального Minecraft Java 1.2.5 + OptiFine HD U C7. Этот этап фиксирует проверяемое состояние OpenJDK 8 Switch source tree и запрещает считать наличие исходников JVM доказательством наличия готовой JVM.

## Подтверждённая база на консоли

Пользователь проверил NRO на Nintendo Switch:
- NRO стартует из hbmenu;
- экран libnx console выводится;
- главный цикл продолжает работать (счётчик кадров растёт);
- нажатие A обнаруживается;
- Java VM и Minecraft в диагностическом приложении явно не подключены.

Это подтверждает только работоспособность минимальной libnx оболочки, не JVM.

## Проверка OpenJDK Switch source bundle v25

Проверен архив `OpenJDK_Switch_Merged_Source_v25.zip`:
- 47 916 записей в архиве;
- 444 285 125 байт распакованного содержимого;
- найден HotSpot AArch64 и BSD/Linux OS layers;
- отсутствует каталог/проект `switch-java-runner`;
- в исходниках нет реализации вызовов `svcManageCodeMemory`;
- в исходниках нет реализации интеграции `__libnx_exception_handler`;
- эти два механизма упоминаются в README, но не обнаружены в исходном коде;
- `do_make.sh` всё ещё компилирует конфигурацию `linux-aarch64-normal-server-release` и содержит `|| true`, маскируя ошибки;
- скрипт затем пытается скопировать `libjvm.a` в отсутствующий `switch-java-runner`;
- `build_jdk.sh` содержит жёстко заданный Windows путь автора;
- найденные OS layers используют POSIX/BSD/Linux интерфейсы; наличие AArch64 CPU backend само по себе не создаёт Horizon OS port.

## Важное техническое следствие

Нельзя безопасно подключить этот архив к NRO простой линковкой `libjvm.a`: готовой `libjvm.a` для Horizon здесь не подтверждено, а необходимые Horizon реализации управления памятью, потоками, сигналами/исключениями, временем, файлами и динамическим кодом не реализованы. Подмена этих вызовов пустыми заглушками даст ложный «успешный» линк, но не работающую JVM.

## Следующая последовательность интеграции

### Gate A — честный HotSpot platform port
1. Выделить отдельную платформу Horizon/libnx в HotSpot build system, не объявлять Linux/BSD backend эквивалентом Horizon.
2. Реализовать Horizon OS primitives: virtual memory reserve/commit/release, thread lifecycle/TLS, mutex/condition variables, clocks/timers, file and dynamic library operations, signal/exception mapping.
3. Отдельно реализовать executable code memory lifecycle для JIT с реальными Horizon syscalls и cache maintenance; пока не подтверждено на консоли — использовать interpreter-only конфигурацию как первый JVM bring-up target.
4. Убрать все masked build failures и требовать проверяемые build outputs.

### Gate B — JNI Invocation smoke test
1. Собрать HotSpot и class libraries под один согласованный target ABI.
2. Включить настоящий JNI Invocation API entry point.
3. Запустить минимальный Java class, который печатает маркер и завершается.
4. Подтвердить запуск этого smoke test на Switch.

### Gate C — OptiCraft bootstrap
1. Установить runtime image и class libraries в SD data root.
2. Реализовать чтение JAR и classpath с SDMC.
3. Собрать LWJGL 2.9.4 switchport adapter и закрыть обязательные JNI symbols.
4. Запустить оригинальный `net.minecraft.client.Minecraft` с исходными JAR.
5. Только затем интегрировать GL compatibility renderer, input, audio и меню.

## Критерии завершения v92

- Источники JVM проаудированы на фактическую реализацию Horizon API, а не по README.
- Блокирующие отсутствующие компоненты перечислены.
- Порядок работ разделён на измеримые gate-критерии.
- Никакой фиктивной `libjvm.a` или пустой JNI callback не выдаётся за JVM.

## Статус

v92 — аудит и техническое разделение этапов runtime bring-up. Реальная JVM ещё не собрана. Полноценный Minecraft на Switch ещё не запускается.
