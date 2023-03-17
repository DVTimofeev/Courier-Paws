# Не просто создаём образ, но даём ему имя build_debug
FROM gcc:11.3 as build_debug

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.59.0

# Запуск conan как раньше
COPY conanfile.txt /app/
RUN mkdir /app/build_debug && cd /app/build_debug && \
    conan install .. -s compiler.libcxx=libstdc++11 -s build_type=Debug

# Папка data больше не нужна
COPY ./src /app/src
COPY ./include /app/include
COPY ./tests /app/tests
COPY CMakeLists.txt /app/

RUN cd /app/build_debug && \
    cmake .. -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build .

# Второй контейнер в том же докерфайле
FROM ubuntu:22.04 as run

# Создадим пользователя www
RUN groupadd -r www && useradd -r -g www www
USER www

# Скопируем приложение со сборочного контейнера в директорию /app.
# Не забываем также папку data, она пригодитсincludeя.
COPY --from=build_debug /app/build_debug/bin/game_server /app/
COPY ./data /app/data
COPY ./static /app/static

# Запускаем игровой сервер
# обычный пуск
ENTRYPOINT ["/app/game_server", "-c", "/app/data/config.json", "-w", "/app/static"]
# запуск с тиком
# ENTRYPOINT ["/app/game_server", "-c", "/app/data/config.json", "-w", "/app/static", "--tick-period", "500"]
