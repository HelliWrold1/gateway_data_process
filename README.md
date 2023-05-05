# Dependencies

MariaDB/MySQL: https://mariadb.com/ or https://www.mysql.com/

lorawan-server: https://github.com/gotthardp/lorawan-server

mosquitto: https://github.com/eclipse/mosquitto

cJSON: https://github.com/DaveGamble/cJSON

paho.mqtt.c: https://github.com/eclipse/paho.mqtt.c

mysql-connector-c: https://dev.mysql.com/downloads/c-api/

MariaDB Connector/C: https://mariadb.com/docs/skysql-previous-release/connect/programming-languages/c/install/

# Installation

## Compile and start service

```shell
cmake --build /home/pi/gateway-data-process/cmake-build-release --target gateway_data_process -- -j 6
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -G "CodeBlocks - Unix Makefiles" -S /home/pi/gateway-data-process -B /home/pi/gateway-data-process/cmake-build-release
chmod 755 gdpStart.sh startService.sh
sudo ./startService.sh
```

## Install MariaDB/MySQL

```shell
sudo apt install  mysql-server
```

## Install MariaDB Connector/C(Linux)

```shell
sudo apt install libmariadb3 libmariadb-dev
```

## Install mysql-connector-c(Linux)

```shell
 sudo apt-get install libmysqlclient-dev
```

```shell
sudo apt-get install mosquitto
```

## Mosquitto Configuration

Copy [Config File](mqtt/mosquitto.conf) to etc/mosquitto/

```shell
sudo cp mqtt/mosquitto.conf etc/mosquitto/mosquitto.conf
```