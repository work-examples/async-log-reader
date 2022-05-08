﻿# ЗАМЕТКИ ПО РЕШЕНИЮ

| Branch      | CI Build Status                                                                                                                                                                                         |
|-------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **master**  | [![CI status](https://github.com/work-examples/async-log-reader/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/work-examples/async-log-reader/actions?query=branch%3Amaster)   |
| **develop** | [![CI status](https://github.com/work-examples/async-log-reader/actions/workflows/main.yml/badge.svg?branch=develop)](https://github.com/work-examples/async-log-reader/actions?query=branch%3Adevelop) |


## Ход решения

Во-первых было интересно, спасибо! :)
Выполнение заняло сильно больше времени, чем я изначально оценил.

Было весьма унизительно, когда первая версия оказалась медленнее `grep` раз в 5-6
при очень высоком уровне оптимизации по скорости со всякими статически выделенными массивами без перевыделений памяти.
А я ожидал, что результат уже должен быть лучше аналогов. Тем более, что у меня специализированный алгоритм,
а у grep синтаксис иной и алгоритм по определению хуже должен быть.

Потом я заменил алгоритм матча строки с динамическим программированием и памятью O(P*N) на другой, более быстрый и с константной памятью.
И... я стал медленнее grep всего в 2 раза. Почти успех :))) При этом по профайлеру релизному у меня 80+% проводилось именно в матче строк.

Не буду томить, сомневаюсь, что более эффективный алгоритм существует для этой задачи.
Победить grep вышло лишь добавив несколько оптимизаций, которые просто ускоренно прокручивали алгоритм в популярных сценариях.
Это ускорило матчинг примерно в 6 раз, а общее время работы программы раза в 3. И эта дало победу над grep всего лишь на 25-35%.

После этого согласно профайлеру почти ровно половина времени проводилась
в матче строк, а процентов под 40 времени проводилось в синхронном ReadFile().

Я решил, что вот он звёздный час асинхронного чтения файлов!
Запрограммировал, и... ничего. Общее время работы не изменилось.
Но профайлер показал перераспределение времени в сторону алгоритма матча. Очень странно, что он замедлился. Я так и не понял почему так.
Я убеждён, что эти 40% можно было сжать до максимум 5% за счет параллелизации вычитки данных и их обработки.
Возможно это как-то связано с тем, что данные лежат в файловом кэше, а не читаются с диска (более короткий путь IRP).
Возможно это банальное копирование памяти в kernel mode плохо параллелизуется.
Может стоило переписать, чтобы чтение с диска в выделенном потоке выполнялось...
Если у вас есть идеи почему так вышло или в чём ошибка, то буду рад если поделитесь.

Также заметил вставку перед циклом команды nop при оптимизации по скорости компилятором.
Есть лишь предположения зачем. Если вы вдруг знаете - тоже дайте знать. Есть скриншот, приложил его в проект.
Я спросил моих разных коллег, они не в курсе.

Попробовал на всякий случай отображать файл на память.
Но у меня были сомнения в эффективности скорости подгрузки новых страниц в этом решении.
Так и вышло. Немного медленнее, процентов на 20% (время всей программы).
Хотя тоже странно при прогретом кэше.
Теоретически, если данные в кэше лежат, то можно было бы их отобразить на виртуальную память в read-only режиме за O(1),
а потом экономить на переходах в kernel mode + экономить на копировании памяти.

## Тестирование и заметки

Тестировал на логе веб сервера, 2 Гб, 5.5 млн строк, средняя длина строки 380 байт, все строки не длиннее 1024 байт.
Подходило под паттерн 1600 строк. Паттерн был взят вида "*строка*" как наиболее популярный в обычной жизни.

Диск SSD, но я прогревал чтобы всё легло в файловый кэш. 8 ядер логических core i5 gen8, ноутбук.

Сборка под x64 архитектуру работала быстрее чем под x86.

Флаг FILE_FLAG_SEQUENTIAL_SCAN не дал прироста скорости на прогретом кэше. Без прогретого кэша надо измерять отдельно.

Иногда скорость надолго залипает на +25%, иногда на самом быстром варианте.
Скорее всего связано с тем, что у меня ноутбук и у ядер есть режимы экономные. Но не факт.

В последней версии grep показывает 2.5 секунды, а моё решение - 1.6 секунды на тестовых данных.

**ДОПОЛНЕНО:**
Реализовал также чтение файла в отдельном потоке. Файловая операция синхронная, синхронизация с потоком lock free (spinlock).
Получил общий выигрыш в 25% относительно решения на синхронном и асинхронном API (1.2 секунды). Итого в 2 раза быстрее чем grep.

## Особенности реализации

В итоге у меня остались все три реализации чтения файлов. Я оставил асинхронную версию как самую перспективную. Переключаются так:

```cpp
#if 0
#if 0
    CSyncLineReader    _lineReader;
#else
    CMappingLineReader _lineReader;
#endif
#else
#if 0
    CAsyncLineReader    _lineReader;
#else
    CLockFreeLineReader _lineReader;
#endif
#endif
```

Также есть юнит тесты на базе gtest с отдельным проектом.

Как и в условии задачи, основное консольное приложение собрано с отключенными исключениями и заодно без RTTI.

Я притянул часть STL на мой страх и риск. Ту часть, которая не требует исключений и работает без лишних накладных расходов.
Не вижу смысла не пользоваться дешевыми абстракциями, позволяющими писать более чистый код
и более защищенный от ошибок программиста. И без велосипедов.
Я имею в виду всякие std::unique_ptr, std::string_view, std::optional.

---
