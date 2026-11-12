# OptiCraft Switch — первый интерактивный прототип

Проект портирования OptiCraft Java 1.2.5 + OptiFine HD U C7 на Nintendo Switch.

## Первый игровой прототип 0.1

Ветка `prototype/first-playable-world` добавляет нативное libnx-приложение с процедурным изометрическим миром 16×16, перемещением игрока, установкой и удалением блоков, управлением Joy-Con / Pro Controller и программным framebuffer-рендерером.

**Управление:** D-pad / левый стик — движение; A — поставить блок; B — убрать поставленный блок; X — сменить Wood / Brick; Y — новый мир; + — выход.

Сборка выполняется workflow **Build OptiCraft playable Switch prototype**. После успешного запуска workflow скачай artifact `OptiCraft-World-Prototype-0.1` из GitHub Actions.

## Важно: что пока не портировано

Это первый интерактивный NRO-прототип, а не запуск оригинального Minecraft Java 1.2.5. В версии 0.1 ещё нет JVM в NRO, Minecraft-классов, оригинальных текстур, сохранения, инвентаря, мобов и мультиплеера. Дальнейшая работа — поэтапно подключить существующий JVM/runtime код и заменить временный renderer и мир компонентами оригинальной игры.

Подробности: [prototype/README_RU.md](prototype/README_RU.md) и [PLAYABLE_PROTOTYPE_v0.1_RU.md](PLAYABLE_PROTOTYPE_v0.1_RU.md).

## Исторические материалы

Архив v91 и старый workflow первого диагностического NRO сохранены как инженерная история. Для нового игрового прототипа используй workflow **Build OptiCraft playable Switch prototype**.
