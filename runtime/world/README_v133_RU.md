# v133 — загрузка terrain.png из оригинального JAR

NRO теперь пытается открыть пользовательский JAR по пути:

sdmc:/switch/opticraft/minecraft.jar

Загрузчик:
- находит terrain.png в ZIP central directory;
- читает ZIP stored/deflate entry;
- проверяет CRC32 записи;
- распаковывает PNG 8-bit RGB/RGBA без interlace;
- применяет PNG filters 0–4;
- хранит RGBA atlas в памяти NRO.

В рендерере верхняя ромбовидная поверхность получает пиксели из atlas. Если JAR отсутствует или atlas не читается, остаётся прежний цветной fallback.

Важно: исходный JAR не копируется в репозиторий и не включается в NRO. Пользователь сам помещает свою копию файла на SD-карту. Пока используется ограниченная таблица atlas tiles; боковые грани, анимация жидкостей, GUI/items atlas, прозрачность и точные модели ещё не реализованы.

## Windows: автоматическое копирование JAR

Из корня репозитория запусти PowerShell:

```powershell
.\tools\install_user_minecraft_jar.ps1 -OptiCraftZip "C:\path\OptiCraft JAVA LITE 1.2.5.zip" -SdRoot "E:\"
```

Замени путь к ZIP и букву SD-карты. Скрипт извлекает только minecraft.jar и не изменяет исходный архив.
