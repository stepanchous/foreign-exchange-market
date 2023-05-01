# Foreign Exchange Market
Foreign exchange market на C++20

# Описание
Проект представляет собой валютную биржу с клиент-серверной архитектурой, реализованную с помощью библиотеки Boost.Asio. Сервер поддерживает подключение нескольких клиентов одновременно, принимая запросы асинхронно. Клиент может оставлять заявки на покупку или продажу валюты, просматривать котировки, отменять заявки, получать отчет об активных заявках и завершенных сделках. Кроме того, история заявок и сделок хранится в базе данных, реализованной на SQLite. Аутентификация пользователей обеспечивает возможность работать одному клиенту из разных инстансов программы.

# Требования
- clang 15.x.x
- CMake `sudo dnf install cmake`
- Boost `sudo dnf install boost && sudo dnf install boost-devel`
- SQLite `sudo dnf install sqlite && sudo dnf install sqlite-devel`
- Catch2 (для тестов)
```
git clone https://github.com/catchorg/Catch2.git
cd Catch2
cmake -Bbuild -H. -DBUILD_TESTING=OFF
sudo cmake --build build/ --target install
```
Команды установки библиотек (кроме Catch2) приведены для Fedora. 

# Сборка и запуск
## Биржа
```
git clone https://github.com/stepanchous/foreign-exchange-market.git
cd foreign-exchange-market
cmake .
make
./server.out
./client.out
```
## Тесты
```
git clone https://github.com/stepanchous/foreign-exchange-market.git
cd foreign-exchange-market/test
cmake .
make
./tests.out
```

# Идеи по доработке
- Расширение списка торговых активов, продаваемых и покупаемых на бирже
- Реализация сохранения и выгрузки состояния сервера при запуске и выключении
- Реализация графического интерфейса

# Замечания по иcпользованию
Так как на данном этапе состояние биржи не срохраняется, данные в базе данных релевантны только в рамках одного запуска сервера. Хотя консистентность данных нарушена не будет, рекомендуется удалять db/market.db перед запуском. Для выключения сервера необходимо отправить сигнал SIGINT (ctrl+c). Сигнал будет обработан и программа завершится корректно.
