
Описание для сборки программы тестирования.
Для кореектной работы приложения, небходимо скопировать файлы main.qml и TestRow.qml на целевое устройство в /usr/local/share

##

1. Создайте папку для сборки build.
```bash
mkdir build
```

2. Перейдите в созданную папку 
```bash
cd build
```

3. Запустить сборку cmake с ипользованием toolchain файла под ds-rk3568, где ${PATH_TO_FOLDER} путь до папки bsp.
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=${PATH_TO_FOLDER}/bsp/output/ds-rk3568-evb/host/share/buildroot/toolchainfile.cmake ..
```
4. Выполните сборку тестовой программы
```bash
make
```

5. На выходе должны получить
```bash
ls
CMakeCache.txt  CMakeFiles  cmake_install.cmake  Makefile  tester  tester_autogen
```

6. tester - испольняемый файл, который вы можете запустить на отладочной плате diasom-rk3568

