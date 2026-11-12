# Чтение сохранений Minecraft Java — v126

Этот модуль — первый слой совместимости миров для OptiCraft Java Lite 1.2.5.

## Что читает
- `level.dat` (GZIP + NBT) и резервный `level.dat_old`;
- Anvil-регионы `region/r.<rx>.<rz>.mca`;
- записи чанков с компрессией GZIP, zlib или без сжатия;
- NBT TAG_Byte/Short/Int/Long/Float/Double/Byte_Array/String/List/Compound/Int_Array/Long_Array;
- старые секции чанков 1.2.x: `Y`, `Blocks`, `Data`, `BlockLight`, `SkyLight`, `HeightMap`.

Полное NBT-дерево чанка сохраняется в `ChunkData::nbt`, поэтому сущности, tile entities и неизвестные теги не отбрасываются.

## Проверка
```sh
tar -xzf runtime/world/world_reader_v126.tar.gz -C .
sh runtime/world/tests/run_world_save_reader_test.sh
```

Локально тест запускался также против сохранения `New World`, приложенного в архиве OptiCraft: прочитан level.dat и найден чанк в `r.-1.0.mca`.

## Границы текущего этапа
Это проверенный reader для Anvil/NBT и legacy chunk sections формата 1.2.x. Он пока не подключён к игровому циклу/генератору мира или экрану выбора миров; загрузка мира в настоящий клиент ещё не подтверждена. Современные миры Java с палитрами блоков и новыми DataVersion требуют отдельного конвертера/адаптера и не заявляются как совместимые на этом этапе.
