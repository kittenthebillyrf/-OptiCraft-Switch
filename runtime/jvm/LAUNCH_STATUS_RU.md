# Запуск OptiCraft Java 1.2.5 — статус v141

Добавлен host-инструмент `opticraft_launch_preflight`. Он открывает настоящий `minecraft.jar`, находит `net/minecraft/client/Minecraft.class`, извлекает запись ZIP (Store/DEFLATE), проверяет CRC32, разбирает constant pool и таблицу методов, проверяет Java 5 classfile (major 49) и наличие `main([Ljava/lang/String;)V` с атрибутом Code.

Запуск проверки на ПК:

```sh
sh runtime/jvm/tests/run_launch_preflight.sh /path/to/minecraft.jar
```

**Важно:** preflight не исполняет Java bytecode и не запускает игру. Это проверяемый первый этап цепочки загрузки. Следующий блок — связать JAR class loader с JVM interpreter, выполнить статические вызовы и затем реализовать объекты/поля/виртуальные вызовы на общей модели runtime. Для реального Minecraft также нужны AWT/Applet compatibility, LWJGL/OpenGL mapping, Java standard library и Switch VM integration.
